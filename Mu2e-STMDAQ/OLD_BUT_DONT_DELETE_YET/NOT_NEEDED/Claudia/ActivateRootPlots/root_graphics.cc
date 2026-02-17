#include "TF1.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TRootCanvas.h"

int main(int argc, char **argv) {

#if defined(USE_GRAPHICS)
  TApplication app("app", &argc, argv);
#endif

  // This code will just create the png
  TCanvas* c = new TCanvas("c", "Something", 0, 0, 800, 600);
  TF1 *f1 = new TF1("f1","sin(x)", -5, 5);
  f1->SetLineColor(kBlue+1);
  f1->SetTitle("My graph;x; sin(x)");
  f1->Draw();
  c->Modified(); 
  c->Update();
  c->Print("Claudia.png");

#if defined(USE_GRAPHICS)
  TRootCanvas *rc = (TRootCanvas *)c->GetCanvasImp();
  rc->Connect("CloseWindow()", "TApplication", gApplication, "Terminate()");
  app.Run();
#endif
   
  return 0;
}
