#include "RooDataSet.h"
#include "RooExponential.h"
#include "RooRealVar.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "TString.h"
#include "RooArgSet.h"
#include "RooAbsReal.h"
#include "RooFitResult.h"
#include "TMatrixDSym.h"
#include "TVectorD.h"
#include "TStopwatch.h"

#include <iostream>

#include "RooWorkspace.h"
#include "RooStats/ConfInterval.h"
#include "RooStats/PointSetInterval.h"
#include "RooStats/ConfidenceBelt.h"
#include "RooStats/FeldmanCousins.h"
#include "RooStats/ModelConfig.h"

#include "RooStats/BayesianCalculator.h"
#include "RooStats/SimpleInterval.h"

using namespace RooFit;

struct CBparams {
 // Parameters
      Double_t cb_mean;
      Double_t cb_sigma;
      Double_t cb_alpha;
      Double_t cb_n;
 // Errors
      Double_t cb_merr;
      Double_t cb_serr;
      Double_t cb_aerr;
      Double_t cb_nerr;
      Double_t cb_ferr;
    Double_t cb_f2err;
// Other Crystal Ball parameters in the fit
     CBparams* partner;
// Fraction used in the fit
     Double_t cb_frac; //frac is shared by both crystal ball functions in the fit
                    //so only vary the fraction from one of the CBs
     Double_t cb_f2;
     Int_t ents;
     RooDataHist hist;
};

double evaluate( const Long64_t nbkg, RooAbsPdf& bkg_mass, RooRealVar& M ){

    RooAbsReal* int_sig = bkg_mass.createIntegral( M, NormSet( M ), Range("sig") );
    RooAbsReal* int_bkg = bkg_mass.createIntegral( M, NormSet( M ), Range("low,upp") );

    double val_int_sig = int_sig->getVal();
    double val_int_bkg = int_bkg->getVal();

    delete int_sig;
    delete int_bkg;

    return nbkg*( val_int_sig/val_int_bkg );
}

CBparams* GetSigParams(TString fname = "", TString tname = "", Int_t min = 5000, Int_t max = 5500) {
      //Load tree and fill hist with data in range
      TFile* file = TFile::Open(fname);
      if(!file) {std::cout << "Couldn't load file: " << fname << std::endl; return(NULL);}
      TTree* tree = (TTree*)file->Get(tname);
      if(!tree) {std::cout << "Couldn't load tree " << tname << " from file: " << fname << std::endl; return(NULL);}

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

      //Fit Monte Carlo Data and store fit parameters in struct::CBparams
      RooRealVar mass("mass","mass",min,max);
      RooRealVar mean("mean","mean",hist->GetBinCenter(hist->GetMaximumBin()));
      RooDataHist hBMass("hBMass", "hBMass", mass, RooFit::Import(*hist));
      file->Close();

      RooRealVar cbs1("cbs1","cbs1",20,0.,100);
      RooRealVar cba1("cba1","cba1",1,-10,10);
      RooRealVar cbn1("cbn1","cbn1",10,0.,20);
      RooRealVar cbs2("cbs2","cbs2",20,0.,100.);
      RooRealVar cba2("cba2","cba2",1,-10,10);
      RooRealVar cbn2("cbn2","cbn2",10,0.,20);

      RooCBShape cb1("cb1","cb1",mass, mean,cbs1,cba1,cbn1);
      RooCBShape cb2("cb2","cb2",mass, mean,cbs2,cba2,cbn2);

//      RooRealVar nsig("nsig","#signal events",hist->GetEntries()*0.5,hist->GetEntries()*0.75,hist->GetEntries()) ;
      RooRealVar nsig("nsig","#signal events",0.5,0.0,1.0) ;

      RooArgList pdfs("pdfs");
      pdfs.add(cb1); pdfs.add(cb2);
      RooArgList yield("yield");
      yield.add(nsig);

      RooAddPdf  pdf("pdf","pdf",pdfs,yield);
      pdf.fitTo( hBMass );

      //Store Values of all parameters
      CBparams* cbp1 = new CBparams;
      cbp1->cb_mean = mean.getVal();
      cbp1->cb_sigma = cbs1.getVal();
      cbp1->cb_alpha = cba1.getVal();
      cbp1->cb_n = cbn1.getVal();
      cbp1->cb_frac = nsig.getVal();


      CBparams* cbp2 = new CBparams;
      cbp2->cb_mean = mean.getVal();
      cbp2->cb_sigma = cbs2.getVal();
      cbp2->cb_alpha = cba2.getVal();
      cbp2->cb_n = cbn2.getVal();
      cbp2->cb_frac = nsig.getVal();

      //Store Values of All Errors
      cbp1->cb_merr = mean.errorVar()->getVal();
      cbp1->cb_serr = cbs1.errorVar()->getVal();
      cbp1->cb_aerr = cba1.errorVar()->getVal();
      cbp1->cb_nerr = cbn1.errorVar()->getVal();
      cbp1->cb_ferr = nsig.errorVar()->getVal();

      cbp2->cb_merr = mean.errorVar()->getVal();
      cbp2->cb_serr = cbs2.errorVar()->getVal();
      cbp2->cb_aerr = cba2.errorVar()->getVal();
      cbp2->cb_nerr = cbn2.errorVar()->getVal();
      cbp2->cb_ferr = nsig.errorVar()->getVal();

      //Set Partnership
      cbp1->partner = cbp2;
      cbp2->partner = cbp1;
      if(!cbp1 || !cbp2 || !cbp1->partner || !cbp2->partner) {std::cout << "One of the CBparams pointers in null\n"; return(NULL);}

      RooPlot* frame = mass.frame(RooFit::Title("Superposition of 2 CBs"));
      hBMass.plotOn(frame);
      pdf.plotOn(frame);
      RooHist* resid = frame->residHist();
      RooPlot* frame2 = mass.frame(RooFit::Title("Residual of the fit"));
      frame2->addPlotable(resid,"P") ;

      TCanvas *c1 = new TCanvas("SigBMass"+fname, "Signal B Mass Histogram", 800, 400);
      c1->Divide(2);
      c1->cd(1); frame->Draw();
      c1->cd(2); frame2->Draw();

      // mass.setRange("Signal",5100,5400);
      // RooAbsReal* integ = pdf.createIntegral(mass,RooFit::NormSet(mass),RooFit::Range("Signal"));
      //
      // std::cout << integ->getVal() << std::endl;
      // std::cout << "nsig = " << nsig.getVal() << std::endl;

      return(cbp1);
}

