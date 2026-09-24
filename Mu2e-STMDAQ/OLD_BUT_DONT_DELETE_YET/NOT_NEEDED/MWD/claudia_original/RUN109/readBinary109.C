#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>

#include "TGraph.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TF1.h"
#include "TTree.h"
#include "TFile.h"
#include "TLegend.h"
#include "TLine.h"


using namespace std;

void readBinary109() {
  auto c1= new TCanvas("c1","Title",400,10,1500,500);

  std::vector<int16_t> ADC;
  ADC.clear();
  std::ifstream myFile("../../../DATA/MWD_Analysis/RUN109/run00109.bin_01", ios::in | ios::binary);
  int16_t inf ;
  while( myFile.read( reinterpret_cast<char*>( &inf ), sizeof(inf) ) ){
    ADC.push_back(inf);
  }


  double xx1[2]={0,5000000};
  double yy1[2]={-2500,5000};
  TGraph *graph1 = new TGraph (2,xx1,yy1);
  graph1->GetXaxis()->SetRangeUser(0,5000000);
  graph1->GetYaxis()->SetRangeUser(-2500,8000);
  graph1->SetTitle("Run00109-BinaryFile");
  graph1->GetXaxis()->SetTitle("Time (ns)");
  graph1->GetYaxis()->SetTitle("ADC Counts");

  //TGraph* gr = new TGraph();
  std::vector<double> time;
  time.clear();
  double t=0;

  unsigned long n=ADC.size();
  //unsigned long n=100000000;

  cout<<ADC.size()<<endl;
  for(unsigned long i=0; i < n; i++){
    time.push_back(t);
    //gr->SetPoint(i,t,ADC.at(i));
    t+=2.7;
  }

  //graph1->Draw("ap");

  //gr->SetLineColor(kBlack);
  //gr->SetMarkerStyle(5);
  //gr->Draw("same");


  ////////////////////////////////     MWD Algorithm   //////////////////////////////////////////////////
  //TGraph* dec = new TGraph();
  // TGraph* dif = new TGraph();
  //TGraph* av = new TGraph();

  bool plotMWD, plotEnergy;
  int number;
  cout<<"Enter 1 for plotting Signal+MWD, 2 for plotting Signal+EnergyPeaks "<<endl;
  cin>>number;
  plotMWD=false;
  plotEnergy=false;
  if(number==1){plotMWD=true;}
  if(number==2){plotEnergy=true;}




  //Deconvolution
  double tau, T0;

  tau=65544.1;
  T0=2.7;

  vector<double> a;
  a.clear();

  a.push_back(ADC.at(0));
  //dec->SetPoint(0,time.at(0),a.at(0));
  for(unsigned long i=1; i<n; i++){
    a.push_back(ADC.at(i)-(1-(T0/tau))*ADC.at(i-1)+a.at(i-1));
    //cout<<a[i]<<endl;
    //dec->SetPoint(i,time.at(i),a.at(i));
  }

  //Differentiation

  vector<double> D;
  D.clear();
  int M=8000;

  for (int i = 0; i < M; ++i) {
    D.push_back(a.at(i));
    //dif->SetPoint(i,time.at(i),D.at(i));
  }

  for (unsigned long i = M; i < n; ++i) {
    D.push_back(a.at(i)-a.at(i-M));
    //  dif->SetPoint(i,time.at(i),D.at(i));
  }

  //Averaging

  vector<double> l;
  l.clear();
  int L=1000;
  double sum = 0.;

  for (int i = 0; i < L-1; ++i) {
    sum += D.at(i);
    l.push_back(D.at(i));
    //cout<<l.at(i)<<endl;
    // av->SetPoint(i,time.at(i),l.at(i));
  }
  sum += D.at(L-1);
  l.push_back(sum/L);

  for (unsigned long i = L; i < n; ++i) {
    l.push_back(sum/L);
    //if(l.at(i)<0){cout<<l.at(i)<<endl;}
    sum += D.at(i)-D.at(i-L);
    //av->SetPoint(i,time.at(i),l.at(i));
  }


  if(plotMWD==true){
    /*graph1->Draw("ap");

    gr->SetLineColor(kBlack);
    gr->SetMarkerStyle(5);
    gr->Draw("same");

    dec->SetMarkerColor(kRed);
    dec->SetLineColor(kRed);
    dec->SetMarkerStyle(5);
    dec->Draw("same");

    dif->SetMarkerColor(kGreen);
    dif->SetLineColor(kGreen);
    dif->SetMarkerStyle(5);
    dif->Draw("same");

    av->SetMarkerColor(kBlue);
    av->SetLineColor(kBlue);
    av->SetMarkerStyle(5);
    av->Draw("same");

    //Line at t=54756.2ns
    //TLine *linet0=new TLine(54756.2 ,-1000,54756.2 ,1000);
    //linet0->Draw("same");
    //Line at t=79059.4 ns
    //TLine *linet1=new TLine(79059.4 ,-1000,79059.4 ,1000);
    //linet1->Draw("same");

    //TLine *linet2=new TLine(1.08015e+06 ,-2000,1.08015e+06 ,1000);
    //linet2->Draw("same");
    //TLine *linet3=new TLine(1.10485e+06 ,-2000,1.10485e+06 ,1000);
    //linet3->Draw("same");

    //TLine *linet4=new TLine(1.24839e+06 ,-2000,1.24839e+06 ,1000);
    //linet4->Draw("same");
    //TLine *linet5=new TLine(1.27055e+06 ,-2000,1.27055e+06 ,1000);
    //linet5->Draw("same");


    auto leg1 = new TLegend(0.1,0.7,0.48,0.9);
    leg1->AddEntry(gr, "Signal","p");
    //leg1->AddEntry(dec, "Deconvolution","p");
    leg1->AddEntry(dif, "M-Step Differentiation","p");
    leg1->AddEntry(av, "Moving Window Average","p");
    leg1->Draw("same");*/
  } //if plotMWD








  ////////////////////////////////     Gradient  //////////////////////////////////////////////////

  //Plotting energy
  TH1F*h1 = new TH1F("TH1","", 100, -3000, 0);
  //Plotting the distribuition of the gradient
  TH1F*h2 = new TH1F("TH2","", 100, -0.5, 0.5);
  //Plotting the gradient
  //TGraph* grad = new TGraph();
  double gradient, gradient1, gradient2;

  if(plotEnergy==true){

    //Plotting the gradient distribuition of a region to see the mean of the gradient
    for(int j = 64000; j < 500000; ++j){
      gradient1=l.at(j-1)-l.at(j);
      h2->Fill(gradient1);}
    h2->SetTitle("Gradient distribution");
    double mean= h2->GetMean();
    double sigma= h2->GetRMS();
    double threesigmas=4*sigma;
    cout<<"Gradient mean and sigma: "<<mean<<"  "<<sigma<<endl;
    //Gradient mean and sigma: 4.13078e-05  0.0254788

    //h2->Draw();



    //Uncomment to plot the gradient
    /*double auxup=0, timeup=0;
   double auxlow=0, timelow=0;
   //1000- n
   for(int i = 1000; i < n; ++i){
   //for(int i = 481481; i < 555555; ++i){
     gradient=l.at(i-1)-l.at(i);
     grad->SetPoint(i,time.at(i),gradient);
       if(gradient<auxlow){auxlow=gradient;
       timelow=time.at(i);}
       if(gradient>auxup){auxup=gradient;
       timeup=time.at(i);}
   }
   cout<<"Empieza el pulso: "<<timeup<<" Termina el pulso: "<<timelow<<endl;
   cout<<"Duracion del pulso: "<<timelow-timeup<<" ns"<<endl;
   grad->GetXaxis()->SetRangeUser(200000,30000000);
   //grad->GetXaxis()->SetRangeUser(1300000,1500000);
   grad->GetYaxis()->SetRangeUser(-2,2);
   //grad->GetXaxis()->SetRangeUser(40000,100000);
   //grad->GetYaxis()->SetRangeUser(-0.6,0.6);
   grad->SetLineColor(kOrange);
   grad->SetMarkerColor(kOrange);
   grad->GetXaxis()->SetTitle("Time (ns)");
   //grad->GetYaxis()->SetTitle("ADC Counts");
   grad->Draw();

   TLine *linenoiseup=new TLine(200000,mean+threesigmas,30000000,mean+threesigmas);
   linenoiseup->Draw("same");

   TLine *linenoisedown=new TLine(200000,mean-threesigmas,30000000,mean-threesigmas);
   linenoisedown->Draw("same");

   TLine *linepulso=new TLine(timeup,auxlow,timelow,auxlow);
   linepulso->SetLineColor(kBlue);
   //linepulso->Draw("same");

   TLine *linepulsovert=new TLine(timeup,auxlow,timeup,auxup);
   linepulsovert->SetLineColor(kBlue);
   //linepulsovert->Draw("same");

   auto leg2 = new TLegend(0.1,0.7,0.48,0.9);
   //leg2->AddEntry(grad, "Voltage gradient","l");
   //leg2->AddEntry(linenoiseup, "4#sigma","l");
   leg2->AddEntry(linepulso, "Pulse duration ~ 22323.6 ns","l");
   leg2->Draw("same");*/




    //Uncomment to plot the energy peaks
    //graph2->Draw("ap");

    double auxup=0, timeup=0;
    double auxlow=0, timelow=0,voltagelow=0;
    double tup[8000], tlow[8000],e1[8000],e2[8000], energy[8000];
    int counterpeak=0;

    //low limit 1000 calculamos los extremos de los gradientes de los picos
    for(unsigned long i = 1000; i < n; ++i){
      gradient=l.at(i-1)-l.at(i);
      //cout<<"Gradient: "<<gradient<<endl;
      //Encuentra el maximo valor del gradiente auxlow
      if(gradient>(mean+threesigmas)&&gradient>auxlow){auxlow=gradient;
	timelow=time.at(i);
	voltagelow=l.at(i);
	//cout<<"low grad "<<auxlow<<endl;
      }

      //Se queda con el valor para el que la diferencia entre tiempos timelow y up sea entre 18000 y 22000 (el primer valor del gradiente por debajo de mean-threesigmas)
      if(gradient<(mean-threesigmas)&&gradient<auxup){auxup=gradient;
	timeup=time.at(i);
	//cout<<"up grad "<<auxup<<endl;
	//voltageup=l.at(i);
      }

      //Descarta el resto de valores de gradiente por debajo de mean-threesigmas
      if(auxup<(mean-threesigmas)&&auxlow==0){auxup=0;
	continue;}

      //cout<<"up "<<auxup<<" "<<timeup<<endl;

      if(timeup-timelow>18000&&timeup-timelow<26000){
	tup[counterpeak]=timeup;
	tlow[counterpeak]=timelow;
	//cout<<counterpeak<<" Start peak "<<tlow[counterpeak]<<" Finish peak "<<tup[counterpeak]<<endl;
	//cout<<" High Gradient "<<auxlow<<" Little Gradient "<<auxup<<endl;
	e1[counterpeak]=voltagelow;
	//cout<<e1[counterpeak]<<endl;

	auxup=0;
	auxlow=0;
	timelow=0;
	timeup=0;
	voltagelow=0;
	counterpeak++;
      }
    }

    //media de los puntos azules que no forman parte de los picos
    TH1F*h4 = new TH1F("TH4","", 500, 0, 500);

    /* for(int i=0;i<counterpeak;i++){
     for(int j = 1000; j < n; ++j){
       if(time.at(j)>tlow[i]&&time.at(j)<tup[i]){continue;}
       else{h4->Fill(l.at(j));}
     }//for
     }//i*/

    for(unsigned long j = 64000; j < 208000; ++j){
      h4->Fill(l.at(j));}
    //Plot the baseline of the energy
    double thresholdE=h4->GetMean();
    cout<<"meanthreshold "<<thresholdE<<endl;




        std::string rootname= "run_00109_energypeaks1.root";
        cout<<"new file"<<endl;
        //Creo el .root en el que se van a guardar los picos del voltaje
        TFile *rootfile=new TFile(rootname.c_str(),"recreate");


        TTree*tree=new TTree("treeADC","treeADC");
        double peaks;
        tree->Branch("peaks",&peaks);


   //Calculamos el minimo valor de los picos y le restamos el baseline (thresholdE)
   for(int i=0;i<counterpeak;i++){
     for(unsigned long j = 1000; j < n; ++j){
       if(time.at(j)>tlow[i]&&time.at(j)<tup[i]){
            if(l.at(j)<l.at(j-1)){e2[i]=l.at(j);}
       }//if
     }//j
     //e2 es el valor minimo de los picos y el valor maximo es para todos el mismo
     energy[i]=e2[i]-thresholdE;
     peaks=energy[i];
     tree->Fill();
     h1->Fill(energy[i]);

   }//i

   //i es el numero del pulso, counterpeak el numero total de pulsos y los tiempos
   //de cada pulso estan guardados en los vectores tlow y tup
   //Por ejemplo para el primer pulso los tiempos serian tlow[0] para cuando empieza y tup[0] para cuando acaba
   for(int i = 0; i < counterpeak; ++i){cout<<i<<" Start peak "<<tlow[i]<<" Finish peak "<<tup[i]<<" Energy "<<energy[i]<<endl;}
   h1->GetXaxis()->SetTitle("ADC Counts");

   //Fit Energy peak cessium
   //TF1*Fit = new TF1("Fit", "[0]*TMath::Gaus(x,[1],[2])", -1500, -1000);
   //Fit->SetParameters(27.3250,-1182.76,-28.0618);

   //Plot energy peaks
   h1->SetTitle("");
   h1->SetStats(0);
   h1->Draw("");


  h1->Write();
  rootfile->Write();
  rootfile->Close();
  h1->Reset();
  h2->Reset();
  h4->Reset();
   //h1->Fit(Fit,"0","",-1500, -1000);
   //Fit->SetLineColor(kRed);
   //Fit->Draw("same");

   } //if plotEnergy==true




    myFile.close();

}
