#include<iostream>
#include<fstream>
#include<vector>
#include <chrono>
#include <ctime>

#include "/work/cgarcia/STMDAQ-TestBeam/utils/DateTime.hh"


using namespace std::chrono;

int main(void){

  std::ofstream myfile;
  myfile.open ("DataRuns.txt");

  int NumRuns = 14;

  //Name of files 15-28 (include unix time)
  std::string run_names[NumRuns] = {"stm_hzdr_raw__run0000015_subrun00000.bin", "stm_hzdr_raw__run0000016_subrun00000.bin", "stm_hzdr_raw__run0000017_subrun00000.bin", "stm_hzdr_raw__run0000018_subrun00000.bin", "stm_hzdr_raw__run0000019_subrun00000.bin", "stm_hzdr_raw__run0000020_subrun00000.bin", "stm_hzdr_raw__run0000021_subrun00000.bin","stm_hzdr_raw__run0000022_subrun00000.bin", "stm_hzdr_raw__run0000023_subrun00000.bin", "stm_hzdr_raw__run0000025_subrun00000.bin", "stm_hzdr_raw__run0000026_subrun00000.bin", "stm_hzdr_raw__run0000027_subrun00000.bin", "stm_hzdr_raw__run0000028_subrun00000.bin", "stm_hzdr_raw__run0000028_subrun00001.bin"};

  //unix time
  time_t First_trig_time[NumRuns];
  First_trig_time[0]=1650800131.036;//15
  First_trig_time[1]=1650804234.636;//16
  First_trig_time[2]=1650807194.697;//17
  First_trig_time[3]=1650811427.071;//18
  First_trig_time[4]=1650812017.536;//19
  First_trig_time[5]=1650814001.368;//20
  First_trig_time[6]=1650816584.627;//21
  First_trig_time[7]=1650821236.226;//22
  First_trig_time[8]=1650822616.064;//23
  First_trig_time[9]=1650825103.392;//25
  First_trig_time[10]=1650826350.604;//26
  First_trig_time[11]=1650828113.310;//27
  First_trig_time[12]=1650829402.700;//28_00
  First_trig_time[13]=1650831115.188;//28_01
  //unix time
  time_t Second_trig_time[NumRuns];
  Second_trig_time[0]=1650800131.036;
  Second_trig_time[1]=1650804263.520;
  Second_trig_time[2]=1650807202.135;
  Second_trig_time[3]=1650811428.655;
  Second_trig_time[4]=1650812017.536;
  Second_trig_time[5]=1650814001.368;
  Second_trig_time[6]=1650816586.179;
  Second_trig_time[7]=1650821252.354;
  Second_trig_time[8]=1650822632.192;
  Second_trig_time[9]=1650825122.535;
  Second_trig_time[10]=1650826366.711;
  Second_trig_time[11]=1650828130.394;
  Second_trig_time[12]=1650829418.780;
  Second_trig_time[13]=1650831124.232;
    
  std::string Nint[NumRuns] ={"199","39","57","27","50","109","163","17", "51", "45", "53","28", "97","40" };
  std::string Next[NumRuns] ={"93","39","58","28","51","105","163","18", "52", "46", "54","28", "97","40" };

  // convert now to string form
  for(int i=0; i<NumRuns; i++){

  char* tt1 = ctime(&First_trig_time[i]);
  std::cout << "Trigger starts: " << tt1;

  myfile << run_names[i] <<std::endl<< tt1;

  char* tt2 = ctime(&Second_trig_time[i]);
  std::cout << "Trigger ends: "<< tt2 <<std::endl;

  myfile << tt2 << Nint[i] <<std::endl<<Next[i]<<std::endl;
  }
  
  



  //Another way of doing it
  for(int i=0; i<NumRuns; i++){
    
    DateTime dt1(int(First_trig_time[i]));
    DateTime dt2(int(Second_trig_time[i]));

    std::string dtt1= dt1.TimeFull();
    std::string dtt2= dt2.TimeFull();

    std::cout << dtt1 << dtt2; 

    std::string dtt1_format= dt1.Format(7);
    std::string dtt2_format= dt2.Format(7);


    std::cout << dtt1_format << std::endl;
    std::cout << dtt2_format << std::endl;
    std::cout<<std::endl;
      }

  

  /*std::time_t epoch_timestamp = 1650800131036;
  char buf[80];

  std::tm *ts = std::gmtime(&epoch_timestamp);
  strftime(buf, sizeof(buf), "%m/%d/%Y %H:%M:%S", ts);
  std::cout << buf << std::endl;
  //seconds
  time_t hi[1];
  hi[0]=1650800131.036;
  char* hi2 = ctime(&hi[0]);
  std::cout << "Trigger ends: "<< hi2 <<std::endl;
  */


  myfile.close();
  return 0;

}
