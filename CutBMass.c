#include "Variable.h"
#include "GeneralFunctions.C"

void CutBMass(Double_t value = 5700.0, Bool_t overwrite = kFALSE, TString outfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/original/withBcut_2011_MgDn.root",
	TString filename = "/home/epp/phunch/Documents/TMVA/RootFiles/original/2011_MgDn_WithDerived.root",
	TString treename = "DecayTree") {

	TString newname = treename;
	TString fileoption;
	if (overwrite) fileoption = "RECREATE";
	else		   fileoption = "UPDATE";

	//Get Tree from file
	TFile *file = TFile::Open(filename);
	TTreeReader Reader(treename, file);
	TTree *ttree = Reader.GetTree();

	//Create B Mass Variable object
	Variable* BMass = new Variable;
	BMass->SetName("B_M");
	BMass->SetCutVal(5700.0);
	BMass->SetCutDir(1);
	BMass->SetNext(NULL);


	//Open new file and save new tree with cuts to file
	TFile* newfile = new TFile(outfilename, fileoption);
	TTree* newtree = MakeCuts(BMass, ttree, newfile, newname);

	//Write tree and close files
	file->Close();
	newtree->Write();
	newfile->Close();

	delete BMass;
	return;
}

void CutHasBremAdded() {
	TString ifname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllDataCombined_afterCuts.root";
	TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllData_Cut.root";
	TFile* file = TFile::Open(ifname);
	TTree* tree = (TTree*)file->Get("DecayTree");

	tree->SetBranchStatus("*",1);
	Bool_t tmp;
	tree->SetBranchAddress("E_HasBremAdded",&tmp);

	TFile* ofile = new TFile(ofname, "RECREATE");
	TTree* ntree = tree->CloneTree(0);

	for(Int_t i = 0; i < tree->GetEntries(); i++) {
			tree->GetEntry(i);
			if(tmp == kFALSE) {
					ntree->Fill();
			}
	}

	file->Close();
	ntree->Write();
	ofile->Close();
	return;
}
