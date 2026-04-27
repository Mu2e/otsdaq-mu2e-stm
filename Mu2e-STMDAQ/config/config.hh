#ifndef CONFIG_HH_
#define CONFIG_HH_

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <regex>
#include <iomanip> 
#include <unordered_map> // NEW: map full keys -> (file, local key) for include-aware setValue()
#include <utility>       // NEW: std::pair
#include <filesystem>    // NEW: path handling for include files relative to main XML
#include <functional>
#include "pugixml.hpp"

// Configuation class 
class Config {
  
public:
  // Access the singleton instance
  static Config& getInstance(const std::string& xmlFile = "") {
    static Config instance(xmlFile); // Lazy initialization
    return instance;
  }
  
  std::string getXMLpath() const {
    return xml_path;
  }

  // Re-read the XML file and reset all internal state
  void reinit() {
    rebuildExpandedDocument();
  }
  
  // Access configuration values
  // Template method definition must be in the header
  template <typename T>
  T getValue(const std::string& key) const {
    pugi::xml_node node = resolveKeyPath(key);
    if (!node) {
      std::string parentKey = key.substr(0, key.find_last_of('.'));
      std::string finalKey = key.substr(key.find_last_of('.') + 1);
      pugi::xml_node parent = resolveKeyPath(parentKey);
      if (parent) {
        node = parent.child(finalKey.c_str());
      }
    }    
    if (!node) {
      throw std::runtime_error("Key not found: " + key);
    }
    std::istringstream iss(node.text().get());
    T value;
    iss >> value;
    if (iss.fail()) {
      throw std::runtime_error("Invalid value or type for key: " + key);
    }
    return value;
  }

  // Set value using a text-patching approach that preserves comments and formatting
  template <typename T>
  void setValue(const std::string& key, const T& value) {
    std::ostringstream oss;
    oss << value;
    std::string strValue = oss.str();

    // Determine which physical XML file should be patched (main or an included fragment) // NEW
    auto fileAndLocal = getFileAndLocalKeyForFullKey(key); // NEW: figure out target file and key within that file
    
    std::cerr << "Patching file: " << fileAndLocal.first << " with local key: " << fileAndLocal.second << "\n";
    
    // Patch the value directly in the raw XML file
    if (!patchValueInFile(fileAndLocal.first, fileAndLocal.second, strValue)) { // NEW: patch correct file + correct local key
      throw std::runtime_error("Failed to patch value for key: " + key);
    }

    // Reload the updated document into memory
    pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
    if (!result) throw std::runtime_error("Failed to reload XML file after patching.");

    // Re-expand <include .../> nodes so the in-memory DOM matches the split-on-disk configuration // NEW
    rebuildExpandedDocument(); // NEW: reload + expand includes + rebuild key->file map
  }

  // Print all key paths and values
  void printAllKeys() const {
    printKeysRecursive(doc.document_element(), "");
  }

  // Extract key-value pairs under a specific subtree
  std::vector<std::pair<std::string, std::string>> extractKeysAndValues(const std::string& key) const {
    std::vector<std::pair<std::string, std::string>> result;
    pugi::xml_node node = resolveKeyPath(key);
    if (!node) throw std::runtime_error("Key not found: " + key);
    
    for (pugi::xml_node child : node.children()) {
      std::string name = child.name();
      if (name.empty()) continue;  // Skip text nodes
      result.emplace_back(name, child.text().as_string());
    }
    return result;
  }

  // Destructor 
  ~Config() {}
  
private:

  // Private constructor
  Config(const std::string& xmlFile) : xml_path(xmlFile) {
    pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
    if (!result) throw std::runtime_error("Failed to load XML file: " + std::string(result.description()));

    // Build the expanded DOM by resolving <include file="..."/> nodes before any lookups happen // NEW
    rebuildExpandedDocument(); // NEW: expand includes and build key->file mapping
  }

  // Get the key path
  pugi::xml_node resolveKeyPath(const std::string& key) const {
    std::string adjustedKey = key;
    // If key starts with root tag name (e.g., "stm."), remove it
    std::string rootName = doc.document_element().name();
    if (adjustedKey.find(rootName + ".") == 0) {
      adjustedKey = adjustedKey.substr(rootName.length() + 1);
    }
    std::istringstream ss(adjustedKey);
    std::string token;
    pugi::xml_node node = doc.document_element();    
    while (std::getline(ss, token, '.')) {
      node = node.child(token.c_str());
      if (!node) break;
    }    
    return node;
  }

