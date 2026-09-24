#ifndef PULSEFINDING_H
#define PULSEFINDING_H

#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <iostream>


class PulseFinding {
 public:
  std::vector<double> peaks(int n, std::vector<double> l, std::vector<double> time);

};





std::vector<double>
PulseFinding::peaks(int n,std::vector<double> l, std::vector<double> time){


  //Return mean gradient and 3sigmas gradient of a region (random region with enough points) 
  TH1F*hgrad = new TH1F("TH1","", 100, -0.5, 0.5);

  double gradient;
  
  for(int j = 64000; j < 500000; j++){
    gradient=l.at(j-1)-l.at(j);
    hgrad->Fill(gradient);}
    double mean=hgrad->GetMean();
    double threesigmas=3*(hgrad->GetRMS());
  

   //Time start and end of energy peaks
    double auxup=0, timeup=0;
    double auxlow=0, timelow=0;
    double tup[8000], tlow[8000],energy[8000], min[8000];
    int counterpeak=0;
    std::vector<double> peakE;
    
    for(int i = 1000; i < n; i++){
      gradient=l.at(i-1)-l.at(i);
      //max value of the gradient: auxlow
      if(gradient>(mean+threesigmas)&&gradient>auxlow){auxlow=gradient;
	timelow=time.at(i);
      }

      //min value of the gradient: auxup
      if(gradient<(mean-threesigmas)&&gradient<auxup){auxup=gradient;
	timeup=time.at(i);
      }

      //Descarta el resto de valores de gradiente por debajo de mean-threesigmas
      if(auxup<(mean-threesigmas)&&auxlow==0){auxup=0;
	continue;}

 
      //Se queda con el valor para el que la diferencia entre tiempos timelow y up sea entre 22000 y 26000 (el primer valor del gradiente por debajo de mean-threesigmas)
      if(timeup-timelow>22000&&timeup-timelow<26000){
	tup[counterpeak]=timeup;
	tlow[counterpeak]=timelow;

	auxup=0;
	auxlow=0;
	timelow=0;
	timeup=0;
	counterpeak++;
      }
    }


    //media de los puntos azules que no forman parte de los picos
    TH1F*hl = new TH1F("TH2","", 500, 0, 500);

    for(int i=0;i<counterpeak;i++){
     for(int j = 1000; j < n; ++j){
       if(time.at(j)>tlow[i]&&time.at(j)<tup[i]){continue;}
       else{hl->Fill(l.at(j));}
     }//for j
     }//for i

    //Baseline of the energy
    double thresholdE=hl->GetMean();


    //Calculamos el minimo valor de los picos y le restamos el baseline (thresholdE)
    for(int i=0;i<counterpeak;i++){
      for(int j = 1000; j < n; ++j){
	if(time.at(j)>tlow[i]&&time.at(j)<tup[i]){
	  if(l.at(j)<l.at(j-1)){min[i]=l.at(j);}
	}//if
      }//j
      //min[] es el valor minimo de los picos y el valor maximo es para todos el mismo
      energy[i]=min[i]-thresholdE;
      peakE.push_back(energy[i]);
    }//i



 hgrad->Reset();
 hl->Reset();

 return peakE;
}








#endif
