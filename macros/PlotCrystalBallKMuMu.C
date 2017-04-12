// #include "/home/henry/Project/TMVA/incl/general.h"
#include "/storage/epp2/phunch/incl/general.h"
#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooCBShape.h"
#include <math.h>

#define PI 3.1415926
//
// class CrystBall{
// private:
//       Double_t A;
//       Double_t B;
//       Double_t C;
//       Double_t D;
//       Double_t N;
//       Double_t sigma;
//       Double_t alpha;
//       Double_t n;
//       Double_t mean;
//       Bool_t error = kFALSE;
//       Bool_t paramsset = kFALSE;
//       void evaluateA();
//       void evaluateB();
//       void evaluateC();
//       void evaluateD();
//       void evaluateN();
//       Double_t GetExponent(Double_t x);
//       Double_t GetExpVal(Double_t x);
//       Double_t LowTermInteg(Double_t high, Double_t low);
//       Double_t HighTermInteg(Double_t high, Double_t low);
// public:
//       void SetParams(Double_t sigma_, Double_t n_, Double_t alpha_, Double_t mean_);
//       void CalcABCDN();
//       Bool_t checkError();
//       Double_t evaluateIntegral(Double_t x);
// };
//
// void CrystBall::SetParams(Double_t sigma_, Double_t n_, Double_t alpha_, Double_t mean_) {
//       sigma = sigma_;
//       n = n_;
//       alpha = alpha_;
//       mean = mean_;
//       return;
// }
// void CrystBall::evaluateA() {
//       if(alpha==0) {std::cout << "alpha cannot be zero\n"; error = kTRUE; return;}
//       if(alpha<0) alpha = alpha*(-1);
//       A =  pow(n/alpha,n)*exp((-0.5)*alpha*alpha);
//       return;
// }
// void CrystBall::evaluateB() {
//       if(alpha==0) {std::cout << "alpha cannot be zero\n"; error = kTRUE; return;}
//       if(alpha<0) alpha = alpha*(-1);
//       B = (n/alpha)-alpha;
//       return;
// }
// void CrystBall::evaluateC() {
//       if(alpha==0) {std::cout << "alpha cannot be zero\n"; error = kTRUE; return;}
//       if(alpha<0) alpha = alpha*(-1);
//       C = (n/alpha)*(1/(n-1))*exp((-0.5)*alpha*alpha);
//       return;
// }
// void CrystBall::evaluateD() {
//       if(alpha==0) {std::cout << "alpha cannot be zero\n"; error = kTRUE; return;}
//       if(alpha<0) alpha = alpha*(-1);
//       D = sqrt(0.5*PI)*(1+erf(alpha/sqrt(2)));
//       return;
// }
// void CrystBall::evaluateN() {
//       N = 1/(sigma*(C+D));
//       return;
// }
// void CrystBall::CalcABCDN() {
//       evaluateA();
//       evaluateB();
//       evaluateC();
//       evaluateD();
//       evaluateN();
//       paramsset = kTRUE;
//       return;
// }
// Bool_t CrystBall::checkError() {
//       if(error) {std::cout << "Crystal Ball has encountered an error\n"; return(kTRUE);}
//       return(kFALSE);
// }
//
// Double_t CrystBall::GetExponent(Double_t x) {
//       return( (x-mean)/sigma );
// }
//
// Double_t CrystBall::LowTermInteg(Double_t high, Double_t low) {
//       Double_t integral = (A*sigma/n)*pow(B - ((high-mean)/sigma),1-n);
//       integral -= (A*sigma/n)*pow(B - ((low-mean)/sigma),1-n);
//       return( integral*N );
// }
//
// Double_t CrystBall::GetExpVal(Double_t x) {
//       return( N*exp((-0.5)*GetExponent(x)*GetExponent(x)) );
// }
//
// Double_t CrystBall::HighTermInteg(Double_t high, Double_t low) {
//       Double_t iter = 1000;
//
//       Double_t h = (high - low)/iter;
//       Double_t sum = GetExpVal(high) + GetExpVal(low);
//       for(Int_t i = 1; i < iter; i++) {
//             sum += 2*GetExpVal(low + i*h);
//       }
//       sum = sum*0.5*h;
//       return(sum);
// }
//
// Double_t CrystBall::evaluateIntegral(Double_t x) {
//       if(!paramsset) CalcABCDN();
//
//       Double_t expo = GetExponent(x);
//       Double_t integral;
//       if(expo<alpha) integral = LowTermInteg(x,0.0);
//       else {
//             integral = LowTermInteg(alpha,0.0)*N;
//             integral += HighTermInteg(x,alpha)*N;
//       }
//       return(integral);
// }

