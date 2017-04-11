#ifndef CUTLIB_H
#define CUTLIB_H

//NOTES FOR DAN!!!!!!!
//You'll need to change the path of the included files to wherever you store them
//There is an included file at the bottom of each .h file (this isn't proper practice but it works in ROOT)
//NOTES FOR DAN!!!!!!!
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <stdlib.h>

#include "TFile.h"
#include "TTree.h"
#include "TEventList.h"
#include "TString.h"
#include "TPRegexp.h"
#include "TCut.h"
#include "TLorentzVector.h"
#include "TEventList.h"
#include "TROOT.h"
#include <TCanvas.h>
#include <TVector3.h>
#include <TTreeReader.h>
#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooPlot.h"
#include "TBranch.h"
#include "RooExponential.h"
#include "TLine.h"
using namespace RooFit;

//Libs found on phunch storage area "/storage/epp2/phunch/incl/"
//Can use the above as the path^
// #include "/home/henry/Project/TMVA/incl/mvalib.h"
// #include "/home/henry/Project/TMVA/incl/particlefromtree.hpp"
// #include "/home/henry/Project/TMVA/incl/general.h"
#include "/storage/epp2/phunch/incl/mvalib.h"
#include "/storage/epp2/phunch/incl/particlefromtree.hpp"
#include "/storage/epp2/phunch/incl/general.h"
//Namespaces defined in this file: analyse
//                                 punzi
//Classes defined in this file:    exponent

namespace analyse {
      //Where Int_t dataset is an input parameter, thise specifies which of the datasets to use (1-4)
      //    with dataset = 1 -> 2011_MgUp
      //         dataset = 2 -> 2011_MgDn
      //         dataset = 3 -> 2012_MgUp
      //         dataset = 4 -> 2012_MgDn


      //Dan's function. Don't think it's used but included just in case.
      void toggleBranches( TTree* tree );
      //Dan's cut macro (slightly modified)
      void DanCuts(const Double_t BDTcut, const TString inname = "",
                    const TString intreename = "", const TString outname = "",
                    TString p1label = "Kaon", TString p2label = "Mu", TString p3label = "E");
      //Finds the peak Punzi FoM using punzi::GetPunzi(), then calls DanCuts() with the BDT cut given by the peak Punzi FoM
      void MakeAllCuts(Int_t dataset = 0);
      //Trains BDT (if trainBDT==kTRUE) then gets the trees with BDT weights and derived variables.
      //Calls: analyse::GetDerivedVariables(), cut::CutDoubleVar(), MVAanalysis::TMVAClassification,
      //       MVAanalysis::TMVAClassificationApplication(), TMVAtree::GetTMVATrees()
      void MakeTreesWithAllData(Int_t infile = 0, Bool_t trainBDT = kTRUE);
      //MakeTreesWithAllData() followed by MakeAllCuts()
      void DoBoth(Int_t dataset = 0);
      //MakeTreesWithAllData() for every dataset
      void makealltrees();
      //MakeAllCuts() for every dataset
      void cutalldata();
      //makealltrees() followed by cutalldata()
      void everything();
      //Fill the tree with all derived variabls.
      //These are (on 28/02/17): MaxPT, MinPT, MaxIPChi2 MinIPChi2, aKMu, aKe. aMue, mass combos
      void GetDerivedVariables(TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithPTandAngles.root",
		           TString bfname = "/home/henry/Project/BuKMuE_2011_MgUp.root",
		           TString btname = "KMuE/DecayTree", TString p1label = "Kaon", TString p2label = "Mu", TString p3label = "E");
}

class exponent {
//Exponent of the form f(x) = a*e^{(x-c)*b}
	private:
		Double_t a; //Multiplier
		Double_t b;	//Exponent multiplier
		Double_t c; //Mass offset
	public:
		Double_t Evaluate(Double_t x); //Evaluate the exponential at x
		Double_t EvaluateIntegral(Double_t xmin, Double_t xmax); //Evaluate the integral of the exponent over the range xmin to xmax
		Double_t Geta(); //Get the value of a
		Double_t Getb(); //Get the value of b
		Double_t Getc(); //Get the value of the offset
		void aMultiply(Double_t multiplier); //multiply a by a factor multiplier
		void SetParams(Double_t a0, Double_t b0, Double_t c0); //Set the values of the parameters
		void PrintParams(); //Print the values of the parameters
};

namespace punzi{
      //Get the number of background event in the mass region (5000, 5500)
      //If PlotFit==kTRUE then the fit of the B mass in the region above the mass region will be drawn
	Double_t GetBkgEvents(TString filename = "/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root",
				TString treename = "KMuE/DecayTree", Bool_t PlotFit = kFALSE);
      //Get the Punzi FoM for the method with method = "methodname" (for example: method = "BDT")
	Double_t *GetPunzi(const TString method = "", TString mvafilename = "RootFiles/TMVA.root", TString bkgfilename = "BMassCut.root",
				TString bkgtreename = "BkgTree", TString ext = "", Bool_t calculate_Nbkg = kTRUE, Bool_t PlotFit = kFALSE);
      //Get Punzi FoM for MLP, BDT, BDTG
	void GetAllPunzi(TString mvafilename = "RootFiles/TMVA.root",
                        TString bkgfilename = "BMassCut.root", TString bkgtreename = "BkgTree");
}

//Libs found on phunch storage area "/storage/epp2/phunch/src/"
//Can use the above as the path^
// #include "/home/henry/Project/TMVA/src/analysis.C"
#include "/storage/epp2/phunch/src/analysis.C"

#endif
