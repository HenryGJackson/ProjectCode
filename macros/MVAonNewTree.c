#include "NewTree.C"
#include "TMVAClassification.C"
#include "PunziFOM3.C"

void MVAonNewTree() {

	
	//Perform MVa on new trees.
	TString filename = "RootFiles/NewTreesWithCuts.root";
	TString btreename = "BkgTree";
	TString streename = "MCTree";
	TString mvafilename = "RootFiles/TMVA_NewTreesWithCuts.root";

	NewTree(filename);
	NewTree(filename, "MCTree", "BuKMuE_Signal_MC_MD.root", "KMuE/DecayTree", "UPDATE");

	TMVAClassification(filename, btreename, filename, streename, mvafilename);

	//Get Punzi Figure of Merit Plots and info for each method
	Double_t* PunziMLP  = GetPunzi("MLP", mvafilename, filename, btreename);
	Double_t* PunziBDT  = GetPunzi("BDT", mvafilename, filename, btreename);
	Double_t* PunziBDTG = GetPunzi("BDTG", mvafilename, filename, btreename);

	//Determine the best method
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
