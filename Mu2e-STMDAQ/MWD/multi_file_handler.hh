#ifndef MULTI_FILE_HANDLER_HH
#define MULTI_FILE_HANDLER_HH

#include <vector>
#include <string>
#include <fstream>

class MultiFileHandler {
public:

  // Constructor: provide a vector of files directly
  MultiFileHandler(const std::vector<std::string>& files);
  
  // Get the next sample; returns true if successful, false if end of all files
  bool next(int16_t& sample);
  
  // Returns how many files were successfully loaded
  size_t file_count() const;
  
  // Total number of samples
  size_t total_samples() const;
  
  // Returns total number of samples seen so far
  size_t total_samples_read() const;

  // Static helper to load subrun files from a directory
  static void load_files_from_directory(const std::string& directory, std::vector<std::string>& out_files);
  
private:

  // Vector of filenames
  std::vector<std::string> filenames_;
  // Current fileindex
  size_t current_file_index_;
  // Current file
  std::ifstream current_stream_;
  // Total number of samples
  uint64_t total_samples_;
  // Number of samples read
  size_t samples_read_;
  
  // Helper to open the next file in the sequence
  bool open_next_file();

};

#endif // MULTI_FILE_HANDLER_HH
