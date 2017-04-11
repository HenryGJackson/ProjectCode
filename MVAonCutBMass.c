#include "CutBMass.c"
#include "TMVAClassification.C"
#include "Variable.h"
#include "GeneralFunctions.C"
#include "PunziFOM3.C"

void MVAonCutBMass() {

	CutBMass(kTRUE, kTRUE);		//Cut on B Mass and overwrite previous file to write BkgTree.
	CutBMass(kFALSE, kFALSE);	//Cut on B Mass and update previous file with MCTree.

	//Perform MVa on new trees.
	TString filename    = "RootFiles/BMassCut.root";
	TString btreename   = "BkgTree";
	TString streename   = "MCTree";
	TString outfilename = "RootFiles/TMVA_BMassCut.root";
	TMVAClassification(filename, btreename, filename, streename, outfilename);

	Double_t* PunziMLP  = GetPunzi("MLP", outfilename, filename, btreename);
	Double_t* PunziBDT  = GetPunzi("BDT", outfilename, filename, btreename);
	Double_t* PunziBDTG = GetPunzi("BDTG", outfilename, filename, btreename);

	TString bestmethod;
	Double_t max;
	if (PunziMLP[0] > PunziBDT[0]) {
		max = PunziMLP[0];
		bestmethod = "MLP";
	}
	else {
		max = PunziBDT[0];
		bestmethod = "BDT";
	}
	if (PunziBDTG[0] > max) {
		max = PunziBDTG[0];
		bestmethod = "BDTG";
	}

	std::cout << "The method with best PFoM is: " << bestmethod << " with peak PFoM = " << max << std::endl;
	return;
}