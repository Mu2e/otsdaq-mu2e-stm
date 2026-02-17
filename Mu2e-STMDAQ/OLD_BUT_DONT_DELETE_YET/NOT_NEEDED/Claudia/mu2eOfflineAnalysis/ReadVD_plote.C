#include<iostream>
#include <string>
#include <fstream>
#include <vector>
#include <utility> // std::pair
#include <stdexcept> // std::runtime_error
#include <sstream> // std::stringstream
#include<fstream>
#include<cstdio>
#include<stdio.h>
#include<stdlib.h>
#include <iomanip> 
#include <random>

#include "TGraph.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TFile.h"
#include "TH1F.h"
#include "TLegend.h"
#include "TLine.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TPad.h"
#include "TSystem.h"
#include "TH2D.h"
#include "TPaveStats.h"
#include "TLatex.h"
#include "TGraph2D.h"
#include "TH3.h"
#include "TH3D.h"

#define mm_to_cm 10
#define PI 3.14159265358979323846  /* pi */
//================================================================
//This program reads readvd/ntvd or readvd/ntvext trees: Virtual detectors analyser
//Reads the root files from a txt and analyse the trees
//================================================================  

void PrintGraph(vector<string> file_name, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, int branch){

  bool debugout=false;

  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //gStyle->SetOptStat(1110);
  //Three significant digits
  //gStyle->SetStatFormat("6.3g");

  int palette_number = 55;
  if(branch==7){palette_number = 109;}
  if(branch==9){palette_number = 51;}
  gStyle->SetPalette(palette_number);
  //just for TH2D
  gStyle->SetPadRightMargin(0.14);
  //gStyle->SetPadRightMargin(0.1); 

  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode()*/;

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());

  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");

  TGraph *gr = new TGraph ();

  TGraph *grSTM1 = new TGraph ();
  TGraph *grcoll1 = new TGraph ();
  TGraph *grSTM2 = new TGraph ();
  TGraph *grcoll2 = new TGraph ();
  
  int Npoints = 100000;
  double xcenterSTM1 = -385.4;
  double xcenterSTM2 = -395.4;
  double xcentercoll1 = -386.34;
  double xcentercoll2 = -394.46;
  double Rcoll = 0.5642;
  double RSTM = 4.5;
  std::random_device dev;
  std::mt19937 rng(dev());
  std::uniform_real_distribution<double> distRSTM(0,RSTM);
  std::uniform_real_distribution<double> distRcoll(0,Rcoll);
  std::uniform_real_distribution<double> disttheta(0,2*PI);

  int b =0;
  
  for(int i = 0; i<Npoints; i++){
    double th = disttheta(rng);
    double costh = cos(th);
    double sinth = sqrt(1-(costh*costh));
    if(th>PI){sinth=(-1)*sinth;}
    double rSTM_ = distRSTM(rng);
    double rcoll_ = distRcoll(rng);
    
    double xSTM1 = xcenterSTM1+(rSTM_*costh);
    double xSTM2 = xcenterSTM2+(rSTM_*costh);
    double ySTM = rSTM_*sinth;

    double xcoll1 = xcentercoll1+(rcoll_*costh);
    double xcoll2 = xcentercoll2+(rcoll_*costh);
    double ycoll = rcoll_*sinth;

    //std::cout<<"costh "<<costh<<" sinth: "<<sinth<<" rSTM_: "<<rSTM_<<" rcoll_: "<<rcoll_<<std::endl;
    
    grSTM1->SetPoint(b, xSTM1, ySTM);
    grcoll1->SetPoint(b, xcoll1, ycoll);
    //b++;
    grSTM2->SetPoint(b, xSTM2, ySTM);
    grcoll2->SetPoint(b, xcoll2, ycoll);
    b++;
  }


  //TH2D *h = new TH2D("TH2D","",100,xmin,xmax,100,ymin,ymax);
  TH2D *h = new TH2D("TH2D","",200,xmin,xmax,200,ymin,ymax);  
  //TH2D *h = new TH2D("TH2D","",500,xmin,xmax,500,ymin,ymax);
  
  vector<double> x_vect,y_vect,p_vect;

  std::cout<<"---Loop over files---"<<std::endl;
  unsigned long j = 0;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    string path;
    path=file_name[file];
    std::cout<<path.c_str()<<std::endl;
   
    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");
    //TTree* tree=(TTree*)input->Get("readvd/ntvdext");

    tree->SetBranchAddress("evt",&evt);
    tree->SetBranchAddress("sid",&sid);
    tree->SetBranchAddress("pdg",&pdg);
    tree->SetBranchAddress("run",&run);
    tree->SetBranchAddress("subrun",&subrun);
    tree->SetBranchAddress("time",&time);
    tree->SetBranchAddress("x",&x);
    tree->SetBranchAddress("y",&y);
    tree->SetBranchAddress("z",&z);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("ke",&ke);
    tree->SetBranchAddress("code",&code);

    unsigned long entries=tree->GetEntries();
    std::cout<<"entries: "<<entries<<std::endl;

    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);
      double mom = sqrt(px*px+py*py+pz*pz);
      double pt = sqrt(px*px+py*py);
      double costheta = pz/mom;

      //calculate radius
      double xcentre = -3904; //mm
      //double xcentre = -3850; //mm
      double ycentre = 0;
      double r = sqrt((x-xcentre)*(x-xcentre)+(y-ycentre)*(y-ycentre));
      
      if((debugout==true)&&(sid==VDnumber)){
	std::cout<<"Virtual Detector number: "<<sid<<std::endl;
	std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm code: "<<code<<std::endl;
	}

      
      if((branch==1)&&(sid==VDnumber)&&((pdg==11)||(pdg==-11))){gr->SetPoint(j,x,y); j++;}
      if((branch==2)&&(pdg!=11)&&(pdg!=-11)){gr->SetPoint(j,x,y); j++;}
      if((branch==3)&&(pdg==22)&&(sid==VDnumber)){gr->SetPoint(j,pz,pt); j++;
	/*if((pz>80)||(pz<-10)||(pt>8)){
	  std::cout<<"Virtual Detector number: "<<sid<<std::endl;
	  std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<"\
	  MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm"<<std::endl;}*/
      }
      if((branch==4)&&(pdg==22)&&(sid==VDnumber)){gr->SetPoint(j,x,y); j++;//std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm"<<std::endl;
      }
      //if((branch==5)&&(pdg==22)&&(sid==VDnumber)){
      //if((branch==5)&&(pdg==11)&&(sid==VDnumber)){
      if((branch==5)&&(pdg==13)&&(sid==VDnumber)){
	//if((r < 71)&&(pz>0)){
	//if((x/mm_to_cm <= -380)&&(x/mm_to_cm >= -400)&&(y/mm_to_cm <= 25)&&(y/mm_to_cm >= -25)&&(pz>0)){
	//if(pz>0){
	//if((r < 35)&&(pz>0)){ 
	//if((r < 75.0224)&&(pz > 0)&&(costheta>=0.99999)){ 
	//if(costheta>=0.99999){
	//if((r < 75.0224)&&(pz > 0)){
	//if((r < 25)&&(pz > 0)){
	//if(costheta>=0.99999){ 
	//if(r < 200){ 
	//if(r < 71){ 
	  h->Fill(x/mm_to_cm,y/mm_to_cm);
	  //std::cout<<"px: "<<px<<" py: " <<py<<" pz: "<<pz<<std::endl;
	  //}
	  //}
	/*std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm"<<std::endl;*/
      }
      //if((branch==5)&&(pdg==13)&&(sid==VDnumber)){h->Fill(x/mm_to_cm,y/mm_to_cm);/*std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm"<<std::endl;*/}
      if((branch==6)&&(pdg==22)&&(sid==VDnumber)){gr->SetPoint(j,time,pz); j++;/*if(time>800){std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" MeV, time: "<<time<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm, code: "<<code<<std::endl;}*/}
      //muons
      if((branch==7)&&(sid==VDnumber)&&(pdg==13)){
      //if((branch==7)&&(sid==VDnumber)&&(pdg==13)&&(pz>0)){
      //pions
      //if((branch==7)&&(sid==VDnumber)&&(pdg==-211)){
      //electrons
      //if((branch==7)&&(sid==VDnumber)&&(pdg==11)){
        x_vect.push_back(x/mm_to_cm);
        y_vect.push_back(y/mm_to_cm);
        p_vect.push_back(mom);
        j++;
      }

       if((branch==8)&&(pdg==22)&&(VDnumber==8990)){

	 if((sid==89)||(sid==90)){
	   h->Fill(x/mm_to_cm,y/mm_to_cm);
	 }
       }

       if((branch==9)&&(sid==VDnumber)&&(pdg==22)){
	 
	 gr->SetPoint(j,x/mm_to_cm,y/mm_to_cm); j++;

	 h->Fill(x/mm_to_cm, y/mm_to_cm, costheta);
       }
       
    }//entries
    input->Close();
  }//for int files

  std::cout<<"Number of points in plot: "<<j<<std::endl;
  gr->SetTitle("");
  gr->GetXaxis()->SetTitle(Xtitle);
  gr->GetYaxis()->SetTitle(Ytitle);
  if((branch==1)||(branch==2)||(branch==4)){gr->SetMarkerStyle(1); gr->SetMarkerColor(kBlack);}
  if(branch==3){gr->SetMarkerStyle(7); gr->SetMarkerColor(kGreen+2);}
  if(branch==5){
    TAxis *X = h->GetXaxis();
    X->SetNdivisions(5,3,0);
    //h->GetZaxis()->SetRangeUser(0,499000);
  }
  if(branch==6){gr->SetMarkerStyle(1); gr->SetMarkerColor(kMagenta+2);}
  if(branch==9){
    grSTM1->SetMarkerStyle(1); grSTM1->SetMarkerColor(kYellow-10);grSTM1->SetLineColor(kYellow-10); grSTM1->SetFillColor(kYellow-10);
    grcoll1->SetMarkerStyle(1); grcoll1->SetMarkerColor(kGreen+2);grcoll1->SetLineColor(kGreen+2); grcoll1->SetFillColor(kGreen+2);
    grSTM2->SetMarkerStyle(1); grSTM2->SetMarkerColor(kYellow-10);grSTM2->SetLineColor(kYellow-10);
    grcoll2->SetMarkerStyle(1); grcoll2->SetMarkerColor(kGreen+2);grcoll2->SetLineColor(kGreen+2);
    gr->SetMarkerStyle(7); gr->SetMarkerColor(kPink);
    grSTM1->Draw("same,l");
    grcoll1->Draw("same,l");
    grSTM2->Draw("same,l");
    grcoll2->Draw("same,l");
    h->GetZaxis()->SetRangeUser(-1,1);
    std::string str_latex = "cos(#theta)";
    char* char_latex = const_cast<char*>(str_latex.c_str());
    TLatex latex;
    latex.DrawLatexNDC(.82,.96,char_latex);
    TAxis *Z = h->GetZaxis();
    Z->SetNdivisions(5,3,0);
    h->Draw("same,colz1");
    c1->Modified();
    c1->Update();
    //gr->Draw("same,p");
    auto leg = new TLegend(0.2,0.8,0.68,0.91);
    leg->AddEntry(grSTM1, "STM","f");
    leg->AddEntry(grcoll1, "SS Collimator","f");
    leg->Draw("same");
  }
  
  if(branch==7){
    TGraph2D *gr2D = new TGraph2D(x_vect.size(),&x_vect[0],&y_vect[0],&p_vect[0]);
    //gr2D->GetHistogram()->GetZaxis()->SetLabelSize(0.038);
    gr2D->Draw("same,colz");
    
    TLatex latex;
    latex.SetTextSize(0.04);
    latex.SetTextAlign(13);
    latex.DrawLatex(-370.44,22.64,"#font[22]{p_{e^{-}}} [MeV]");
  }


  //gr->Draw("same,p");

  
  h->GetXaxis()->SetTitle(Xtitle);
  h->GetYaxis()->SetTitle(Ytitle);
  h->Draw("colz");
  /*
  c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  //ps->SetX1NDC(0.62); //new x start position
  ps->SetX1NDC(0.54);
  ps->SetX2NDC(0.82); //new x end position
  //ps->SetY1NDC(0.76); //new y start position
  ps->SetY1NDC(0.74);
  ps->SetY2NDC(0.9); //new y end position
  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats 
  h->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
  c1->Modified();
  */
    
  /*
  std::string rootfile = "PhotonsTH2DMuonElectronBeamVD101_MDC2020.root";

  TFile* output=new TFile(rootfile.c_str(),"recreate");
  output->WriteObject(h, "h");
  output->Close();
  */

   std::string plot_name = "Electrons_VD15fixedpos_pz_simelectronsatVD"+std::to_string(VDnumber)+"_1000MeV_v3new.png";
   char* plot_name_char = const_cast<char*>(plot_name.c_str());
  
   //c1->Print(plot_name_char);
   c1->Print("VD8_InputPos_muons_nostats.png");

}