  // Helper function for recursive printing
  void printKeysRecursive(pugi::xml_node node, const std::string& prefix) const {
    for (pugi::xml_node child : node.children()) {
      std::string fullKey = prefix.empty() ? child.name() : prefix + "." + child.name();
      std::string value = child.text().as_string();
      std::cout << fullKey << ": " << value << std::endl;
      printKeysRecursive(child, fullKey);
    }
  }

  // Patch th
  //e XML value by directly editing the raw file content
  bool patchValueInFile(const std::string& targetPath, const std::string& key, const std::string& newValue) { // NEW: patch a specified file + local key
    // Load the entire file into a string
    std::ifstream fileIn(targetPath); // NEW: open the selected file (main or included)
    if (!fileIn.is_open()) return false;

    std::ostringstream buffer;
    buffer << fileIn.rdbuf();
    std::string content = buffer.str();
    fileIn.close();

    // Parse key into path segments
    std::vector<std::string> pathSegments; // NEW: now supports root-only keys too
    std::istringstream iss(key); // NEW: parse the LOCAL key (within that file)
    std::string token; // NEW: token for splitting
    while (std::getline(iss, token, '.')) { // NEW: split local key on '.'
      if (!token.empty()) pathSegments.push_back(token); // NEW: ignore empty tokens defensively
    }

    if (pathSegments.empty()) return false; // NEW: no key segments -> cannot patch

    // Special case: included files often contain ONLY a single root element (e.g. <channel>0</channel>) // NEW
    if (pathSegments.size() == 1) { // NEW: patch a root-only tag anywhere in the file
      std::string tag = pathSegments.back(); // NEW: the element name to replace
      std::regex oneTag("<" + tag + ">([\\s\\S]*?)</" + tag + ">"); // NEW: match the first occurrence of <tag>...</tag>
      std::smatch m; // NEW: regex match object
      if (!std::regex_search(content, m, oneTag)) return false; // NEW: if tag not found, fail
      std::string replacement = "<" + tag + ">" + newValue + "</" + tag + ">"; // NEW: replacement text
      content = std::regex_replace(content, oneTag, replacement, std::regex_constants::format_first_only); // NEW: patch only first match
    } else {
      // Keep the existing behaviour: patch the last tag within its immediate parent block // NEW (comment only; logic preserved + generalized to selected file)
      std::string tag = pathSegments.back(); // NEW: leaf tag to patch
      std::string parent = pathSegments[pathSegments.size() - 2]; // NEW: immediate parent tag

      // Regex to find the parent block
      std::regex outerTag("<" + parent + ">([\\s\\S]*?)</" + parent + ">");
      std::smatch parentMatch;
      if (!std::regex_search(content, parentMatch, outerTag)) return false;

      std::string parentBlock = parentMatch[0];

      // Regex to find the target tag inside the parent block
      std::regex innerTag("<" + tag + ">(.*?)</" + tag + ">");
      std::smatch innerMatch;
      if (!std::regex_search(parentBlock, innerMatch, innerTag)) return false;

      std::string oldInner = innerMatch[0];
      std::string newInner = "<" + tag + ">" + newValue + "</" + tag + ">";

      std::string modifiedParentBlock = std::regex_replace(
							 parentBlock, innerTag, newInner, std::regex_constants::format_first_only);

      // Replace the parent block in the full document
      content.replace(parentMatch.position(0), parentBlock.length(), modifiedParentBlock);
    }

    // Save modified file
    std::ofstream fileOut(targetPath); // NEW: write back to the selected file (main or included)
    if (!fileOut.is_open()) return false;
    fileOut << content;
    fileOut.close();

    return true;
  }