void fituncertainty(const Long64_t bkg_entries,RooArgSet fit_pars,RooFitResult* result,RooAddPdf pdf,RooRealVar mass)
{
      std::cout << "ENTRIES: " << bkg_entries << std::endl;
        //RooFitResult* result = GetParams("MCwithBDTandDerived_CUT.root","DecayTree","BuKMuE_allCuts2.root",kTRUE,kFALSE,5000);

        const TMatrixDSym cov = result->covarianceMatrix();


        const int num_pars = fit_pars.getSize();
        cout << num_pars << endl;
        // Vector to hold derivative
        TVectorD D(num_pars);

        //RooArgSet fit_pars(cba1,cba2,cbn1,cbn2,cbs1,cbs2,C,efrac,frac,cbcoeff,C2,mean);

        const RooArgList final_fit_pars = result->floatParsFinal();
        cout << "Fit Pars" << endl; fit_pars.Print();
        cout << "Final Fit Pars" << endl; final_fit_pars.Print();

        RooRealVar* def_fit_par = 0;
      //   double fn_up, fn_dn;

        mass.setRange( "low", mass.getMin(), 5240 );
        mass.setRange( "upp", 5320, mass.getMax() );
        mass.setRange( "sig", 5240, 5320 );

        for ( unsigned int i = 0; i < num_pars; ++i ){

            // reset the values of the parameters
            for ( unsigned int j = 0; j < num_pars; ++j ){
                def_fit_par = dynamic_cast<RooRealVar*>( &final_fit_pars[j] );
                fit_pars.setRealValue( final_fit_pars[j].GetName(), def_fit_par->getVal() );
            }

            def_fit_par = dynamic_cast<RooRealVar*>( &final_fit_pars[i] );

            // fluctuate up
            fit_pars.setRealValue( def_fit_par->GetName(), def_fit_par->getVal() + sqrt(cov(i,i)));
            double fn_up = evaluate( bkg_entries, pdf, mass );

            // fluctuate dn
            fit_pars.setRealValue( def_fit_par->GetName(), def_fit_par->getVal() - sqrt(cov(i,i)));
            double fn_dn = evaluate( bkg_entries, pdf, mass );

            // Derivative
            D(i) = ( fn_up - fn_dn )/(2*sqrt(cov(i,i)));
            std::cout << "D(" << i << ") = " << D(i);
            std::cout << " || fn_up = " << fn_up << " || fn_dn = " << fn_dn << "\n";

        }

        std::cout << " Shape uncertainty = " << sqrt( D*(cov*D) ) << std::endl;
        cov.Print(); D.Print();

        return ;
}



