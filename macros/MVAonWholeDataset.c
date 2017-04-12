#include "TMVAClassification.C"
#include "PunziFOM3.C"

void MVAonWholeDataset() {

	//Perform MVa on Whole Dataset.
	TMVAClassification();

	//Get Punzi Figure of Merit Plots and info for each method
	Double_t* PunziMLP = GetPunzi("MLP");
	Double_t* PunziBDT = GetPunzi("BDT");
	Double_t* PunziBDTG = GetPunzi("BDTG");

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