  // Expand <include file="..."/> directives and rebuild the key->file mapping // NEW
  void rebuildExpandedDocument() { // NEW
    include_key_map.clear(); // NEW: clear old mapping (files may have changed after setValue)
    base_dir = std::filesystem::path(xml_path).parent_path(); // NEW: remember base directory for relative include paths

    // Start by re-loading the main XML file into doc (so we always expand from the on-disk source) // NEW
    pugi::xml_parse_result result = doc.load_file(xml_path.c_str()); // NEW: load main file again
    if (!result) throw std::runtime_error("Failed to load XML file: " + std::string(result.description())); // NEW: maintain strong error reporting

    // Expand includes recursively into the in-memory document // NEW
    expandIncludesRecursive(doc.document_element()); // NEW

    // Build the mapping for main-file-owned nodes // NEW
        registerKeysForSubtree(doc.document_element(), doc.document_element().name(), doc.document_element().name(), xml_path, false); // NEW: do NOT overwrite include-owned mappings
        //    registerKeysForSubtree(doc.document_element(), doc.document_element().name(), doc.document_element().name(), xml_path); // NEW: main doc keys map to main file
  }

  // Recursively expand <include file="..."/> nodes by splicing the included file's root element into the DOM // NEW
  void expandIncludesRecursive(pugi::xml_node node) { // NEW
    for (pugi::xml_node child = node.first_child(); child; ) { // NEW: iterate manually because we may erase/insert while iterating
      pugi::xml_node next = child.next_sibling(); // NEW: stash next sibling before modifying the tree

      // Recurse into normal elements first // NEW
      if (child.type() == pugi::node_element && std::string(child.name()) != "include") { // NEW: only recurse into non-include elements
        expandIncludesRecursive(child); // NEW: expand nested includes deeper in the tree
      }

      // If this is an <include file="..."/> element, splice in the referenced XML // NEW
      if (child.type() == pugi::node_element && std::string(child.name()) == "include") { // NEW: detect include directive
        const char* fileAttr = child.attribute("file").value(); // NEW: read include filename
        if (!fileAttr || std::string(fileAttr).empty()) { // NEW: validate attribute exists
          throw std::runtime_error("Include tag missing 'file' attribute."); // NEW
        }

        std::filesystem::path includePath = base_dir / std::filesystem::path(fileAttr); // NEW: resolve include file relative to main XML directory

        pugi::xml_document subdoc; // NEW: temporary document for included file
        pugi::xml_parse_result subres = subdoc.load_file(includePath.string().c_str()); // NEW: parse included XML file
        if (!subres) { // NEW: error handling for include parse failures
          throw std::runtime_error("Failed to load included XML file: " + includePath.string() + " (" + std::string(subres.description()) + ")"); // NEW
        }

        pugi::xml_node subroot = subdoc.document_element(); // NEW: root element of included file
        if (!subroot) { // NEW: sanity check
          throw std::runtime_error("Included XML file has no document element: " + includePath.string()); // NEW
        }

        // Insert the included root node BEFORE the <include> element, preserving order in the parent // NEW
        pugi::xml_node inserted = node.insert_copy_before(subroot, child); // NEW: splice included root into main DOM

        // Register keys under the inserted subtree so setValue knows which file to patch // NEW
        std::string parentFull = computeNodeFullKey(node); // NEW: compute full key for the parent node in the expanded DOM
        std::string insertedFull = parentFull.empty() ? inserted.name() : parentFull + "." + inserted.name(); // NEW: full key for inserted root under parent
        std::string insertedLocal = inserted.name(); // NEW: local key within the included file starts at its root
        //        registerKeysForSubtree(inserted, insertedFull, insertedLocal, includePath.string()); // NEW: map inserted keys -> included file path
        registerKeysForSubtree(inserted, insertedFull, insertedLocal, includePath.string(), true); // NEW: force include-owned mapping for inserted subtree


        // Remove the <include> node itself // NEW
        node.remove_child(child); // NEW: delete the directive from the expanded DOM
      }

      child = next; // NEW: advance iteration safely
    }
  }

