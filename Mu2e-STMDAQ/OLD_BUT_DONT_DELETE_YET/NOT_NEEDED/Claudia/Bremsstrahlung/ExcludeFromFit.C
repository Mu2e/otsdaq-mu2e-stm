// example to illustrate how to fit excluding points in a given range
// Author: Rene Brun
#include "TH1.h"
#include "TF1.h"

bool reject;
double xreject[2];

double fline(Double_t *x, Double_t *par)
{
  if (reject && x[0] > xreject[0] && x[0] < xreject[1]) {
    TF1::RejectPoint();
    return 0;
  }
  return par[0] + par[1]*x[0];
}

void ExcludeFromFit() {
  //Create a source function
  TF1 *f1 = new TF1("f1","[0] +[1]*x +gaus(2)",0,5);
  f1->SetParameters(6,-1,5,3,0.2);
  // create an histogram and fill it according to the source function
  TH1F *h = new TH1F("h","background + signal",100,0,5);
  h->FillRandom("f1",20000);
  h->Draw("");
  TF1 *fl = new TF1("fl",fline,0,5,2);
  fl->SetParameters(2,-1);
  //we want to fit only the linear background excluding the signal area
  reject = true;
  xreject[0]=2.5;
  xreject[1]=3.5;
  h->Fit(fl,"0");
  //fl->Draw("");
  double chi2reject = fl->GetChisquare();
  std::cout<<"Chi2: "<<chi2reject<<std::endl;

  reject = false;
  TF1 *fl2 = new TF1("fl",fline,0,5,2);
  h->Fit(fl2,"0");
  double chi2noreject = fl2->GetChisquare();
  std::cout<<"Chi2: "<<chi2noreject<<std::endl;
  //fl2->Draw("same");
  //store 2 separate functions for visualization
  TF1 *fleft = new TF1("fleft",fline,0,xreject[0],2);
  fleft->SetParameters(fl->GetParameters());
  h->GetListOfFunctions()->Add(fleft);
  gROOT->GetListOfFunctions()->Remove(fleft);
  TF1 *fright = new TF1("fright",fline,xreject[1],5,2);
  fright->SetParameters(fl->GetParameters());
  h->GetListOfFunctions()->Add(fright);
  gROOT->GetListOfFunctions()->Remove(fright);
  //h->Draw("same");
  //h->Fit("fl","l");

  /*double par1 = 0., par2 = 2.;
    auto fit_lamb = [=](const double *x, const double *) {return par1 + x[0] * par2; };
    TF1 *fit = new TF1("fit", fit_lamb, 0., 6., 0);
    fit->Print();
    new TCanvas();
    fit->Draw();*/
}
