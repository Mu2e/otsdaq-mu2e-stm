void GraphDoubleString(TGraph *gr, std::string c[]) {
  auto c1= new TCanvas("c1");

  gr->Draw("ALP");
  gr->GetXaxis()->SetLabelSize(0);
  gr->GetXaxis()->SetTickLength(0);

  Int_t i,n;
  Double_t x,y;
  TLatex *t;
  TLine *tick;
  TLine *grid;
  printf("%s\n",c[0].c_str());

  double ymin = gr->GetHistogram()->GetMinimum();
  double ymax = gr->GetHistogram()->GetMaximum();
  double dy   = (ymax-ymin);
  n = gr->GetN();

  for (i=0; i<n; i++) {
    gr->GetPoint(i,x,y);
    t = new TLatex(x, ymin-0.05*dy, c[i].c_str());
    t->SetTextSize(0.05);
    t->SetTextFont(42);
    t->SetTextAlign(21);
    t->Draw();
    tick = new TLine(x,ymin,x,ymin+0.03*dy);
    tick->Draw();
    grid = new TLine(x,ymin,x,y);
    grid->SetLineStyle(3);
    grid->Draw();
  }

  //c1->Print("CalibrationE-ADCFit_Chi2_40K_peak4.png","png");
}

void plotChi2() {

  gROOT->SetStyle("ATLAS");

  int peaks_i[7] = {0,1,2,3,4,5,6};
  Double_t ADCs[7] ={-640,-790,-938.831,-1624.92,-3892.31,-4702.46,-6965.78};
  Double_t fit_E[3] = {511,1460.8,2614.51};
  Double_t fit_ADC[3];
  Double_t fit_E_err[3] = {0,0,0};
  Double_t fit_ADC_err[3] = {10,10,10};
  Double_t y1[9]={1,2,3,4,5,6,7,8,9};
  std::string peaks_1[9];
  Double_t chi2_1[9];
  Double_t y2[8]={1,2,3,4,5,6,7,8};
  std::string peaks_2[8];
  Double_t chi2_2[9];

  int peak_K = 4;
  int peak_Pb, peak_511;
  int k = 0;

  TGraphErrors* ge;
  TF1* FitCalibrationErrors;
  TGraph* gr;

  int n=3;

  //Select peaks to do the fit
  if(peak_K==3){
    for(int i=0;i<=2;i++){
      peak_511=i;
      for(int j=4;j<=6;j++){
	peak_Pb=j;

	fit_ADC[0]=ADCs[peak_511];
	fit_ADC[1]=ADCs[peak_K];
	fit_ADC[2]=ADCs[peak_Pb];

	ge=new TGraphErrors(n,fit_E,fit_ADC,fit_E_err,fit_ADC_err);
	FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -100, 3000);
	FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
	ge->Fit(FitCalibrationErrors,"0","",-100,3000);
	FitCalibrationErrors->SetLineColor(kRed);
	double Chi2errors;
	Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
	cout<<"Chi2= "<<Chi2errors<<endl;
	chi2_1[k] = Chi2errors;
	peaks_1[k] = "["+std::to_string(peak_511)+","+std::to_string(peak_Pb)+"]";
	k++;
	std::cout<<"fit_E[0]="<<fit_E[0]<<" fit_ADC[0]="<<fit_ADC[0]<<std::endl;
	std::cout<<"fit_E[1]="<<fit_E[1]<<" fit_ADC[1]="<<fit_ADC[1]<<std::endl;
	std::cout<<"fit_E[2]="<<fit_E[2]<<" fit_ADC[2]="<<fit_ADC[2]<<std::endl;
	ge->SetMarkerStyle(20);
	//ge->Draw("ap");
	//FitCalibrationErrors->Draw("same,l");
      }
      
    }
    
  }

  //Select peaks to do the fit
  if(peak_K==4){
    for(int i=0;i<=3;i++){
      peak_511=i;
      for(int j=5;j<=6;j++){
	peak_Pb=j;

	fit_ADC[0]=ADCs[peak_511];
	fit_ADC[1]=ADCs[peak_K];
	fit_ADC[2]=ADCs[peak_Pb];

	ge=new TGraphErrors(n,fit_E,fit_ADC,fit_E_err,fit_ADC_err);
	FitCalibrationErrors = new TF1("FitCalibrationErrors", "[1]*x+[0]", -100, 3000);
	FitCalibrationErrors->SetParameters(-2.81467e-01,-1.74204e+00);
	ge->Fit(FitCalibrationErrors,"0","",-100,3000);
	FitCalibrationErrors->SetLineColor(kRed);
	double Chi2errors;
	Chi2errors=ge->GetFunction("FitCalibrationErrors")->GetChisquare();
	cout<<"Chi2= "<<Chi2errors<<endl;
	chi2_2[k] = Chi2errors;
	peaks_2[k] = "["+std::to_string(peak_511)+","+std::to_string(peak_Pb)+"]";
	k++;
	std::cout<<"fit_E[0]="<<fit_E[0]<<" fit_ADC[0]="<<fit_ADC[0]<<std::endl;
	std::cout<<"fit_E[1]="<<fit_E[1]<<" fit_ADC[1]="<<fit_ADC[1]<<std::endl;
	std::cout<<"fit_E[2]="<<fit_E[2]<<" fit_ADC[2]="<<fit_ADC[2]<<std::endl;
	ge->SetMarkerStyle(20);
	//ge->Draw("ap");
	//FitCalibrationErrors->Draw("same,l");
      }
      
    }
    
  }
 
  int v;
  if(peak_K==3){v=9;gr = new TGraph(v,y1,chi2_1);}
  if(peak_K==4){v=8;gr = new TGraph(v,y2,chi2_2);}

  std::string Xtitle = "Peaks[Annihilation,208-Pb], with 40-K=peak "+std::to_string(peak_K);
  char* Xtitle_char = const_cast<char*>(Xtitle.c_str());
  gr->GetYaxis()->SetTitle("Fit #chi^{2}");
  gr->GetXaxis()->SetTitle(Xtitle_char);
  gr->SetMarkerStyle(20);

  if(peak_K==3){GraphDoubleString(gr,peaks_1);}
  if(peak_K==4){GraphDoubleString(gr,peaks_2);}
}