  // Compute the full dotted key for a node by walking up to the document element // NEW
  std::string computeNodeFullKey(pugi::xml_node node) const { // NEW
    if (!node) return ""; // NEW: defensive
    std::vector<std::string> parts; // NEW: collect path segments bottom-up
    pugi::xml_node cur = node; // NEW: iterator up the tree
    while (cur && cur.type() == pugi::node_element) { // NEW: walk element nodes
      parts.push_back(cur.name()); // NEW: store this element name
      if (cur == doc.document_element()) break; // NEW: stop at the document root element
      cur = cur.parent(); // NEW: move up
    }
    std::ostringstream oss; // NEW: build dotted key top-down
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) { // NEW: reverse to get root->leaf
      if (it != parts.rbegin()) oss << "."; // NEW: dot separators
      oss << *it; // NEW: segment
    }
    return oss.str(); // NEW: resulting full key
  }

  // Register every element in a subtree as belonging to a particular source file // NEW
  void registerKeysForSubtree(pugi::xml_node node, const std::string& fullPrefix, const std::string& localPrefix, const std::string& sourceFile, const bool overwriteExisting = true) { // NEW: add overwrite flag (default keeps existing behaviour)
    if (!node) return; // NEW: defensive

    // Store mapping for this node itself // NEW
    if (overwriteExisting) { // NEW: allow caller to force ownership mapping
      include_key_map[fullPrefix] = std::make_pair(sourceFile, localPrefix); // NEW: overwrite mapping (old behaviour)
    } else { // NEW: preserve earlier mapping if it already exists (used to protect include-owned keys)
      include_key_map.emplace(fullPrefix, std::make_pair(sourceFile, localPrefix)); // NEW: insert only if key not already present
    } // NEW

    // Recurse through element children // NEW
    for (pugi::xml_node child : node.children()) { // NEW
      if (child.type() != pugi::node_element) continue; // NEW: skip text/comments/etc.
      std::string childName = child.name(); // NEW: child tag name
      std::string childFull = fullPrefix.empty() ? childName : fullPrefix + "." + childName; // NEW: full dotted key
      std::string childLocal = localPrefix.empty() ? childName : localPrefix + "." + childName; // NEW: local dotted key (within its source file)
      registerKeysForSubtree(child, childFull, childLocal, sourceFile, overwriteExisting); // NEW: propagate overwrite policy down the subtree
    }
  }

  
  // // Register every element in a subtree as belonging to a particular source file // NEW
  // void registerKeysForSubtree(pugi::xml_node node, const std::string& fullPrefix, const std::string& localPrefix, const std::string& sourceFile) { // NEW
  //   if (!node) return; // NEW: defensive

  //   // Store mapping for this node itself // NEW
  //   include_key_map[fullPrefix] = std::make_pair(sourceFile, localPrefix); // NEW: full key -> (file, local key)

  //   // Recurse through element children // NEW
  //   for (pugi::xml_node child : node.children()) { // NEW
  //     if (child.type() != pugi::node_element) continue; // NEW: skip text/comments/etc.
  //     std::string childName = child.name(); // NEW: child tag name
  //     std::string childFull = fullPrefix.empty() ? childName : fullPrefix + "." + childName; // NEW: full dotted key
  //     std::string childLocal = localPrefix.empty() ? childName : localPrefix + "." + childName; // NEW: local dotted key (within its source file)
  //     registerKeysForSubtree(child, childFull, childLocal, sourceFile); // NEW: depth-first registration
  //   }
  // }

  // Look up which file a given full key belongs to, and what the local key is inside that file // NEW
  std::pair<std::string, std::string> getFileAndLocalKeyForFullKey(const std::string& fullKey) const { // NEW
    // Normalize by ensuring the key is rooted at the main document element name // NEW
    std::string rootName = doc.document_element().name(); // NEW
    std::string normalized = fullKey; // NEW
    if (normalized.find(rootName + ".") != 0 && normalized != rootName) { // NEW: if user passes "channel" instead of "stm.channel", keep behaviour consistent
      // If the caller provided a relative key like "channel", interpret it relative to the root // NEW
      normalized = rootName + "." + normalized; // NEW
    }

    // Prefer exact mapping hits // NEW
    auto it = include_key_map.find(normalized); // NEW
    if (it != include_key_map.end()) { // NEW
      return it->second; // NEW: return (file, local key)
    }

    // If not found, default to patching the main XML file using the provided key // NEW
    // (This preserves prior behaviour for any keys that weren't in the mapping for some reason.) // NEW
    return std::make_pair(xml_path, fullKey); // NEW
  }
  
  // Delete copy constructor and assignment operator                     
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  // The xml path
  std::string xml_path;
  // The xml document
  pugi::xml_document doc;

  // Base directory of the main XML file for resolving relative include paths // NEW
  std::filesystem::path base_dir; // NEW

  // Map from expanded full keys (e.g. "stm.channel") to (source file, local key in that file) // NEW
  std::unordered_map<std::string, std::pair<std::string, std::string>> include_key_map; // NEW
};

#endif
