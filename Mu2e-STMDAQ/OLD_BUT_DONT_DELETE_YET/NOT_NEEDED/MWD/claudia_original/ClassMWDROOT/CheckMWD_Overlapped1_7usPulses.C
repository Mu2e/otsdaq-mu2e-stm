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



void CheckMWD_Overlapped1_7usPulses(std::string path1, std::string path2) {


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

  double window_us = 1.7;
  int counter = 0;
  
  //Compare vectors
  for(int i= 0; i< Nfiles; i++){

    std::cout<<"FIlE NUMBER: "<<i<<std::endl;
    
    for(unsigned long int j=0; j< Xraytimes[i].size(); j++){

      double window_number = int(Xraytimes[i].at(j) / window_us);
      double window_start = double(window_number * window_us);
      double window_end = double(window_start + window_us);
      double time = Xraytimes[i].at(j);
      std::cout<<" "<<std::endl;
    
      std::cout<<"X-ray peak : "<<std::setprecision(8)<<Xraytimes[i].at(j)<<" us "<<Xrayenergies[i].at(j)<<" keV"<<std::endl;
      std::cout<<"In window number "<<std::setprecision(8)<<window_number<<": from "<<window_start<<" to "<<window_end<<" us"<<std::endl;

      int k = 0;

      while(Flashtimes[i].at(k) <= window_end){

	if( Flashtimes[i].at(k) > window_start){
	  double difference = abs(Flashtimes[i].at(k) - Xraytimes[i].at(j));
	  std::cout<<"Flash in the same  window, time: "<<Flashtimes[i].at(k)<<" us "<<Flashenergies[i].at(k)<<" keV"<<std::endl; 
	  std::cout<<"Difference: "<<difference<<std::endl;
	  std::cout<<"MISSED PEAKS: "<<counter<<std::endl;
	  std::cout<<""<<std::endl;
	  counter++;
	}
	k++;
      }
      
    }//j
    std::cout<<"Number of missed 347 keV: "<<counter<<std::endl;
    std::cout<<"end of file"<<std::endl;
  }//i


  std::cout<<"Number of 347keV not reconstructed: "<<counter<<std::endl;
}
