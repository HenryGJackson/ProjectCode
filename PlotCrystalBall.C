// #include "/home/henry/Project/TMVA/incl/general.h"
#include "/storage/epp2/phunch/incl/general.h"
#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooCBShape.h"


void PlotCrystalBall(TString fname = "", TString tname = "", Int_t min = 5000, Int_t max = 5500){

      TTree* tree = load::tree(fname,tname);
      if(!tree) return;

      Double_t value;
      tree->SetBranchStatus("*",0);
      tree->SetBranchStatus("B_M",1);
      tree->SetBranchAddress("B_M",&value);

      TH1D* hist = new TH1D("BMass"+fname,"BMass_"+fname,100,min,max);
      for(Int_t i = 0; i < tree->GetEntries(); i++) {
            tree->GetEntry(i);
            if(value < max && value > min) {
                  hist->Fill(value);
            }
      }

      RooRealVar mass("mass","mass",5000,5500);
      RooRealVar mean("mean","mean",5270);
      RooDataHist hBMass("hBMass", "hBMass", mass, RooFit::Import(*hist));


      RooRealVar cbs1("cbs","cbs",50,0.,100);
      RooRealVar cbs2("cbs2","cbs2",50,0.,100.);

      RooRealVar cba1("cba1","cba1",-1,-10,10);
      RooRealVar cbn1("cbn1","cbn1",10,0.,20);
      RooRealVar cba2("cba2","cba2",1,-10,10);
      RooRealVar cbn2("cbn2","cbn2",10,0.,20);

      RooCBShape cb1("cb1","cb1",mass, mean,cbs1,cba1,cbn1);
      RooCBShape cb2("cb2","cb2",mass, mean,cbs2,cba2,cbn2);

      RooRealVar frac("frac","frac",0.5,0,1);

      RooAddPdf  pdf("pdf","pdf",cb1,cb2,frac);
      pdf.fitTo( hBMass );

      RooPlot* frame = mass.frame();
      hBMass.plotOn(frame);
      pdf.plotOn(frame);
      TCanvas *c1 = new TCanvas("BMass", "B Mass Histogram", 400, 400);
      frame->Draw();

      return;
}