RooFitResult* GetParams(TString sigfname = "", TString sigtname = "",
                  TString bkgfname = "", TString bkgtname = "", Bool_t KMuE = kTRUE, Bool_t fixedyield = kFALSE,
                  Int_t min = 5000, Int_t max = 5500) {

      CBparams* cbp = GetSigParams(sigfname,sigtname,min,max);
      if(!cbp) {std::cout << "Exiting GetParams()...\n"; return(NULL);}
      if(!cbp->partner) {std::cout << "Exiting GetParams()...\n"; return(NULL);}

      //Load input tree
      TFile* file = TFile::Open(bkgfname);
      if(!file) {std::cout << "Couldn't load file: " << bkgfname << std::endl; return(NULL);}
      TTree* tree = (TTree*)file->Get(bkgtname);
      if(!tree) {std::cout << "Couldn't load tree " << bkgtname << " from file: " << bkgfname << std::endl; return(NULL);}

      Double_t value;
      tree->SetBranchStatus("*",0);
      tree->SetBranchStatus("B_M",1);
      tree->SetBranchAddress("B_M",&value);

      //Create Hist of B Mass within the range
	Int_t intcount = 0;
      TH1D* hist = new TH1D("BMass"+bkgfname,"BMass_"+bkgfname,100,min,max);
      for(Int_t i = 0; i < tree->GetEntries(); i++) {
            tree->GetEntry(i);
            if(value < max && value > min) {
                  hist->Fill(value);
			if(value > 5240 && value < 5340){
				intcount++;
			}
            }
      }
      cbp->ents = hist->GetEntries();
      cbp->partner->ents = hist->GetEntries();



      //Create RooFit Variables
      RooRealVar mass("mass","mass",min,max);
      RooRealVar mean("mean","mean",cbp->cb_mean, cbp->cb_mean*0.9,cbp->cb_mean*1.1);
      RooDataHist hBMass("hBMass", "hBMass", mass, RooFit::Import(*hist));
	file->Close();
	// cbp->hist = hBMass;
	// cbp->partner->hist = hBMass;


      RooRealVar cbs1("cbs1","cbs1",20,0.,100.);
      RooRealVar cba1("cba1","cba1",cbp->cb_alpha);
      RooRealVar cbn1("cbn1","cbn1",cbp->cb_n);
      RooRealVar cbs2("cbs2","cbs2",20,0.,100.);
      RooRealVar cba2("cba2","cba2",cbp->partner->cb_alpha);
      RooRealVar cbn2("cbn2","cbn2",cbp->partner->cb_n);

      //Create Crystal Ball PDFs for signal
      RooCBShape cb1("cb1","cb1",mass, mean,cbs1,cba1,cbn1);
      RooCBShape cb2("cb2","cb2",mass, mean,cbs2,cba2,cbn2);

      //Create Exponential PDFs for Background
      RooRealVar C("C", "exp_param", -0.001, -0.1, 0.0);
      // mass.setRange("belowsig",5000,5200);
      RooExponential bkgExp("bkgExp", "Exponential for fitting B Mass", mass, C);
      RooRealVar C2("C2", "exp_param2", -0.001, -0.1, 0.0);
      // mass.setRange("abovesig",5350,5500);
      RooExponential bkgExp2("bkgExp2", "Exponential for fitting B Mass2", mass, C2);

      //Create combined EXP PDF for background
      RooRealVar efrac("efrac","efrac",0.5,0.0,1.0);
      RooAddPdf Exp("Exp","Exp",bkgExp,bkgExp2,efrac);

      //Create combined CB PDF for signal
      RooRealVar frac("frac","frac",cbp->cb_frac);
      RooAddPdf cb12("cb12","cb12",cb1,cb2,frac);

      //Create full PDF to fit on data
      // RooRealVar cbcoeff("cbcoeff","#signal coeff",0.5,0.0,1.0) ;
      // if(fixedyield) {cbcoeff.setVal(0.0); cbcoeff.setRange(0.0,0.0);}

      RooRealVar nsig("nsig","nsig",cbp->ents*0.5,cbp->ents*0.25,cbp->ents);
      RooRealVar nbkg("nbkg","nbkg",cbp->ents*0.5,0,cbp->ents);
      if(fixedyield) {
            nsig.setVal(0); nsig.setRange(0,0);
            nbkg.setVal(0); nbkg.setRange(0,0);
      }

      RooArgList pdfs("pdfs");
      // pdfs.add(cb12); pdfs.add(Exp);
      if(KMuE) {pdfs.add(cb12); pdfs.add(Exp);}
      else {pdfs.add(cb12); pdfs.add(bkgExp);}
      RooArgList yields("yields");
//      yields.add(cbcoeff);
      yields.add(nsig); yields.add(nbkg);
      RooAddPdf pdf("pdf","pdf",pdfs,yields);

	//Fit and plot
      RooFitResult* result = pdf.fitTo( hBMass, RooFit::Save(kTRUE) );
      RooPlot* frame = mass.frame(RooFit::Title("Superposition of 2 CBs and Exp"));
      hBMass.plotOn(frame);
      pdf.plotOn(frame);

      //Set Values that have changed
      cbp->cb_sigma = cbs1.getVal();
      cbp->cb_mean = mean.getVal();
//	cbp->cb_f2 = cbcoeff.getVal();
      cbp->cb_f2 = nsig.getVal();
      cbp->partner->cb_sigma = cbs2.getVal();
      cbp->partner->cb_mean = mean.getVal();
	// cbp->partner->cb_f2 = cbcoeff.getVal();
      cbp->partner->cb_f2 = nsig.getVal();

      cbp->cb_serr = cbs1.errorVar()->getVal();
      cbp->cb_merr = mean.errorVar()->getVal();
	// cbp->cb_f2err = cbcoeff.errorVar()->getVal();
      cbp->cb_f2err = nsig.errorVar()->getVal();
      cbp->partner->cb_serr = cbs2.errorVar()->getVal();
      cbp->partner->cb_merr = mean.errorVar()->getVal();
	// cbp->partner->cb_f2err = cbcoeff.errorVar()->getVal();
      cbp->partner->cb_f2err = nsig.errorVar()->getVal();

      //Get Residual
      RooHist* resid = frame->residHist();
      RooPlot* frame2 = mass.frame(RooFit::Title("Residual of fit"));
      frame2->addPlotable(resid,"P") ;

      TCanvas *c1 = new TCanvas("BMassFixedParams"+bkgfname, "B Mass Histogram With Params from Signal fit", 800, 400);
      c1->Divide(2);
      c1->cd(1); frame->Draw();
      c1->cd(2); frame2->Draw();


      mass.setRange("Signal",5240,5320);
	std::cout << "\n\n\n\n\n\n\n************************************************************************\nInfo From Actual Fit of Data:\n************************************************************************\n";
      RooAbsReal* CBinteg = cb12.createIntegral(mass,RooFit::NormSet(mass),RooFit::Range("Signal"));
//      RooAbsReal* EXPinteg = Exp.createIntegral(mass,RooFit::NormSet(mass),RooFit::Range("Signal"));
      // std::cout << "ACTUAL INTEGRAL:                                 " << cbp->cb_f2*CBinteg->getVal()*cbp->ents << std::endl;
      // std::cout << "Multiplier =                                     " << cbp->cb_f2 << " * " << CBinteg->getVal() << " * " << cbp->ents << std::endl;
	std::cout << "Events within integration range (including bkg): " << intcount << std::endl;
      std::cout << "YIELD:                                           " << nsig.getVal() << std::endl;
      // RooArgSet fitparams(C,C2,cbcoeff,cbs1,cbs2,efrac,mean);
      RooArgSet fitparams(C,C2,nsig,cbs1,cbs2,efrac,mean);
      fituncertainty(cbp->ents, fitparams, result, pdf, mass);

      return(result);
}


