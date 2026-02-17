#include "supalg.h"
 
int main () {

  in_stream_t s_in;
  out_stream_t s_out;
  
  //input text file
  /*int16_data ADC;
  ifstream inFILE;
  inFILE.open ("result.golden.dat");

  for (unsigned j = 0; j < n; j++) {
    inFILE >> ADC;
    s_in.write(ADC);
  }
  inFILE.close();
*/


  ulongint_data ntriggers_chunk=(chunk/fADC)/hardwareclock;
  cout<<"Number of triggers per chunk to send: "<<ntriggers_chunk<<endl;

  ulongint_data nADC_trigger=fADC*hardwareclock;
  cout<<"Number of ADC values per trigger: "<<nADC_trigger<<endl;

  ulongint_data nADC_chunk=nADC_trigger*ntriggers_chunk;
  //nADC_chunk=10000;
  cout<<"Number of ADC values per chunk: "<<nADC_chunk<<endl;

  //fill an array with number of ADC values (element array) per trigger (dimension array)
  int16_data ADC_hardtrig[ntriggers_chunk];
  for(int i=0;i<ntriggers_chunk;i++){
    ADC_hardtrig[i]=nADC_trigger;
    cout<<i<<" "<<ADC_hardtrig[i]<<endl;
  }

  ulongint_data nchunks=n/nADC_chunk;
  cout<<"Number of chunks: "<<nchunks<<endl;

  //Percentage where the overlapped chunk has to start at least
  double_data overlapped_chunk = double_data((nADC_chunk-1)-postnumADCstored+1-prenumADCstored)/nADC_chunk;
  cout<<nADC_chunk<<" "<<postnumADCstored<<" "<<prenumADCstored<<endl;
  cout<<overlapped_chunk<<endl;
  //Just one decimal
  overlapped_chunk = overlapped_chunk*10;
  overlapped_chunk = floor(overlapped_chunk);
  overlapped_chunk = overlapped_chunk/10;
  overlapped_chunk = 0.5;
  cout<<"Start of overlapping pulse with respect to the previous one: "<<overlapped_chunk<<endl;


  //Through warning
   if(prenumADCstored>((nADC_chunk*(1-overlapped_chunk))-(postnumADCstored-1))){std::cout<<"WARNING::::::: In the limit case where trigger is in (chunk-post-1), this trigger would be stored in next chunk but pre adc values chosen exceeds the start of the next chunk, PRE values stored must be lower::::::: ERROR"<<std::endl;exit(1);}



  ulongint_data ntriggers=0;
  ulongint_data totalnumtriggers=0;
  double_data topcalls=0;
  ulongint_data last_trigerstored=0;
  //input binary file
  int16_data* nADC = new int16_data[n];
  int16_data ADC[nADC_chunk];

  FILE *inFILE = fopen("run00109.new.bin_00", "rb");
  fread(nADC,n,2,inFILE);

  ofstream outFILE;

  outFILE.open ("output.dat");

 for(unsigned i=0;i<nchunks;i++){

   std::cout<<""<<endl;
   cout<<"Chunk goes from: "<<int(nADC_chunk*i)<<" to "<<int(nADC_chunk*i+(nADC_chunk-1))<<endl;

   for (unsigned j = 0; j < nADC_chunk; j++) {
     ADC[j]=nADC[i*nADC_chunk+j];
      s_in.write(ADC[j]);
    }

    topcalls=i;
    supalg(s_in,s_out,ntriggers,last_trigerstored,topcalls,nADC_chunk,ADC_hardtrig);

    cout<<"Number of triggers returned: "<<ntriggers<<endl;

  //number of triggers
  for (unsigned j = 0; j < ntriggers; j++) {

    out_stream_data result =s_out.read();    
    int16_data data_zp= result.data;
    double_data time_zp = result.time;
    std::cout<< "zp time: "<<time_zp<<std::endl;
    outFILE<<"hi1 "<<setprecision(9)<<time_zp<<endl;
    
  }
  totalnumtriggers +=ntriggers;
  cout<<"Total number of triggers: "<<totalnumtriggers<<endl;

  unsigned aux=nADC_chunk*(i+overlapped_chunk)+nADC_chunk;
  if(aux>=n){cout<<"index exceeds array size"<<endl; break;}
  
  std::cout<<""<<endl;
  cout<<"Chunk goes from: "<<unsigned(nADC_chunk*(i+overlapped_chunk))<<" to "<<unsigned(nADC_chunk*(i+overlapped_chunk)+(nADC_chunk-1))<<endl;
  //Starting in halfchunk
  for (unsigned j = 0; j < nADC_chunk; j++) {
    unsigned index= unsigned(nADC_chunk*(i+overlapped_chunk)+j);
    ADC[j]=nADC[index];
    s_in.write(ADC[j]);

  }

  topcalls=i+overlapped_chunk;
  supalg(s_in,s_out,ntriggers,last_trigerstored,topcalls,nADC_chunk,ADC_hardtrig);

  cout<<"Number of triggers returned: "<<ntriggers<<endl;

  //number of triggers 
  for (unsigned j = 0; j < ntriggers; j++) {
    
    out_stream_data result =s_out.read();
    int16_data data_zp= result.data;
    double_data time_zp = result.time;
    std::cout<< "zp time: "<<time_zp<<std::endl;
    outFILE<<"hi2 "<<setprecision(9)<<time_zp<<endl;

  }
  totalnumtriggers +=ntriggers;
  cout<<"Total number of triggers: "<<totalnumtriggers<<endl;
  
 
  }//nchunks

 fclose(inFILE);
 outFILE.close();



  // Compare the results file with the golden results
  int retval=0;
  retval = system("diff --brief -w output.dat triggers_00.dat");
  if (retval != 0) {
    cout << "Test failed  !!!" << endl; 
    retval=1;
  } else {
    cout << "Test passed !" << endl;
  }

  // Return 0 if the test
  //return retval;




  /*ifstream FILE_soft, FILE_hard;
  FILE_hard.open("output.dat");
  FILE_soft.open("triggers_00.dat");

  for (unsigned j = 0; j < 1605; j++) {
    double_data soft;
    double_data hard;
    FILE_soft>>soft;
    FILE_hard>>hard;
    cout<<soft<<" "<<hard<<endl;
    
  }
 FILE_hard.close();
 FILE_soft.close();
  */

  return 0;


}
