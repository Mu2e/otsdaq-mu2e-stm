#ifndef CSV_CLASS_H
#define CSV_CLASS_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream

class csv_utils {
 public:
  std::vector<std::pair<std::string, std::vector<int>>> read_csv(std::string filename);
  void write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<int>>> dataset);
  std::vector<int16_t> read_csv_values(std::string filename);
  std::vector<int16_t> read_csv_partvalues(std::string filename);

};




//Function 1: Reads the name of the column and store it in a string and reads all
//the data in a column and store it in a vector. Reads by column!
//Example of csv: One Two Three
//                1   2   3
//                1   2   3
//                1   2   3
//                1   2   3
std::vector<std::pair<std::string, std::vector<int>>>
  csv_utils::read_csv(std::string filename){

  // Reads a CSV file into a vector of <string, vector<int>> pairs where
  // each pair represents <column name, column values>

  // Create a vector of <string, int vector> pairs to store the result
  std::vector<std::pair<std::string, std::vector<int>>> result;

  // Create an input filestream
  std::ifstream myFile(filename);

  // Make sure the file is open
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

  // Helper vars
  std::string line, colname;
  int val;

  // Read the column names
  if(myFile.good())
    {
      // Extract the first line in the file
      std::getline(myFile, line);

      // Create a stringstream from line
      std::stringstream ss(line);

      // Extract each column name
      while(std::getline(ss, colname, ',')){

	// Initialize and add <colname, int vector> pairs to result
	result.push_back({colname, std::vector<int> {}});
      }
    }

  // Read data, line by line
  while(std::getline(myFile, line))
    {
      // Create a stringstream of the current line
      std::stringstream ss(line);

      // Keep track of the current column index
      int colIdx = 0;

      // Extract each integer
      while(ss >> val){

	// Add the current integer to the 'colIdx' column's values vector
	result.at(colIdx).second.push_back(val);

	// If the next token is a comma, ignore it and move on
	if(ss.peek() == ',') ss.ignore();

	// Increment the column index
	colIdx++;


      }
    }

  // Close file
  myFile.close();

  return result;
}


//Function 2. Write a csv by columns: with column name and column vector.
void
csv_utils::write_csv(std::string filename, std::vector<std::pair<std::string, std::vector<int>>> dataset){
  // Make a CSV file with one or more columns of integer values
  // Each column of data is represented by the pair <column name, column data>
  //   as std::pair<std::string, std::vector<int>>
  // The dataset is represented as a vector of these columns
  // Note that all columns should be the same size

  // Create an output filestream object
  std::ofstream myFile(filename);

  // Send column names to the stream
  for(int j = 0; j < dataset.size(); ++j)
    {
      myFile << dataset.at(j).first;
      if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
    }
  myFile << "\n";

  // Send data to the stream
  //dataset.at(0).second.size()=100
  for(int i = 0; i < dataset.at(0).second.size(); ++i)
    {
      //dataset.size()=3, accede a las columnas y escribe los vectores hacia abajo
      for(int j = 0; j < dataset.size(); ++j)
	{
          myFile << dataset.at(j).second.at(i);
          if(j != dataset.size() - 1) myFile << ","; // No comma at end of line
	}
      myFile << "\n";
    }

  // Close the file
  myFile.close();
}



//Function 3. Reads a csv file with values separated by comas
//Example of csv:
//1,2,3,4,5,6
//7,8,9,10,11,12
std::vector<int16_t>
csv_utils::read_csv_values(std::string filename){

  // Reads a CSV file into a vector<int>

  // Create a vector of ints to store the result
  std::vector<int16_t> result;

  // Create an input filestream
  std::ifstream myFile(filename);

  // Make sure the file is open
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

  // Helper vars
  std::string line;
  int val;

  // Read data, line by line
  while(std::getline(myFile, line))
    {
      // Create a stringstream of the current line
      std::stringstream ss(line);

      // Extract each integer
      while(ss >> val){

	// Add the current integer to the 'colIdx' column's values vector
	result.push_back(val);

	// If the next token is a comma, ignore it and move on
	if(ss.peek() == ',') ss.ignore();


      }
    }

  // Close file
  myFile.close();
  return result;
}


//No lee todo el csv completo
std::vector<int16_t>
csv_utils::read_csv_partvalues(std::string filename){

  // Reads a CSV file into a vector<int>

  // Create a vector of ints to store the result
  std::vector<int16_t> result;

  // Create an input filestream
  std::ifstream myFile(filename);

  // Make sure the file is open
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

  // Helper vars
  std::string line;
  int val;
  int i=0;

  // Read data, line by line
  while(std::getline(myFile, line)) //Este paso es el que tarda, lee todo el csv de golpe
    {

      // Create a stringstream of the current line
      std::stringstream ss(line);

      // Extract each integer
      //Lee solo los 139452881 primeros valores
      while(ss >> val&&i<139452881){ //no copiamos todos los valores en el vector de salida
	// Add the current integer to the 'colIdx' column's values vector
	result.push_back(val);
	std::cout<<val<<std::endl;
	// If the next token is a comma, ignore it and move on
	if(ss.peek() == ',') ss.ignore();
	i++;
      }
    }

  // Close file
  myFile.close();
  return result;
}











#endif