Double_t* error(Bool_t BKMuE = kTRUE) {
	TString sf, bf;
	Int_t masslowbound;
	RooFitResult* fixedyield,*floatyield;
	if(BKMuE) {
		TString sf = "MCwithBDTandDerived_CUT.root";
		TString bf = "BuKMuE_allCuts2.root";
		masslowbound = 5000;
		floatyield = GetParams(sf,"DecayTree",bf,"DecayTree",BKMuE,kTRUE,masslowbound);
            fixedyield = GetParams(sf,"DecayTree",bf,"DecayTree",BKMuE,kFALSE,masslowbound);

	}
	else {
		TString sf = "BuKMuMu_MC_Derived.root";
		TString bf = "TigCutsBuKMuMu.root";
		masslowbound = 5175;
		floatyield = GetParams(sf,"DecayTree",bf,"DecayTree",BKMuE,kFALSE,masslowbound);
            fixedyield = GetParams(sf,"DecayTree",bf,"DecayTree",BKMuE,kTRUE,masslowbound);
	}
//	if(!cbp1) {std::cout << "Exiting: error()\n"; return(NULL);}

      Double_t fixedlikelihood = fixedyield->minNll();
      Double_t floatlikelihood = floatyield->minNll();

      std::cout << "Fixed: " << fixedlikelihood << "\n";
      std::cout << "Float: " << floatlikelihood << "\n";


      return(NULL);
}