//================================================================ 
void PrintHisto(vector<string> file_name, int nbins, double xmin, double xmax, double ymin, double ymax, string Xtitlest, string Ytitlest, int VDnumber, int branch){

  double POT= 200000000;
  bool debugout=false;
  unsigned long int countelectrons=0;
  unsigned long int countentries=0;
  unsigned long int counterphot=0;
  gROOT->SetStyle("ATLAS");
  //gStyle->SetOptStat(1111);
  //Removes title of the histogram (TH1)
  //gStyle->SetOptStat(1110);
  //gStyle->SetOptStat(111110);
  //Three significant digits, doesn't work for entries
  //gStyle->SetStatFormat("6.3g"); 

  float evt, trk, sid /*virtualdet ID*/, pdg, run, subrun, time /*ns hit time*/, x, y, z /*mm mu2e coord*/, px, py, pz /*MeV*/, xl, yl, zl /*mm center each VD coord*/, pxl, pyl, pzl /*MeV same as px py pz*/, gtime /*hit proper time, gtime=gtime_parent+sim.startProperTime()*/, g4bl_weight /*extra.weight() / =0*/, g4bl_time /*extra.time() / =0*/, ke /*MeV*/, code /*sim.creationCode(), Creation code*/;

  TCanvas *c1 = new TCanvas("");
  double Xrange[2]={xmin,xmax};
  double Yrange[2]={ymin,ymax};
  char* Xtitle = const_cast<char*>(Xtitlest.c_str());
  char* Ytitle = const_cast<char*>(Ytitlest.c_str());
     
  TGraph *graph1 = new TGraph (2,Xrange,Yrange);
  graph1->GetXaxis()->SetRangeUser(Xrange[0], Xrange[1]);
  graph1->GetYaxis()->SetRangeUser(Yrange[0],Yrange[1]);
  graph1->SetTitle("");
  graph1->GetXaxis()->SetTitle(Xtitle);
  graph1->GetYaxis()->SetTitle(Ytitle);
  graph1->SetMarkerStyle(1);
  graph1->Draw("ap");
  
  TH1F*h1 = new TH1F("TH1","", nbins, Xrange[0], Xrange[1]);
  TH1F*h2 = new TH1F("TH2","", nbins, Xrange[0], Xrange[1]);
  TH1F*h3 = new TH1F("TH3","", nbins, Xrange[0], Xrange[1]);
  TH1F*h4 = new TH1F("TH4","", nbins, Xrange[0], Xrange[1]);
 
  double totpiVD8=0;
  double totpiVD85=0;
  double totmuVD8=0;
  double totmuVD85=0;

  double photons=0;
  double pions=0;
  double muons=0;
  double electrons=0;

  Double_t scale_photons = 100000;
  Double_t scale_pions = 70000;
  Double_t scale_muons = 300;
  Double_t scale_electrons = 4;

  unsigned long int integral2_costheta = 0;

  double count_photopeak =0;

  double countphotons[5];
  double countphotons_smallsolidangle[5];

  double countele[5];
  double countele_smallsolidangle[5];
  
  for (int i=0; i< 5; i++){
    countphotons[i]=0;
    countphotons_smallsolidangle[i]=0;
    countele[i]=0;
    countele_smallsolidangle[i]=0;
  }


  
  std::cout<<"---Loop over files---"<<std::endl;
  for (long unsigned int file=0;file<(file_name.size()-1);file++){
    //for (long unsigned int file=0;file<1;file++){
    string path;
    path=file_name[file];
    std::cout<<"file #: "<<file<<" "<<path.c_str()<<std::endl;

    TFile *input=new TFile(path.c_str());
    TTree* tree=(TTree*)input->Get("readvd/ntvd");
    //TTree* tree=(TTree*)input->Get("readvd/ntvdext");

    tree->SetBranchAddress("evt",&evt);
    tree->SetBranchAddress("trk",&trk);
    tree->SetBranchAddress("sid",&sid);
    tree->SetBranchAddress("pdg",&pdg);
    tree->SetBranchAddress("run",&run);
    tree->SetBranchAddress("subrun",&subrun);
    tree->SetBranchAddress("time",&time);
    tree->SetBranchAddress("x",&x);
    tree->SetBranchAddress("y",&y);
    tree->SetBranchAddress("z",&z);
    tree->SetBranchAddress("px",&px);
    tree->SetBranchAddress("py",&py);
    tree->SetBranchAddress("pz",&pz);
    tree->SetBranchAddress("ke",&ke);
    tree->SetBranchAddress("code",&code);

    unsigned long entries=tree->GetEntries();
    countentries=countentries+entries;
    std::cout<<"entries: "<<entries<<std::endl;

    double piVD8=0;
    double muVD8=0;
    double piVD85=0;
    double muVD85=0;

    double nphotons = 0;
    
    for(unsigned long i=0;i<entries;i++){

      tree->GetEntry(i);

      if((pdg==-211)&&(sid==8)){piVD8++; totpiVD8++;}
      if((pdg==-211)&&(sid==85)){piVD85++; totpiVD85++;}
      if((pdg==13)&&(sid==8)){muVD8++; totmuVD8++;}
      if((pdg==13)&&(sid==85)){muVD85++; totmuVD85++;}


      //Just fill at Virtual detector ID number:
      //float VDnumber = 89;
      if(sid==VDnumber){

	double mom = sqrt(px*px+py*py+pz*pz);
	double pt = sqrt(px*px+py*py);
	double costheta = pz/mom;

	 //calculate radius
          double xSTcentre = -3904; //mm
          double ySTcentre = 0;
          double r = sqrt((x-xSTcentre)*(x-xSTcentre)+(y-ySTcentre)*(y-ySTcentre));
	  
	if((debugout==true)/*&&(file==0)*/){
	  std::cout<<"Virtual Detector number: "<<sid<<std::endl;
	  std::cout<<"Entry: "<<i<<", pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<", pt: "<<pt<<" mom: "<<mom<<" MeV, time: "<<time<<std::setprecision(30)<<" ns, xmu2eVD= "<<x<<" ymu2eVD= "<<y<<", zmu2eVD= "<<z<<" mm, creation code: "<<code<<" trk: "<<trk<<std::endl;
	  //if((file==0)&&(i<=200000)){std::cout<<"Evt: "<<evt<<" VDid: "<<sid<<" pdg: "<<pdg<<" ke: "<<ke<<" MeV "<<"|p|: "<<mom<<" z: "<<z<<" mm "<<"thit: "<<time<<" ns"<<std::endl;}
	  //if((pdg!=11)&&(pdg!=-11)){std::cout<<"pdg: "<<pdg<<" WARNING!!: Not e- or e+ at VD 8, should remove this particle from analysis"<<std::endl;}
	  //if((ke>80)&&((pdg==11)||(pdg==-11))){std::cout<<"pdg: "<<pdg<<" with ke>80MeV, ke= "<<ke<<" MeV"<<std::endl;}
	  //if(time>500){std::cout<<"pdg: "<<pdg<<" with arrival time>500ns, time= "<<time<<" ns"<<std::endl;}
	  //if((px>50)||(py>50)||(pz>100)){std::cout<<"pdg: "<<pdg<<" with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<std::endl;}
	  //if((pdg!=11)&&(pdg!=-11)&&(pz>100)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns"<<std::endl;} 
	  //if((pdg!=11)&&(pdg!=-11)&&(time>500)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns"<<std::endl;} 
	  //if((pdg==11)||(pdg==-11)){std::cout<<"pdg: "<<pdg<<", with px: "<<px<<" py: "<<py<<" and pz: "<<pz<<"MeV, time: "<<time<<" ns, xmu2eVD8= "<<x<<" ymu2eVD8= "<<y<<", zmu2eVD8= "<<z<<" mm"<<std::endl;}
	}

	//if(z!=11810.099609375){std::cout<<std::setprecision(30)<<"Z: "<<z<<std::setprecision(30)<<std::endl;}
	//if((code!=56)&&(code!=40)&&(code!=16)&&(pdg==22)){std::cout<<"CREATION CODE PHOT: "<<code<<std::endl;}
	//if((code!=56)&&(pdg==22)){std::cout<<"CREATION CODE PHOT: "<<code<<std::endl;} 
	//if((time>0)&&(pz>0)){std::cout<<"HELLO"<<std::endl;}
	
	if((pdg==11)||(pdg==-11)){countelectrons++;}
	//if((branch==0)&&(pdg==11)){
	//if((branch==0)&&(pdg==11)&&(r < 75.0224)&&(pz > 0)){
	/*if((branch==0)&&(pdg==11)&&(pz > 0)){ 
	//if(r < 71){

	  if (mom<0.5){
            countele[0]++;
            if(costheta>=0.99999){ countele_smallsolidangle[0]++;}
          }
          if ((mom>=0.5)&&(mom<1)){
            countele[1]++;
            if(costheta>=0.99999){ countele_smallsolidangle[1]++;}
          }
          if ((mom>=1)&&(mom<5)){
            countele[2]++;
            if(costheta>=0.99999){ countele_smallsolidangle[2]++;}
          }
          if ((mom>=5)&&(mom<25)){
            countele[3]++;
            if(costheta>=0.99999){ countele_smallsolidangle[3]++;}
          }
          if (mom>=25){
            countele[4]++;
            if(costheta>=0.99999){ countele_smallsolidangle[4]++;}
          }

	    h1->Fill(mom);
	    //}
	    }*/
	//if((branch==0)&&(pdg==-11)&&(pz > 0)){h1->Fill(mom);}
	if((branch==0)&&((pdg==11)||(pdg==-11))){h1->Fill(mom);/*std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<", trk: "<<trk<<", x: "<<x<<" y: "<<y<<" z: "<<z<<" time: "<<time<<std::endl;*/}
	//if((branch==1)&&(pdg!=11)&&(pdg!=-11)){h1->Fill(ke);}
	//if((branch==1)&&((pdg==13)||(pdg==-13))){h1->Fill(ke);}
	//if((branch==1)&&(pdg==-211)){h1->Fill(ke);}
	/*if((branch==1)&&(pdg==22)&&(r < 75.0224)&&(pz>0)){
	//if((branch==1)&&(pdg==22)&&(pz>0)){ 
	if (mom<0.1){
	    countphotons[0]++;
	    if(costheta>=0.99999){ countphotons_smallsolidangle[0]++;}
	  }
	  if ((mom>=0.1)&&(mom<0.3)){
	    countphotons[1]++;
            if(costheta>=0.99999){ countphotons_smallsolidangle[1]++;}
          }
	  if ((mom>=0.3)&&(mom<0.4)){
	    countphotons[2]++;
            if(costheta>=0.99999){ countphotons_smallsolidangle[2]++;}
          }
	  if ((mom>=0.4)&&(mom<0.5)){
	    countphotons[3]++;
            if(costheta>=0.99999){ countphotons_smallsolidangle[3]++;}
          }
	  if (mom>=25){
	    countphotons[4]++;
            if(costheta>=0.99999){ countphotons_smallsolidangle[4]++;}
          }

	  h1->Fill(mom);
	}
	*/
	/*
	if((branch==1)&&(pdg==22)&&(pz>0)){
	  if(r < 75.0224){
	    h1->Fill(mom);
	    }
	}
	*/
	//if((branch==1)&&(pdg==22)&&(mom>Xrange[0])&&(mom<Xrange[1])){h1->Fill(mom); 
	  //if((mom>0.8)&&(mom<0.9)){std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<std::endl;counterphot++;}
	//}
	//if((branch==1)&&(pdg==22)&&(mom>0.064)&&(mom<0.068)&&(time>200)){h1->Fill(mom); std::cout<<"Creation Code: "<<code<<" mom: "<<mom<<std::endl;}
	//if((branch==1)&&(pdg==22)&&(code==115)){  h1->Fill(mom);}
	//if((branch==1)&&(pdg==22)&&(pz>0)&&(code==56)){h1->Fill(mom);}
	
	if((branch==1)&&(pdg==22)){
	  double xcentre = -3904; //mm
	  double ycentre = 0;
          double r = sqrt((x-xcentre)*(x-xcentre)+(y-ycentre)*(y-ycentre));
	  //if((r < 75.0224)&&(pz > 0)&&(mom>Xrange[0])&&(mom<Xrange[1])){
	  //if((r < 75.0224)&&(pz > 0)){
	  //if(pz > 0){
	  //if( mom > 0.1 ){
	    h1->Fill(mom);
	    //if(mom>60){std::cout<<"gamma momentum: "<<mom<<std::endl;}
	    //if((mom>0.344)&&(mom<0.35)){ count_photopeak++ ;}
	    //}
	}
	//if((branch==1)&&(pdg==22)){h1->Fill(px);} 
	//if((branch==1)&&(pdg==22)&&(code==56)){h1->Fill(px);} 
	//if((branch==1)&&(pdg==22)&&((mom==0.34700000286102294921875)||(mom==0.3469999730587005615234375))){h1->Fill(px);}
	//if((branch==1)&&(pdg==22)&&(pz>0)){h1->Fill(px);}
	//if((branch==1)&&(pdg==22)&&(pz>0)&&(code==56)&&(time==0)){h1->Fill(px);}
	//if((branch==1)&&(pdg==22)&&(time!=0)&&((mom==0.34700000286102294921875)||(mom==0.3469999730587005615234375))){h1->Fill(px);}
	//if((branch==1)&&(pdg==22)&&(pz>0)&&(code==56)&&((mom==0.34700000286102294921875)||(mom==0.3469999730587005615234375))){h1->Fill(px);}
	//if((branch==1)&&(pdg==22)){h1->Fill(py);}
	//if((branch==1)&&(pdg==22)){h1->Fill(pz);}
	//if((branch==1)&&(pdg==22)&&(pz>0)&&(code==56)){h1->Fill(pz);}
	if((branch==2)&&((pdg==11)||(pdg==-11))){h1->Fill(time);}
	//if((branch==3)&&(pdg!=11)&&(pdg!=-11)){h1->Fill(time);}
	//if((branch==3)&&((pdg==13)||(pdg==-13))){h1->Fill(time);}
	//if((branch==3)&&(pdg==-211)){h1->Fill(time);} 
	//if((branch==3)&&(pdg==22)){h1->Fill(time);}
	if((branch==3)&&(pdg==22)&&(mom>0.8)&&(mom<0.9)&&(time>200)){h1->Fill(time); std::cout<<"Creation Code: "<<code<<" mom: "<<mom<<std::endl;}
	if((branch==4)&&((pdg==11)||(pdg==-11))){/*h1->Fill(px)*//*h1->Fill(py)*/h1->Fill(pz);}
	//if((branch==5)&&(pdg!=11)&&(pdg!=-11)){/*h1->Fill(px)*//*h1->Fill(py)*/h1->Fill(pz);}
	//if((branch==5)&&((pdg==13)||(pdg==-13))){h1->Fill(pz);}
	//if((branch==5)&&(pdg==13)){h1->Fill(mom); std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<", trk: "<<trk<<", x: "<<x<<" y: "<<y<<" z: "<<z<<" time: "<<time<<std::endl;}
	//if((branch==5)&&(pdg==13)&&(pz > 0)){h1->Fill(mom); /*std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<", trk: "<<trk<<", x: "<<x<<" y: "<<y<<" z: "<<z<<" time: "<<time<<std::endl;*/}
	//if((branch==5)&&(pdg==-211)){h1->Fill(mom); std::cout<<"Event: "<<i<< " VD: "<<sid<<", Pdgid: "<<pdg<<", Creation Code: "<<code<<" mom: "<<mom<<", trk: "<<trk<<", x: "<<x<<" y: "<<y<<" z: "<<z<<" time: "<<time<<std::endl;} 
	//if((branch==5)&&(pdg==22)){h1->Fill(mom); nphotons++;}
	if((branch==5)&&(pdg==22)&&(pz>0)){h1->Fill(mom);} 
	//if((branch==5)&&(pdg==22)){h1->Fill(pz);} 
	//if((branch==5)&&(pdg==2212)){h1->Fill(pz);}
	if((branch==6)&&((pdg==13)||(pdg==-13))){h1->Fill(time);muons++;}
        if((branch==6)&&(pdg==-211)){h2->Fill(time);pions++;}
        if((branch==6)&&(pdg==22)){h3->Fill(time);photons++;}
        if((branch==6)&&((pdg==11)||(pdg==-11))){h4->Fill(time);electrons++;}
	if((branch==7)&&(pdg==22)&&(code==16)){h1->Fill(mom);} //brems
	//if((branch==7)&&(pdg==22)&&(code==2)){h1->Fill(mom);} //annih
	//if((branch==7)&&(pdg==22)&&(code==40)){h1->Fill(mom);} //phot
	if((branch==8)&&(pdg==22)){h1->Fill(y/mm_to_cm); //std::cout<<"mom: "<<mom<<std::endl;
	}  
	//if((branch==8)&&(pdg==22)){h1->Fill(x/mm_to_cm);}
        //if((branch==8)&&(pdg==22)){h1->Fill(y/mm_to_cm);}
	//if((branch==8)&&(pdg==11)&&(pz>0)){h1->Fill(x/mm_to_cm);}
	//if((branch==8)&&(pdg==11)&&(pz>0)){h1->Fill(y/mm_to_cm);}
	if((branch==9)&&(pdg==22)&&(mom>0.344)&&(mom<0.350)&&(pz>0)){
	  if(r <= 75.0224){
	    if((costheta<=xmax)&&(costheta>=xmin)){ 
	      h1->Fill(costheta);
	      //std::cout<<"costheta: "<<costheta<<std::endl;
	      integral2_costheta++;
	      }	    
	    }
	}
	if((branch==10)&&(pdg==22)){ //phi
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
	  h1->Fill(phi);
	}

	if((branch==11)&&(pdg==22)&&(mom>1.805)&&(mom<1.81)&&(time>200)){h1->Fill(code);}
	
      } //if sid
   
    }//entries

    double pimuVD8=muVD8+piVD8;
    double pimuVD85=muVD85+piVD85;
    std::cout<<"Muons at VD=8: "<<muVD8<<" pions at VD=8: "<<piVD8<<" pions+muons: "<<pimuVD8<<std::endl;
    std::cout<<"Muons at VD=85: "<<muVD85<<" pions at VD=85: "<<piVD85<<" pions+muons: "<<pimuVD85<<std::endl;
    //if((pimuVD8-pimuVD85)!=0){std::cout<<"CHECK THIS EVENT"<<std::endl;}

    std::cout<<"Number of photons in this file: "<<nphotons<<std::endl;
    input->Close();
  }//for int files

  h1->SetTitle("");
  //h1->SetStats(0);
  h1->GetXaxis()->SetTitle(Xtitle);
  h1->GetYaxis()->SetTitle(Ytitle);
  //h1->SetFillStyle(3001);
  
  if(branch==0){h1->SetFillColor(kOrange+2);}
  if(branch==1){
    TAxis *X = h1->GetXaxis();
    X->SetNdivisions(5,3,0);
    h1->SetFillColor(kGreen+2);
    //h1->SetFillColor(kOrange-3);
  }
  if(branch==2){h1->SetFillColor(kViolet+2);}
  if(branch==3){h1->SetFillColor(kMagenta+2);}
  if(branch==4){h1->SetFillColor(kOrange+2);}
  if(branch==5){h1->SetFillColor(kRed+2);}
  if(branch==6){h1->SetLineColor(kBlue+2);
    h2->SetLineColor(kMagenta-7);
    h3->SetLineColor(kGreen+2);
    h4->SetLineColor(kBlue-7);
    h1->SetFillColor(kBlue+2);
    h2->SetFillColor(kMagenta-7);
    h3->SetFillColor(kGreen+2);
    h4->SetFillColor(kBlue-7);
    h1->SetFillStyle(3001);
    h2->SetFillStyle(3001);
    h3->SetFillStyle(3001);
    h4->SetFillStyle(3001);
  }

  if(branch==7){h1->SetFillColor(kOrange-3);}
  if(branch==8){h1->SetLineColor(kBlue+1);}
  if(branch==9){
    double integral1 = h1->Integral(h1->FindFixBin(xmin), h1->FindFixBin(xmax));
    std::cout<<"cos(theta) integral between(binning): ["<<xmin<<","<<xmax<<"]: "<<integral1<<std::endl;
    std::cout<<"cos(theta) integral between(count events): ["<<xmin<<","<<xmax<<"]: "<<integral2_costheta<<std::endl;

    TAxis *X = h1->GetXaxis();
    X->SetNdivisions(5,3,0);
    h1->SetFillColor(kGreen-6);
    h1->SetLineColor(kGreen-6);}

  if(branch==10){ //phi phot
    h1->SetLineColor(kGreen-5);
    h1->SetFillColor(kGreen-5);
  }
  if(branch==11){h1->SetFillColor(kYellow);}

  //Comment for branch=6 or 9
  //h1->Scale(1./POT);
  //h1->Draw("HIST");
  h1->Draw("");

  auto leg = new TLegend(0.1,0.7,0.48,0.9);
  TH1F*hnorm1 = (TH1F*)(h1->Clone("TH1"));
  hnorm1->Scale(1./hnorm1->Integral());
  TH1F*hnorm2 = (TH1F*)(h2->Clone("TH2"));
  hnorm2->Scale(1./hnorm2->Integral());
  TH1F*hnorm3 = (TH1F*)(h3->Clone("TH3"));
  hnorm3->Scale(1./hnorm3->Integral());
  TH1F*hnorm4 = (TH1F*)(h4->Clone("TH4"));
  hnorm4->Scale(1./hnorm4->Integral());
  if(branch==6){
    //Draw histograms without scaling or normalization
    //h2->Draw("same");
    //h1->Draw("same");
    //h3->Draw("same");           
    //h4->Draw("same");
    
    //Scaled histograms to these numbers           
    std::string str_scale_muons = std::to_string(int(scale_muons));
    std::string str_scale_muons_chain = "#mu^{#pm} arrival time at VD8 (#times"+str_scale_muons+")";
    char* char_scale_muons = const_cast<char*>(str_scale_muons_chain.c_str());

    std::string str_scale_pions = std::to_string(int(scale_pions));
    std::string str_scale_pions_chain = "#pi^{-} arrival time at VD8 (#times"+str_scale_pions+")";
    char* char_scale_pions = const_cast<char*>(str_scale_pions_chain.c_str());

    std::string str_scale_photons = std::to_string(int(scale_photons));
    std::string str_scale_photons_chain = "#gamma arrival time at VD8 (#times"+str_scale_photons+")";
    char* char_scale_photons = const_cast<char*>(str_scale_photons_chain.c_str());

    std::string str_scale_electrons = std::to_string(int(scale_electrons));
    std::string str_scale_electrons_chain = "#font[42]{e}^{#pm} arrival time at VD8 (#times"+str_scale_electrons+")";
    char* char_scale_electrons = const_cast<char*>(str_scale_electrons_chain.c_str());

    h1->Scale(scale_muons);
    h2->Scale(scale_pions);
    h3->Scale(scale_photons);
    h4->Scale(scale_electrons);
    h2->Draw("HIST,same");
    h1->Draw("HIST,same");
    h4->Draw("HIST,same");
    h3->Draw("HIST,same");
    std::string str_latex = "#scale[1]{Mu2e simulation, 2#times10^{8} POT}";
    char* char_latex = const_cast<char*>(str_latex.c_str());
    TLatex latex;
    latex.DrawLatexNDC(.3,.85,char_latex);

    //Normalized to integral
    //hnorm2->Draw("HIST,same");
    //hnorm1->Draw("HIST,same");
    //hnorm3->Draw("HIST,same");
    //hnorm4->Draw("HIST,same");


    leg->AddEntry(h3, char_scale_photons,"l");
    leg->AddEntry(h4, char_scale_electrons,"l");
    leg->AddEntry(h2, char_scale_pions,"l");
    leg->AddEntry(h1, char_scale_muons,"l");
    leg->Draw("same");
  }

  //Comment this for branch==6
  /*c1->Update();
  TPaveStats *ps = (TPaveStats*)c1->GetPrimitive("stats");
  ps->SetX1NDC(0.71); //new x start position
  ps->SetX2NDC(0.91); //new x end position
  ps->SetY1NDC(0.76); //new y start position
  ps->SetY2NDC(0.9); //new y end position
  ps->SetTextSize(0.032);
  ps->SetName("mystats");

  // the following line is needed to avoid that the automatic redrawing of stats
  h1->SetStats(0);
  ps->GetLineWith("Entries")->SetTextColor(kBlue);
*/
  
  /*
  ps->GetLineWith("Entries")->SetTextSize(0.027);
  ps->GetLineWith("Mean")->SetTextSize(0.027);
  ps->GetLineWith("Std Dev")->SetTextSize(0.027);
  */

  c1->Modified();


  std::cout<<"Total number of entries(=events=particles) in all the .root files: "<<countentries<<std::endl;
  std::cout<<"Number of electrons+positrons: "<<countelectrons<<std::endl;
 
  std::cout<<"Total number of muons at VD8: "<<totmuVD8<<std::endl;
  std::cout<<"Total number of muons at VD85: "<<totmuVD85<<std::endl;
  std::cout<<"Total number of pions at VD8: "<<totpiVD8<<std::endl;
  std::cout<<"Total number of pions at VD85: "<<totpiVD85<<std::endl;
  if(branch==6){
  std::cout<<"Total number of muons: "<<muons<<std::endl;
  std::cout<<"Total number of electrons: "<<electrons<<std::endl;
  std::cout<<"Total number of pions: "<<pions<<std::endl;
  std::cout<<"Total number of photons: "<<photons<<std::endl;
  }
 
  //h1->Draw("same");
  //gPad->SetLogy();  
  std::cout<<"Number of photons not coming from annihil, brems or phot: "<<counterphot<<std::endl;
  /*
    int pos1 = file_name.at(0).find("/PhotGun_")+1;
    //int pos2 = file_name.at(0).find("/FlashAcceptance_");
    int pos2 = file_name.at(0).find("/nts.claudiaa")-13;
    
    int diff = pos2-pos1;
    std::cout<<"pos1: "<<pos1<<" pos2: "<<pos2<<" diff: "<<diff<<std::endl;
    std::string plot_name = "10to7"+file_name.at(0).substr(pos1, diff)+"_StoppedposST_099999_1_VD"+std::to_string(VDnumber);
 
    std::string plot_name_png = plot_name+"_new.png";
    std::string plot_name_pdf = plot_name+"_new.pdf";

    char* imagename_png = const_cast<char*>(plot_name_png.c_str());
    char* imagename_pdf = const_cast<char*>(plot_name_pdf.c_str());
  
    c1->Print(imagename_png);
    c1->Print(imagename_pdf);
  */

  std::cout<<"Number of counts in photopeak: "<<count_photopeak<<std::endl;
  /*
  std::string rootfile = "eleElectronBeamVD101_MDC2020.root";

  //c1->Print("eleElectronBeamVD101_MDC2020.png");
  
  TFile* output=new TFile(rootfile.c_str(),"recreate");
  output->WriteObject(h1, "h1");
  output->Close();
  */

  std::cout<<"Photons <0.1MeV: "<<countphotons[0]<<" in 0.99999-1 costheta: "<<countphotons_smallsolidangle[0]<<std::endl; 
  std::cout<<"Photons 0.1-0.3MeV: "<<countphotons[1]<<" in 0.99999-1 costheta: "<<countphotons_smallsolidangle[1]<<std::endl;
  std::cout<<"Photons 0.3-0.4MeV: "<<countphotons[2]<<" in 0.99999-1 costheta: "<<countphotons_smallsolidangle[2]<<std::endl;
  std::cout<<"Photons 0.4-0.5MeV: "<<countphotons[3]<<" in 0.99999-1 costheta: "<<countphotons_smallsolidangle[3]<<std::endl;
  std::cout<<"Photons 25-endMeV: "<<countphotons[4]<<" in 0.99999-1 costheta: "<<countphotons_smallsolidangle[4]<<std::endl;


  std::cout<<"Electrons <0.5MeV: "<<countele[0]<<" in 0.99999-1 costheta: "<<countele_smallsolidangle[0]<<std::endl;
  std::cout<<"Electrons 0.5-1MeV: "<<countele[1]<<" in 0.99999-1 costheta: "<<countele_smallsolidangle[1]<<std::endl;
  std::cout<<"Electrons 1-5MeV: "<<countele[2]<<" in 0.99999-1 costheta: "<<countele_smallsolidangle[2]<<std::endl;
  std::cout<<"Electrons 5-25MeV: "<<countele[3]<<" in 0.99999-1 costheta: "<<countele_smallsolidangle[3]<<std::endl;
  std::cout<<"Electrons 25-endMeV: "<<countele[4]<<" in 0.99999-1 costheta: "<<countele_smallsolidangle[4]<<std::endl;

  
  c1->Print("MDC2020Photonsy_VD8.png"); 
  //c1->Print("VD10_115NuclearCapture_time.pdf");
}



