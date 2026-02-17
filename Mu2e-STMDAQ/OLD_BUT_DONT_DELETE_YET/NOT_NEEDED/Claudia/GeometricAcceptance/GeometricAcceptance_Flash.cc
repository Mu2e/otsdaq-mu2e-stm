//Make executable: g++ GeometricAcceptance.cc -o GeometricAcceptance.exe
//Run: ./GeometricAcceptance.exe "try.txt" "out.txt"
//Run locally where data is (input_path) //

#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include <random>
#include <math.h>

#define PI 3.14159265358979323846  /* pi */

void GeometricAcceptance(int argc, char *argv[], std::string input_path, std::string  output_path){
  //Open txt
  std::fstream readfile;
  std::fstream writefile;
  readfile.open(input_path,std::ios::in);
  writefile.open(output_path,std::ios::out);

  double row, px, py, pz, p, x_ST, y_ST, z_ST;
  double count_hitSTM=0.0;
  double count_nohit=0.0;

  double x_STM, y_STM, z_STM, x_centerSTM, y_centerSTM, r_STM, area_STM;
  //x_centerSTM = -3950;//VD=90
  x_centerSTM = -3850;//mm VD=89
  y_centerSTM = 0;
  z_STM = 40750; //mm fixed
  r_STM = 45; //mm
  area_STM = 3.1415*(r_STM/10)*(r_STM/10);//cm2
  std::cout<<"Area STM: "<<area_STM<<"cm2"<<" center STM (x,y,z): ("<<x_centerSTM<<", "<<y_centerSTM<<", "<<z_STM<<")mm"<<" r STM: "<<r_STM<<"mm"<<std::endl;

  //Read each art file from txt
  while(1){
    readfile>>row;
    readfile>>px;
    readfile>>py;
    readfile>>pz;
    readfile>>p;
    readfile>>x_ST;
    readfile>>y_ST;
    readfile>>z_ST;

    double theta = acos(pz/p);
    double phi = atan(py/px);

    //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
    if((px<0)&&(py>0)){
      //these angles are negative angles (-phi) in the 4th quadrant that should be in 2nd quadrant
      phi = PI + phi;
      //std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
    }
    if((px<0)&&(py<0)){
      //These are positive angles in the 1st quadrant that should be in 3rd quadrant
      phi = PI + phi;
      //std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
    }
    //Conversion so that 4th quadrant returns positive angles
    //Conversion from 1st and 4th quadrants to 2nd and 3rd, the atan just return values in 1st and 4th
    if((px>0)&&(py<0)){
      //these angles are negative angles (-phi) in the 4th quadrant that should be positive angles in the 4th quadrant
      phi = 2*PI + phi;
      //std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;
    }
    //std::cout<<"px: "<<px<<" py: "<<py<<" phi: "<<phi<<std::endl;

    x_STM = x_ST+tan(theta)*cos(phi)*(z_STM-z_ST);
    y_STM = y_ST+tan(theta)*sin(phi)*(z_STM-z_ST);


    if(readfile.eof())break;
    //std::cout<<"theta: "<<theta<<" phi: "<<phi<<std::endl;
    //std::cout<<row<<" "<<px<<" "<<" "<<py<<" "<<pz<<" "<<x_ST<<" "<<y_ST<<" "<<z_ST<<" "<<"STM: x: "<<x_STM<<" y: "<<y_STM<<std::endl;

    if(sqrt((x_STM-x_centerSTM)*(x_STM-x_centerSTM)+(y_STM-y_centerSTM)*(y_STM-y_centerSTM))<=r_STM){
      writefile<<row<<" ST: px: "<<px<<" py: "<<py<<" pz: "<<pz<<" p: "<<p<<" x: "<<x_ST<<" y: "<<y_ST<<" z: "<<z_ST<<" -hit STM: x: "<<x_STM<<" y: "<<y_STM<<" theta: "<<theta<<" phi: "<<phi<<std::endl;
      count_hitSTM++;
    }
    else{
      writefile<<row<<" ST: px: "<<px<<" py: "<<py<<" pz: "<<pz<<" p: "<<p<<" x: "<<x_ST<<" y: "<<y_ST<<" z: "<<z_ST<<" don't hit STM: x: "<<x_STM<<" y: "<<y_STM<<" theta: "<<theta<<" phi: "<<phi<<std::endl;
      count_nohit++;
    }

  }

  std::cout<<"Number of bremsstrahlung photons generated at ST: "<<row<<std::endl;
  std::cout<<"Number of hits in the STM: "<<count_hitSTM<<std::endl;
  std::cout<<"Number of hits outside: "<<count_nohit<<std::endl;
  double sum = count_hitSTM+count_nohit;
  std::cout<<"Total hits check: "<<sum<<std::endl;
  double acceptance = count_hitSTM/row;
  std::cout<<"Acceptance : "<<acceptance<<std::endl;
  double acceptancecm2 = acceptance/area_STM;
  std::cout<<"Acceptance/cm2 : "<<acceptancecm2<<std::endl;
  std::cout<<" "<<std::endl;
  std::cout<<"File created: "<<output_path<<std::endl;

  readfile.close();
  writefile.close();

}

int main(int argc, char *argv[]){

  // argv[0]=program, argv[1]= input px,py,pz,x,y,z, argv[2]=output (px,py,pz,x,y,z,hit the STM)
  std::string input_path = std::string(argv[1]);
  std::string output_path = std::string(argv[2]);

  GeometricAcceptance(argc,argv,input_path,output_path);

  return 0;
}