void PlotCrystalBall(TString fname = "", TString tname = "", Bool_t hasBkg = kFALSE, Int_t min = 5000, Int_t max = 5500)
{

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
      if(!hasBkg) {
            RooRealVar mass("mass","mass",min,max);
            RooRealVar mean("mean","mean",hist->GetBinCenter(hist->GetMaximumBin()));
            RooDataHist hBMass("hBMass", "hBMass", mass, RooFit::Import(*hist));

            RooRealVar cbs1("cbs1","cbs1",20,0.,100);
            RooRealVar cba1("cba1","cba1",1,-10,10);
            RooRealVar cbn1("cbn1","cbn1",10,0.,20);
            RooRealVar cbs2("cbs2","cbs2",20,0.,100.);
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

            mass.setRange("Signal",5100,5400);
            RooAbsReal* integ = pdf.createIntegral(mass,RooFit::NormSet(mass),RooFit::Range("Signal"));

            std::cout << integ->getVal() << std::endl;
      }
      else {
            RooRealVar mass("mass","mass",min,max);
            RooRealVar mean("mean","mean",hist->GetBinCenter(hist->GetMaximumBin()));
            RooDataHist hBMass("hBMass", "hBMass", mass, RooFit::Import(*hist));

            RooRealVar cbs1("cbs1","cbs1",20,0.,100);
            RooRealVar cba1("cba1","cba1",1,-10,10);
            RooRealVar cbn1("cbn1","cbn1",10,0.,20);
            RooRealVar cbs2("cbs2","cbs2",20,0.,100.);
            RooRealVar cba2("cba2","cba2",1,-10,10);
            RooRealVar cbn2("cbn2","cbn2",10,0.,20);

            RooCBShape cb1("cb1","cb1",mass, mean,cbs1,cba1,cbn1);
            RooCBShape cb2("cb2","cb2",mass, mean,cbs2,cba2,cbn2);

      	RooRealVar C("C", "exp_param", -0.001, -0.01, 0.0);
            RooExponential bkgExp("bkgExp", "Exponential for fitting B Mass", mass, C);

            RooRealVar fracCB("fracCB","fracCB",0.5,0,1);
            RooRealVar fracEXP("fracEXP","fracEXP",0.5,0,1);

            RooArgList pdfs("pdfs");
            pdfs.add(cb1); pdfs.add(cb2); pdfs.add(bkgExp);
            RooArgList coeffs("coeffs");
            coeffs.add(fracCB); coeffs.add(fracEXP);
            RooAddPdf pdf("pdf","pdf",pdfs,coeffs);

            pdf.fitTo( hBMass );

            RooPlot* frame = mass.frame();
            hBMass.plotOn(frame);
            pdf.plotOn(frame);
            TCanvas *c1 = new TCanvas("BMass", "B Mass Histogram", 400, 400);
            frame->Draw();

            RooArgSet analytic(mass,"deps");
            RooArgSet all(mass,mean,cbs1,cba1,cbn1,"all");
            RooArgSet all2(mass,mean,cbs2,cba2,cbn2,"all2");
            std::cout << cb1.analyticalIntegral(cb1.getAnalyticalIntegral(all, analytic)) + fracCB.getValV()*cb2.analyticalIntegral(cb2.getAnalyticalIntegral(all2, analytic)) <<std::endl;
      }
      // CrystBall* cball = new CrystBall;
      // cball->SetParams(cbs1.getValV(),cbn1.getValV(),cba1.getValV(),hist->GetBinCenter(hist->GetMaximumBin()));
      // cball->CalcABCDN();
      // if(cball->checkError()) return;
      // Double_t integ = cball->evaluateIntegral(5250);
      //
      // std::cout << integ << std::endl;

      return;
}
