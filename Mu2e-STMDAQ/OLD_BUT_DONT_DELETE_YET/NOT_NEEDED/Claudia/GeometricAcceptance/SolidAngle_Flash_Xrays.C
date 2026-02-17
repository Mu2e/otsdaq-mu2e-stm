#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <math.h>
#include "TRandom3.h"
#include "TF1.h"
#include "TH1F.h"

#define PI 3.14159265358979323846  /* pi */
//seed 0 different seed for different values every time we run it
//seed 1 different seed for different values but same values every time we run it

void SolidAngle_Flash_Xrays(double seed, unsigned long int number_photons_sim){
  bool debugout = false;

  TH1F*h1 = new TH1F("TH1","", 100, -1, 3.5);

  gRandom->SetSeed(seed);

  std::cout<<"SEED: "<<seed<<std::endl;

  double count_Xray_hitSTM=0.0;
  double count_flashphot_hitSTM=0.0;
  double x_STM_Xrays, y_STM_Xrays, x_STM_flash, y_STM_flash;

  //Choose a fixed x,y,z to generate the photons at the ST and generate the X-Rays (semicircle) and Flash photons (sphere)
  double x_ST = -3910; //mm
  double y_ST = 0; //mm
  double z_ST = 5800; //mm

  //center of the STM is fixed, use the one behind VD=89
  double x_centerSTM=-3850; //mm
  double y_centerSTM=0; //mm
  double z_centerSTM=40750; //mm
  double r_STM=45; //mm
  double area_STM = PI*(r_STM/10)*(r_STM/10);//cm2
  std::cout<<"Area STM: "<<area_STM<<"cm2"<<" center STM (x,y,z): ("<<x_centerSTM<<", "<<y_centerSTM<<", "<<z_centerSTM<<")mm"<<" r STM: "<<r_STM<<"mm"<<std::endl;

  double theta_min = 0;
  double theta_max = 0.003;
  double costheta_min = cos(theta_max);
  double costheta_max = cos(theta_min);
  double phi_min = 0;
  double phi_max = 2*PI;
  std::cout<<"Photons generated with cos(theta)["<<costheta_min<<","<<costheta_max<<"], theta: ["<<theta_min<<","<<theta_max<<"] phi: ["<<phi_min<<","<<phi_max<<"]rad: "<<number_photons_sim<<std::endl;

  //Total Xrays in the sphere
  double surface_area = (-cos(theta_max)+cos(theta_min))*(phi_max-phi_min);
  double total_Xrays = 4*PI*number_photons_sim/surface_area;
  std::cout<<"Xrays generated in the whole sphere area: ("<<surface_area<<"): "<<total_Xrays<<std::endl;

  //Total Flash Photons in the sphere.
  //Flash photons with cos(theta) between: [0.999996,1]rad: 188984
  //Rest of events: 26352260
  //total photons: 26540988
  double total_flashphot = number_photons_sim*26540988/188984;
  std::cout<<"Flash photons generated in the whole sphere area: "<<total_flashphot<<std::endl;
  std::cout<<""<<std::endl;


  for (unsigned long int i=0; i < number_photons_sim; i++){

    //Xrays
    double phi_Xrays = gRandom->Uniform(phi_min, phi_max); //rad
    //opc1: uniform in theta
    //double theta_Xrays = gRandom->Uniform(theta_min, theta_max); //rad
    //double costheta_Xrays = cos(theta_Xrays);
    //opc2: uniform in costheta
    double costheta_Xrays = gRandom->Uniform(costheta_min, costheta_max);
    double theta_Xrays = acos(costheta_Xrays); //rad
    h1->Fill(theta_Xrays);


    x_STM_Xrays = x_ST+tan(theta_Xrays)*cos(phi_Xrays)*(z_centerSTM-z_ST);
    y_STM_Xrays = y_ST+tan(theta_Xrays)*sin(phi_Xrays)*(z_centerSTM-z_ST);

    //Flash photons
    double phi_flash = gRandom->Uniform(phi_min, phi_max); //rad
    //double theta_flash = gRandom->Uniform(theta_min, theta_max); //rad
    //double costheta_flash = cos(theta_flash);
    double costheta_flash = gRandom->Uniform(costheta_min, costheta_max);
    double theta_flash = acos(costheta_flash); //rad

    x_STM_flash = x_ST+tan(theta_flash)*cos(phi_flash)*(z_centerSTM-z_ST);
    y_STM_flash = y_ST+tan(theta_flash)*sin(phi_flash)*(z_centerSTM-z_ST);

    if(debugout==true){
      std::cout<<"Xrays: "<<"phi: "<<phi_Xrays<<" cos(theta): "<<costheta_Xrays<<" theta: "<<theta_Xrays<<" rad, x_STM_Xrays: "
	       <<x_STM_Xrays<<" y_STM_Xrays: "<<y_STM_Xrays<<" mm"<<std::endl;

      std::cout<<"Flash photons: "<<"phi: "<<phi_flash<<" cos(theta): "<<costheta_flash<<" theta: "<<theta_flash<<" rad, x_STM_flash: "
	       <<x_STM_flash<<"y_STM_flash: "<<y_STM_flash<<" mm"<<std::endl;
      std::cout<<""<<std::endl;
    }

    //See if Xrays hit the STM
    if(sqrt((x_STM_Xrays-x_centerSTM)*(x_STM_Xrays-x_centerSTM)+(y_STM_Xrays-y_centerSTM)*(y_STM_Xrays-y_centerSTM))<=r_STM){
      count_Xray_hitSTM++;
      if(debugout==true){std::cout<<"Xray hit at STM"<<std::endl;}
    }
    //See if flash photons hit the STM
    if(sqrt((x_STM_flash-x_centerSTM)*(x_STM_flash-x_centerSTM)+(y_STM_flash-y_centerSTM)*(y_STM_flash-y_centerSTM))<=r_STM){
      count_flashphot_hitSTM++;
      if(debugout==true){std::cout<<"Flash photon hit at STM"<<std::endl;}
    }

  }
  std::cout<<"Number of Xrays hitting STM: "<<count_Xray_hitSTM<<std::endl;
  std::cout<<"X-ray acceptance (ST-STM(VD89)): "<<count_Xray_hitSTM/total_Xrays<<std::endl;
  std::cout<<""<<std::endl;
  std::cout<<"Number of Flash photons hitting STM: "<<count_flashphot_hitSTM<<std::endl;
  std::cout<<"Flash phot acceptance (ST-STM(VD89)): "<<count_flashphot_hitSTM/total_flashphot<<std::endl;
  h1->Draw();

}
