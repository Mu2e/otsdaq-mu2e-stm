#include <filesystem>
#include <regex>
#include <iostream>

// Include multi file handler code
#include "multi_file_handler.hh"

// Filesystem namespace
namespace fs = std::filesystem;

// Constructor
MultiFileHandler::MultiFileHandler(const std::vector<std::string>& files)
  : filenames_(files), current_file_index_(0), samples_read_(0) {

  // If number of files is non-zero
  if (!filenames_.empty()) {
    // Open the next file
    open_next_file();
  }

  // Calculate the total number of samples to process
  total_samples_ = 0;
  for (const auto& fname : filenames_) {
    std::error_code ec;
    auto filesize = fs::file_size(fname, ec);
    if (!ec) {
      total_samples_ += filesize / sizeof(int16_t);
    }
  }
  
}

// Load fils from a directory
void MultiFileHandler::load_files_from_directory(const std::string& directory,
							std::vector<std::string>& out_files) {

  // Vector of numbered files
  std::vector<std::pair<int, std::string>> numbered_files;
  // Look for subrun filename pattern(e.g., subrun(\d+)\.dat)
  std::regex pattern(R"(subrun(\d+))");  

  // Loop over files in diretory
  for (const auto& entry : fs::directory_iterator(directory)) {
    // If a regular file
    if (entry.is_regular_file()) {
      // Store the filename as a sring
      std::string filename = entry.path().filename().string();
      // Match the pattern in the filename
      std::smatch match;
      // If match, store files
      if (std::regex_search(filename, match, pattern)) {
	if (match.size() == 2) {
	  int num = std::stoi(match[1].str());
	  numbered_files.emplace_back(num, entry.path().string());
	}
      }
    }
  }

  // Sort the fles into numeric / consecutive order
  std::sort(numbered_files.begin(), numbered_files.end(),
	    [](const auto& a, const auto& b) { return a.first < b.first; });
  
  // Fill the output vector
  for (const auto& [num, path] : numbered_files) {
    out_files.push_back(path);
  }

  // Log to user
  std::cout << "Found " << out_files.size() << " subrun files in directory." << std::endl;
  
}

// Open next file
bool MultiFileHandler::open_next_file() {

  // Whilst curreent file index is less than total
  while (current_file_index_ < filenames_.size()) {

    // Curent the new filename
    const std::string& fname = filenames_[current_file_index_];

    // Open the new file
    current_stream_.open(fname, std::ios::binary);
    // If successful
    if (current_stream_) {
      // Log to user and return
      std::cout << "Opened file: " << fname << std::endl;
      return true;
    }
    // Else if open failed
    else {
      // Log to user and exit 
      std::cerr << "Failed to open: " << fname << std::endl;
      ++current_file_index_;
    }
  }
  // Return failure
  return false;
  
}

// Get next sample in file
bool MultiFileHandler::next(int16_t& sample) {

  // Reutnr false if file is not open
  if (!current_stream_.is_open()) return false;

  // Get the next sample
  current_stream_.read(reinterpret_cast<char*>(&sample), sizeof(int16_t));

  // If the next file is a int16_t
  if (current_stream_.gcount() == sizeof(int16_t)) {
    // Increment number of samples
    ++samples_read_;
    // And return true
    return true;
  }
  // End of current file; move to next
  else {
    // Close current file
    current_stream_.close();
    // Increment file index
    ++current_file_index_;
    // Open next file
    if (open_next_file()) {
      // Recursively fetch from the next file
      return next(sample);
    }
    // Else, end of files
    else {
      // No more files
      return false;
    }
  }
}

// Get the number of files
size_t MultiFileHandler::file_count() const {
    return filenames_.size();
}

// Get the total number of samples
size_t MultiFileHandler::total_samples() const {
    return total_samples_;
}

// Get the number of read samples
size_t MultiFileHandler::total_samples_read() const {
    return samples_read_;
}
