#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <fstream>
#include <cstdio>

#include <stdio.h>
#include <stdlib.h>


#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TAxis.h"
#include "TH1F.h"



void CheckMWDtimes(std::string path1, std::string path2) {


  int Nfiles = 10;

  std::vector<double> Xraytimes[10], Flashtimes[10];
  std::vector<double> Xrayenergies[10], Flashenergies[10];
  
  for(int i= 0; i< Nfiles; i++){

    std::string filename = path1+"MWD_job0"+std::to_string(i)+".log";
    
    std::ifstream myFile(filename);

    // Make sure the file is open
    if(!myFile.is_open()) throw std::runtime_error("Could not open file");
    std::cout<<"File: "<<filename<<std::endl;
    
    std::string line;
    double time, energykeV;
    std::string val;
    int linecount =0 ;
    // Read data, line by line 
    while(std::getline(myFile, line))
    {
      if(linecount < 9){linecount++;}

      else{
        std::stringstream ss(line);
	
        ss >> val; ss >> val; ss >> val;
	ss >> time;
	ss >> val; ss >> val; ss >> val; ss >> val; ss >> val;
	ss >> energykeV;
	if(time != 0){
	  //std::cout<<time <<" "<<energykeV<<std::endl;
	  Xraytimes[i].push_back(time);
	  Xrayenergies[i].push_back(energykeV);
	}
	linecount++;
      }
      
    }
    
    // Close file
    myFile.close();
  }


  for(int i= 0; i< Nfiles; i++){

    std::string filename = path2+"MWD_job0"+std::to_string(i)+".log";

    std::ifstream myFile(filename);

    if(!myFile.is_open()) throw std::runtime_error("Could not open file");
    std::cout<<"File: "<<filename<<std::endl;
    
    std::string line;
    double time, energykeV;
    std::string val;
    int linecount =0 ;
    while(std::getline(myFile, line))
    {
      
      if(linecount < 9){linecount++;}
      
      else{
        std::stringstream ss(line);

        ss >> val; ss >> val; ss >> val;
        ss >> time;
        ss >> val; ss >> val; ss >> val; ss >> val; ss >> val;
        ss >> energykeV;
        if(time != 0){
          //std::cout<<time <<" "<<energykeV<<std::endl;
          Flashtimes[i].push_back(time);
          Flashenergies[i].push_back(energykeV);
        }
        linecount++;
      }

    }

    
    myFile.close();
  }


  std::cout<<"Compare..."<<std::endl;

  double peak_limit[2] = {344, 350};
  //double peak_limit[2] = { 344, 370};
  
  unsigned long int counter = 0;
  std::vector<double> foundtimes, foundpositions;
  //Compare vectors
  for(int i= 0; i< Nfiles; i++){

    std::cout<<"FIlE NUMBER: "<<i<<std::endl;
    
    for(unsigned long int j=0; j< Xraytimes[i].size(); j++){

      double time = Xraytimes[i].at(j);
      std::cout<<" "<<std::endl;
      std::cout<<"new finding..."<<std::endl;
      std::cout<<"element to find: "<<Xraytimes[i].at(j)<<" "<<Xrayenergies[i].at(j)<<std::endl;
      
      auto start = std::lower_bound(Flashtimes[i].begin(), Flashtimes[i].end(), time-0.2);
      auto end = std::upper_bound(Flashtimes[i].begin(), Flashtimes[i].end(), time+0.2);

      foundtimes.clear();
      foundpositions.clear();
      
      for (auto it = start; it != end; it++) {

	//std::cout << std::endl;

	foundtimes.push_back(*it);
	double index = it - Flashtimes[i].begin();
	foundpositions.push_back(index);
	std::cout<<"found time: "<<*it<<" pos: "<<index<<std::endl;
      }//it

      unsigned long int size = foundtimes.size();
      std::cout<<"Number of peaks found at this times: "<<size<<std::endl;

      int counted = 0;
      if(size !=0){
      for(unsigned long int b =0; b < size; b++){
	
	// If element was found
	// calculating the index
	double pos = foundpositions.at(b);
	std::cout<<"element found number "<<b<<" "<<Flashtimes[i].at(pos)<<" "<<Flashenergies[i].at(pos)<<std::endl;
	//outside the peak range increment the counter
	if( (size==1)&&((Flashenergies[i].at(pos) < peak_limit[0])||(Flashenergies[i].at(pos) > peak_limit[1])) ){ counter++;std::cout<<"ERROR"<<std::endl; }

	if( (size>1)&&(counted!=0) ){std::cout<<"Don't count this peak"<<std::endl;}
	//inside the peak range
	if( (size>1)&&((Flashenergies[i].at(pos) > peak_limit[0])||(Flashenergies[i].at(pos) < peak_limit[1])) ){ std::cout<<"Don't count this peak"<<std::endl; counted++; }

	//Just count the missing peak in the las peak
	if( (size>1)&&(counted==0)&&(b==(size-1)) ){counter++;std::cout<<"ERROR"<<std::endl;}

	
      }//if size
      }//b 
      else{std::cout<<"This time wasn't found"<<std::endl; counter++;}

      std::cout<<"Number of 347keV not reconstructed until here...: "<<counter<<std::endl;
    }//j
    std::cout<<"end of file"<<std::endl;
  }//i


  std::cout<<"Number of 347keV not reconstructed: "<<counter<<std::endl;
}
