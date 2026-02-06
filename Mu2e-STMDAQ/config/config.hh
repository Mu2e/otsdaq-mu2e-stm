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
#include <array>
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
    
    // Patch the value directly in the raw XML file
    if (!patchValueInFile(key, strValue)) {
      throw std::runtime_error("Failed to patch value for key: " + key);
    }

    // Reload the updated document into memory
    pugi::xml_parse_result result = doc.load_file(xml_path.c_str());
    if (!result) throw std::runtime_error("Failed to reload XML file after patching.");
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

  // Patch the XML value by directly editing the raw file content
  bool patchValueInFile(const std::string& key, const std::string& newValue) {
    // Load the entire file into a string
    std::ifstream fileIn(xml_path);
    if (!fileIn.is_open()) return false;

    std::ostringstream buffer;
    buffer << fileIn.rdbuf();
    std::string content = buffer.str();
    fileIn.close();

    // Parse key into tag and parent
    std::vector<std::string> pathSegments;
    std::istringstream iss(key);
    std::string token;
    while (std::getline(iss, token, '.')) {
      pathSegments.push_back(token);
    }

    if (pathSegments.size() < 2) return false;
    std::string tag = pathSegments.back();
    std::string parent = pathSegments[pathSegments.size() - 2];

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

    // Save modified file
    std::ofstream fileOut(xml_path);
    if (!fileOut.is_open()) return false;
    fileOut << content;
    fileOut.close();

    return true;
  }
  
  // Delete copy constructor and assignment operator                     
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;

  // The xml path
  std::string xml_path;
  // The xml document
  pugi::xml_document doc;
};

#endif