//================================================================
void ReadVD_plote(std::string ArtFiles_location, int VDnumber ) {
  //To store the screen in a log output file
  //gSystem->RedirectOutput("DatainputGenPaths.log"); 

  std::string p_gamma_name = "p_{#gamma, VD="+std::to_string(VDnumber)+"} [MeV]";
  char* p_gamma_name_char = const_cast<char*>(p_gamma_name.c_str());

  std::string x_gamma_name = "x_{#gamma, VD="+std::to_string(VDnumber)+"} [cm]";
  char* x_gamma_name_char = const_cast<char*>(x_gamma_name.c_str());

  std::string y_gamma_name = "y_{#gamma, VD="+std::to_string(VDnumber)+"} [cm]";
  char* y_gamma_name_char = const_cast<char*>(y_gamma_name.c_str());
  
  //Open txt
  fstream readfile;
  readfile.open(ArtFiles_location,ios::in);
  string name;
  vector<string> file_name;
  file_name.clear();

  //Read each art file from txt
  while(1){
    readfile>>name;
    file_name.push_back(name);
    if(readfile.eof())break;
    std::cout<<name<<std::endl;
  }




  
  //0 is to print momentum of e+, e- at VD
  //PrintHisto(file_name, 500, 0, 80, 0, 15000000, "p_{e^{#pm}, VD=10} [MeV]", "Counts", VDnumber, 0);
  //PrintHisto(file_name, 500, 0, 80, 0, 15000000, "p_{e^{-}, VD=10, cut ST size, p_{z}>0}[MeV]", "Counts", VDnumber, 0);
  //PrintHisto(file_name, 500, 0, 80, 0, 15000000, "p_{e^{+}, VD=10, p_{z}>0} [MeV]", "Counts", VDnumber, 0); 
  //PrintHisto(file_name, 100, -5, 80, 0, 15000000, "p_{e^{#pm}, VD=9} [MeV]", "Counts", VDnumber, 0);
  //1 is to print kinetic energy of rest of particles at VD
  //PrintHisto(file_name, 100, -5, 120, 0, 500, "E_{kin, #mu^{#pm},#pi^{-},p,#gamma} [MeV]", "Counts", VDnumber, 1);
  //1 is to print kinetic energy of muons of particles at VD
  //PrintHisto(file_name, 100, -5, 120, 0, 500, "E_{kin, #mu^{#pm}} [MeV]", "Counts", VDnumber, 1);
  //1 is to print kinetic energy of pions of particles at VD
  //PrintHisto(file_name, 100, -5, 120, 0, 500, "E_{kin, #pi^{-}} [MeV]", "Counts", VDnumber, 1);
  //1 is to print momentum of photons of particles at VD
  //PrintHisto(file_name, 100, 0.064, 0.068, 0, 500, "E_{#gamma, t>200ns, VD=10} [MeV]", "Counts", VDnumber, 1);
  //PrintHisto(file_name, 100, 0, 5, 0, 500, "p_{#gamma, VD=10 (cut ST size, p_{z}>0)} [MeV]", "Counts", VDnumber, 1);
  //PrintHisto(file_name, 1600, 0, 80, 0, 500, "p_{#gamma, VD=10} [MeV]", "Counts", VDnumber, 1); 
  //PrintHisto(file_name, 100, -0.003, 0.003, 0, 500, "p_{y, #gamma, VD=15} [MeV]", "Counts", VDnumber, 1);
  //PrintHisto(file_name, 100, 0.346, 0.348, 0, 500, "p_{z, #gamma, VD=15} [MeV]", "Counts", VDnumber, 1);
  //PrintHisto(file_name, 100, -10, 10, 0, 500, "p_{x, e, VD=15} [MeV]", "Counts", VDnumber, 1);
  //2 is to print arrival time of e+, e- to VD PROMPT 20-500ns
  //PrintHisto(file_name, 100, 20, 500, 0, 12000000, "t_{hit, e^{#pm}} [ns]", "Counts", VDnumber, 2);
  //2 is to print arrival time of e+, e- to VD LATE 500-1e14ns
  //PrintHisto(file_name, 100, 500, 100000000000000000, 0, 500000, "t_{hit, e^{#pm}} [ns]", "Counts", VDnumber, 2);
  //3 is to print arrival time of rest of particles to VD 
  //PrintHisto(file_name, 100, 0, 500, 0, 500, "t_{hit, #mu^{#pm},#pi^{-},p,#gamma} [ns]", "Counts", VDnumber, 3); 
  //PrintHisto(file_name, 100, 0, 500, 0, 500, "t_{hit, #mu^{#pm}} [ns]", "Counts", VDnumber, 3);
  //PrintHisto(file_name, 100, 0, 500, 0, 500, "t_{hit, #pi^{-}} [ns]", "Counts", VDnumber, 3);
  //PrintHisto(file_name, 100, 0, 2000, 0, 500, "t_{#gamma (nuclear capture), VD=10} [ns]", "Counts", VDnumber, 3);
  //PrintHisto(file_name, 100, 0, 10000, 0, 500, "t_{hit, p} [ns]", "Counts", VDnumber, 3);
  //3 is to print arrival time of rest of particles to VD
  //PrintHisto(file_name, 100, 500, 100000, 0, 500, "t_{hit, #mu^{#pm},#pi^{-},p,#gamma} [ns]", "Counts", VDnumber, 3); 
  //4 is px,py,pz of e+, e- at VD
  //PrintHisto(file_name, 100, -10, 100, 0, 500, "p_{z, e^{#pm}} [MeV]", "Counts", VDnumber, 4); 
  //5 is px,py,pz of rest of particles at VD
  //PrintHisto(file_name, 100, -10, 100, 0, 500, "p_{z, #mu^{#pm},#pi^{-},p,#gamma} [MeV]", Counts", VDnumber, 5);
  //PrintHisto(file_name, 100, -10, 100, 0, 500, "p_{z, #mu^{#pm}} [MeV]", "Counts", VDnumber, 5); 
  //PrintHisto(file_name, 100, -10, 100, 0, 500, "p_{z, #pi^{-}} [MeV]", "Counts", VDnumber, 5); 
  //PrintHisto(file_name, 100, 0, 1.2, 0, 500, p_gamma_name_char, "Counts", VDnumber, 5);
  //PrintHisto(file_name, 100, -10, 200, 0, 500, "p_{z, p} [MeV]", "Counts", VDnumber, 5);
  //PrintHisto(file_name, 500, -5, 100, 0, 500, "p_{#mu^{-}, VD=10, p_{z}>0} [MeV]", "Counts", VDnumber, 5); 
  //PrintHisto(file_name, 100, -10, 110, 0, 500, "p_{#pi^{-}, VD=} [MeV]", "Counts", VDnumber, 5);
  //6 Print arrival times of photons, muons and pions in different histos
  //PrintHisto(file_name, 100, 0, 500, 0, 0.3, "t_{hit} [ns]", "", VDnumber, 6);
  //PrintHisto(file_name, 100, 0, 600, 0, 400, "t_{hit} [ns]", "Counts", VDnumber, 6);
  //PrintHisto(file_name, 100, 0, 500, 0, 0.3, "t_{hit} [ns]", "Normalised Events", VDnumber, 6);
  //PrintHisto(file_name, 50, 0, 500, 0, 70000000, "t_{hit} [ns]", "N / 10 ns", VDnumber, 6);
  //7 is to print spectrum of photons that comes from bremstrahlung interaction
  //PrintHisto(file_name, 100, -1, 4, 0, 400, "p_{bremsstrahlung #gamma, VD=15} [MeV]", "Counts", VDnumber, 7); 
  //PrintHisto(file_name, 1000, 0.04, 2, 0, 400, "p_{bremsstrahlung #gamma, VD=15} [MeV]", "Normalised Counts/(0.00196 MeV)", VDnumber, 7);
  //PrintHisto(file_name, 100, -0.5, 2, 0, 400, "p_{annihilation #gamma (Electron Beam), VD=15} [MeV]", "Counts", VDnumber, 7);
  //PrintHisto(file_name, 100, -0.5, 2, 0, 400, "p_{photoelectric effect #gamma (Electron Beam), VD=15} [MeV]", "Counts", VDnumber, 7);
  //8 is to print x, y distribution (z is fixed for VDs)
  //PrintHisto(file_name, 200, -420, -360, 0, 400, "x_{#gamma, VD=8} [cm]", "Counts", VDnumber, 8);
  //PrintHisto(file_name, 100, -20, 20, 0, 400, "y_{#gamma, VD=8} [cm]", "Counts", VDnumber, 8); 
  //PrintHisto(file_name, 100, -5, 5, 0, 400, "y_{#gamma, VD=15} [cm]", "Counts", VDnumber, 8);
  //PrintHisto(file_name, 200, -400.4, -380.4, 0, 400, "x_{e^{-}, p_{z}>0, VD=10} [cm]", "Counts / (0.1 cm)", VDnumber, 8); 
  //PrintHisto(file_name, 200, -10, 10, 0, 400, "y_{e^{-}, p_{z}>0, VD=10} [cm]", "Counts / (0.1 cm)", VDnumber, 8);
  //PrintHisto(file_name, 100, -430, -350, 0, 400, "x_{#gamma, VD=101} [cm]", "Counts / (0.8 cm)", VDnumber, 8); 
  //PrintHisto(file_name, 100, -40, 40, 0, 400, "y_{#gamma, VD=101} [cm]", "Counts / (0.8 cm)", VDnumber, 8);
  //PrintHisto(file_name, 100, 0.99999, 1, 0, 400, "cos(#theta_{#gamma, VD=10, cut ST size, 0.344<p<0.350MeV})", "Counts", VDnumber, 9); 
  //PrintHisto(file_name, 100, -1, 7, 0, 400, "#phi_{#gamma} [rad]", "Counts", VDnumber, 10);
  //Creation code
  //PrintHisto(file_name, 100, 90, 150, 0, 500, "Creation Code_{#gamma, t>200ns, VD=10} [MeV]", "Counts", VDnumber, 11);




  //1 is x,y position hit of e+, e- in VD
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{e^{#pm}, VD=9}(Mu2e) [mm]", "y_{e^{#pm}, VD=9}(Mu2e) [mm]", VDnumber, 1); 
  //2 is x,y position hit of rest of particles in VD
  //PrintGraph(file_name, -4200, -3600, -300, 300, "x_{#mu^{#pm},#pi^{-},p,#gamma}(Mu2e) [mm]", "y_{#mu^{#pm},#pi^{-},p,#gamma}(Mu2e) [mm]", VDnumber, 2);
  //3 is pt as function of pz in VD 15
  //PrintGraph(file_name, -20, 80, 0, 15, "p_{z, #gamma, VD=15} [MeV]", "p_{T, #gamma, VD=15} [MeV]", VDnumber, 3);
  //4 is x,y position hit of gammas in VD
  //PrintGraph(file_name, -5000, -3000, -1000, 1000, "x_{#gamma, VD=15}(Mu2e) [mm]", "y_{#gamma, VD=15}(Mu2e) [mm]", VDnumber, 4);
  //5 is x,y position hit of gammas in VD (TH2)
  //PrintGraph(file_name, -410, -370, -20, 20, "x_{e^{-}, VD=8}(Mu2e) [cm]", "y_{e^{-}, VD=8}(Mu2e) [cm]", VDnumber, 5);
  //PrintGraph(file_name, -460, -280, -80, 80, "x_{#gamma, VD=10 (cut ST size, p_{z}>0)}(Mu2e) [cm]", "y_{#gamma, VD=10 (cut ST size, p_{z}>0)}(Mu2e) [cm]", VDnumber,5);
  //PrintGraph(file_name, -400, -373, -10, 10, "x_{#gamma_{cos(#theta_{#gamma})>0.99999}, VD=10 (cut ST size, p_{z}>0)}(Mu2e) [cm]", "y_{#gamma_{cos(#theta_{#gamma})>0.99999}, VD=10 (cut ST size, p_{z}>0)}(Mu2e) [cm]", VDnumber, 5);
  //PrintGraph(file_name, -420, -350, -30, 30, "x_{#gamma, VD=81}(Mu2e) [cm]", "y_{#gamma, VD=81}(Mu2e) [cm]", VDnumber, 5);
  //PrintGraph(file_name, -420, -360, -20, 20, "x_{#gamma, VD=86, r<200cm} [cm]", "y_{#gamma, VD=86, r<200cm} [cm]", VDnumber, 5);
  //PrintGraph(file_name, -420, -350, -40, 40, x_gamma_name_char, y_gamma_name_char, VDnumber, 5);
  //6 is to plot pz over time
  //PrintGraph(file_name, 0, 1000, -0.2, 0.4, "t_{#gamma, VD=15} [ns]", "p_{z, #gamma, VD=15} [MeV]", VDnumber, 6);
  //Plot momentum distribution in VDs
  //PrintGraph(file_name, -388, -383, -2.5, 2.5, "x_{#gamma, VD=89,90} [cm]", "y_{#gamma, VD=89,90} [cm]", VDnumber, 8);  
  //Plot costheta and the radius
  //PrintGraph(file_name, -379, -400, -10, 10, "x_{#gamma, VD=88} [cm]", "y_{#gamma, VD=88} [cm]", VDnumber, 9);
  //PrintGraph(file_name, -410, -370, -20, 20, "x_{#mu^{-}, VD=8} [cm]", "y_{#mu^{-}, VD=8} [cm]", VDnumber, 7);
  PrintGraph(file_name, -410, -370, -20, 20, "x_{#mu^{-}, VD=8} [cm]", "y_{#mu^{-}, VD=8} [cm]", VDnumber, 5);

  readfile.close();
}

//================================================================
