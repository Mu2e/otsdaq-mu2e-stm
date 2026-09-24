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
#include <unordered_map> // map full keys -> (file, local key) for include-aware setValue()
#include <utility>       // std::pair
#include <filesystem>    // path handling for include files relative to main XML
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

    // Determine which physical XML file should be patched (main or an included fragment) 
    auto fileAndLocal = getFileAndLocalKeyForFullKey(key); // figure out target file and key within that file
    
    std::cerr << "Patching file: " << fileAndLocal.first << " with local key: " << fileAndLocal.second << "\n";
    
    // Load the target file (main or included fragment)
    pugi::xml_document targetDoc;
    if (!targetDoc.load_file(fileAndLocal.first.c_str()))
        throw std::runtime_error("Failed to load: " + fileAndLocal.first);

    // Walk the local key path to find the exact node
    pugi::xml_node targetNode = targetDoc.document_element();
    std::istringstream ss(fileAndLocal.second);
    std::string token;
    while (std::getline(ss, token, '.')) {
        if (token == targetNode.name()) continue; // skip root element name
        targetNode = targetNode.child(token.c_str());
        if (!targetNode)
            throw std::runtime_error("Node not found in file: " + token);
    }

    // Set the value on the exact node — no regex, no ambiguity
    targetNode.text().set(strValue.c_str());

    // Save back to the correct file
    if (!targetDoc.save_file(fileAndLocal.first.c_str()))
        throw std::runtime_error("Failed to save: " + fileAndLocal.first);

    // Rebuild in-memory DOM to reflect the change
    rebuildExpandedDocument();
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

    // Build the expanded DOM by resolving <include file="..."/> nodes before any lookups happen 
    rebuildExpandedDocument(); // expand includes and build key->file mapping
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

  // Expand <include file="..."/> directives and rebuild the key->file mapping 
  void rebuildExpandedDocument() { 
    include_key_map.clear(); // clear old mapping (files may have changed after setValue)
    base_dir = std::filesystem::path(xml_path).parent_path(); // remember base directory for relative include paths

    // Start by re-loading the main XML file into doc (so we always expand from the on-disk source) 
    pugi::xml_parse_result result = doc.load_file(xml_path.c_str()); // load main file again
    if (!result) throw std::runtime_error("Failed to load XML file: " + std::string(result.description())); // maintain strong error reporting

    // Expand includes recursively into the in-memory document 
    expandIncludesRecursive(doc.document_element()); 

    // Build the mapping for main-file-owned nodes 
    registerKeysForSubtree(doc.document_element(), doc.document_element().name(), doc.document_element().name(), xml_path, false); // do NOT overwrite include-owned mappings
    // registerKeysForSubtree(doc.document_element(), doc.document_element().name(), doc.document_element().name(), xml_path); // main doc keys map to main file
  }

  // Recursively expand <include file="..."/> nodes by splicing the included file's root element into the DOM 
  void expandIncludesRecursive(pugi::xml_node node) { 
    for (pugi::xml_node child = node.first_child(); child; ) { // iterate manually because we may erase/insert while iterating
      pugi::xml_node next = child.next_sibling(); // stash next sibling before modifying the tree

      // Recurse into normal elements first 
      if (child.type() == pugi::node_element && std::string(child.name()) != "include") { // only recurse into non-include elements
        expandIncludesRecursive(child); // expand nested includes deeper in the tree
      }

      // If this is an <include file="..."/> element, splice in the referenced XML 
      if (child.type() == pugi::node_element && std::string(child.name()) == "include") { // detect include directive
        const char* fileAttr = child.attribute("file").value(); // read include filename
        if (!fileAttr || std::string(fileAttr).empty()) { // validate attribute exists
          throw std::runtime_error("Include tag missing 'file' attribute."); 
        }

        std::filesystem::path includePath = base_dir / std::filesystem::path(fileAttr); // resolve include file relative to main XML directory

        pugi::xml_document subdoc; // temporary document for included file
        pugi::xml_parse_result subres = subdoc.load_file(includePath.string().c_str()); // parse included XML file
        if (!subres) { // error handling for include parse failures
          throw std::runtime_error("Failed to load included XML file: " + includePath.string() + " (" + std::string(subres.description()) + ")"); 
        }

        pugi::xml_node subroot = subdoc.document_element(); // root element of included file
        if (!subroot) { // sanity check
          throw std::runtime_error("Included XML file has no document element: " + includePath.string()); 
        }

        // Insert the included root node BEFORE the <include> element, preserving order in the parent 
        pugi::xml_node inserted = node.insert_copy_before(subroot, child); // splice included root into main DOM

        // Register keys under the inserted subtree so setValue knows which file to patch 
        std::string parentFull = computeNodeFullKey(node); // compute full key for the parent node in the expanded DOM
        std::string insertedFull = parentFull.empty() ? inserted.name() : parentFull + "." + inserted.name(); // full key for inserted root under parent
        std::string insertedLocal = inserted.name(); // local key within the included file starts at its root
        //registerKeysForSubtree(inserted, insertedFull, insertedLocal, includePath.string()); // map inserted keys -> included file path
        registerKeysForSubtree(inserted, insertedFull, insertedLocal, includePath.string(), true); // force include-owned mapping for inserted subtree


        // Remove the <include> node itself 
        node.remove_child(child); // delete the directive from the expanded DOM
      }

      child = next; // advance iteration safely
    }
  }

  // Compute the full dotted key for a node by walking up to the document element 
  std::string computeNodeFullKey(pugi::xml_node node) const { 
    if (!node) return ""; // defensive
    std::vector<std::string> parts; // collect path segments bottom-up
    pugi::xml_node cur = node; // iterator up the tree
    while (cur && cur.type() == pugi::node_element) { // walk element nodes
      parts.push_back(cur.name()); // store this element name
      if (cur == doc.document_element()) break; // stop at the document root element
      cur = cur.parent(); // move up
    }
    std::ostringstream oss; // build dotted key top-down
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) { // reverse to get root->leaf
      if (it != parts.rbegin()) oss << "."; // dot separators
      oss << *it; // segment
    }
    return oss.str(); // resulting full key
  }

  // Register every element in a subtree as belonging to a particular source file 
  void registerKeysForSubtree(pugi::xml_node node, const std::string& fullPrefix, const std::string& localPrefix,
		  const std::string& sourceFile, const bool overwriteExisting = true) { // add overwrite flag (default keeps existing behaviour)
    if (!node) return; // defensive

    // Store mapping for this node itself 
    if (overwriteExisting) { // allow caller to force ownership mapping
      include_key_map[fullPrefix] = std::make_pair(sourceFile, localPrefix); // overwrite mapping (old behaviour)
    } else { // preserve earlier mapping if it already exists (used to protect include-owned keys)
      include_key_map.emplace(fullPrefix, std::make_pair(sourceFile, localPrefix)); // insert only if key not already present
    } 

    // Recurse through element children 
    for (pugi::xml_node child : node.children()) { 
      if (child.type() != pugi::node_element) continue; // skip text/comments/etc.
      std::string childName = child.name(); // child tag name
      std::string childFull = fullPrefix.empty() ? childName : fullPrefix + "." + childName; // full dotted key
      std::string childLocal = localPrefix.empty() ? childName : localPrefix + "." + childName; // local dotted key (within its source file)
      registerKeysForSubtree(child, childFull, childLocal, sourceFile, overwriteExisting); // propagate overwrite policy down the subtree
    }
  }

  
  // Look up which file a given full key belongs to, and what the local key is inside that file 
  std::pair<std::string, std::string> getFileAndLocalKeyForFullKey(const std::string& fullKey) const { 
    // Normalize by ensuring the key is rooted at the main document element name 
    std::string rootName = doc.document_element().name(); 
    std::string normalized = fullKey; 
    if (normalized.find(rootName + ".") != 0 && normalized != rootName) { // if user passes "channel" instead of "stm.channel", keep behaviour consistent
      // If the caller provided a relative key like "channel", interpret it relative to the root 
      normalized = rootName + "." + normalized; 
    }

    // Prefer exact mapping hits 
    auto it = include_key_map.find(normalized); 
    if (it != include_key_map.end()) { 
      return it->second; // return (file, local key)
    }

    // If not found, default to patching the main XML file using the provided key 
    // (This preserves prior behaviour for any keys that weren't in the mapping for some reason.) 
    return std::make_pair(xml_path, fullKey); 
  }
  
  // Delete copy constructor and assignment operator                     
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  // The xml path
  std::string xml_path;
  // The xml document
  pugi::xml_document doc;

  // Base directory of the main XML file for resolving relative include paths 
  std::filesystem::path base_dir; 

  // Map from expanded full keys (e.g. "stm.channel") to (source file, local key in that file) 
  std::unordered_map<std::string, std::pair<std::string, std::string>> include_key_map; 
};

#endif
