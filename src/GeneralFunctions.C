#ifndef GENERALFUNCTIONS_C
#define GENERALFUNCTIONS_C

#include <string>
#include <stdlib.h>

#include "TFile.h"
#include "TTree.h"
#include "TEntryList.h"
#include "TH1D.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TBranch.h"

#include "Variable.h"

TTree* GetTree(Bool_t GetBkg) {
	if (GetBkg) {
		TFile* infile = TFile::Open("BuKMuE_2011_MgUp.root");
		if (!infile) return(NULL);
		TTree* intree = (TTree*)infile->Get("KMuE/DecayTree");
		if (!intree) return(NULL);
		return(intree);
	}
	else {
		TFile* infile = TFile::Open("BuKMuE_Signal_MC_MD.root");
		if (!infile) return(NULL);
		TTree* intree = (TTree*)infile->Get("KMuE/DecayTree");
		if (!intree) return(NULL);
		return(intree);
	}
}

TEntryList* GetEList(TTree* tree, Bool_t GetBkg) {
	if (GetBkg) {
		TEntryList* list = new TEntryList("BkgList", "List of Background entries to keep", tree);
		for (int i = 0; i < tree->GetEntries(); i++) {
			list->Enter(i, tree);
		}
		if (list->GetN() == 0) return(NULL);
		return(list);
	}
	else {
		TEntryList* list = new TEntryList("MCList", "List of Monte Carlo entries to keep", tree);
		for (int i = 0; i < tree->GetEntries(); i++) {
			list->Enter(i, tree);
		}
		if (list->GetN() == 0) return(NULL);
		return(list);
	}
}

Variable* CreateVarList(TTree* BkgTree) {
	//Open two instances of the file to read the variables from
	TString filestr = "NamesAndCuts.txt";
	std::ifstream file(filestr);
	std::ifstream file2(filestr);

	//Create two pointers to Variable objects, seed to point to the head of the
	//   list and temp to point to each new Variable created.
	Variable *seed = new Variable;
	Variable *temp = NULL;
	TString  name_tmp;
	TString type;
	Double_t min;
	Double_t max;
	Double_t cut_tmp;
	Int_t	 cut_int;
	Int_t    data_int;

	std::string line; getline(file, line);
	Int_t vars = atoi(line.c_str());
	if (vars == 0) return(NULL);

	file2.seekg(2, ios::beg);
	for (Int_t i = 0; i < vars; i++) {

		//if list is not empty add a new entry to the end of the list
		if (temp != NULL) {
			temp->SetNext(new Variable);
			temp = temp->GetNext();
		}
		//else if list is empty create a new one.
		else temp = seed;
		temp->NotDerived();
		//Get the Variable name from the file
		getline(file, line);
		name_tmp = line;
		temp->SetName(name_tmp);
		//		std::cout << "Variable Name: " << temp->GetName() << std::endl;

		//Set the limits of this variable
		getline(file, line);
		min = atof(line.c_str());
		getline(file, line);
		max = atof(line.c_str());
		temp->SetLimits(min, max);
		//		std::cout << "Limits: " << temp->GetMin() << " - " << temp->GetMax() << std::endl;

		//Set the cut value of thhis variable
		getline(file, line);
		cut_tmp = atof(line.c_str());
		temp->SetCutVal(cut_tmp);
		//		std::cout << "Cut Value: " << temp->GetCutVal() << std::endl;

		//Tell whether to cut above or below the cut value
		getline(file, line);
		cut_int = atoi(line.c_str());
		temp->SetCutDir(cut_int);
		//		std::cout << "The Cut is Less than the Cut Value: " << temp->GetCutDir() << std::endl;

		getline(file, line);
		data_int = atoi(line.c_str());
		temp->SetHasMC(data_int);

		getline(file, line);
		temp->SetType(line);
		if(temp->GetType() == "double") BkgTree->SetBranchAddress(temp->GetName(), &temp->bkg_value);
		else if(temp->GetType() == "bool") BkgTree->SetBranchAddress(temp->GetName(), &temp->bool_value);
		else BkgTree->SetBranchAddress(temp->GetName(), &temp->mc_value);
		//Set the branch status and address for this variable
		//	BkgTree->SetBranchStatus(temp->GetName(), 1);
		//	if (temp->HasMC()) MCTree->SetBranchStatus(temp->GetName(), 1);
		//	if (temp->HasMC()) MCTree->SetBranchAddress(temp->GetName(), &temp->mc_value);
	}

	file.close();
	file2.close();
	return(seed);
}



TTree* MakeCuts(Variable* seed, TTree* ttree, TFile *newfile, TString newtreename = "DecayTree") {

	//Clone Empty Tree
	newfile->cd();
	TTree* newtree = ttree->CloneTree(0);
	newtree->SetName(newtreename);

	Variable* temp = seed;
	Int_t variable_count = 0;
	TCut cuts;

	//Loop through variables setting up the cut then adding it to the cuts to be applied
	while (temp) {
		temp->SetCut();
		cuts = cuts + temp->GetCut();
		temp = temp->GetNext();
	}

	//Get the event list with the cuts
	TEventList events("event_list", "events");
	ttree->Draw(">>event_list", cuts);

	//Fill the new tree with the cuts applied
	for (int i = 0; i < events.GetN(); ++i) {
		ttree->GetEntry(events.GetEntry(i));
		newtree->Fill();
		std::cout << "\r" << "Make Cuts Progrss: " << std::setw(4) << i * 100 / (events.GetN() - 1) << " %." << std::flush;
	}
	std::cout << "\n";
	return(newtree);
}



void DrawHist(TBranch* branch, Int_t min, Int_t max) {
	TCanvas* canv = new TCanvas("canv", "canv", 400, 400);
	TH1D* hist = new TH1D("hist", "hist", 100, min, max);
	Double_t value;
	branch->SetAddress(&value);
	for (Int_t i = 0; i < branch->GetEntries(); i++) {
		branch->GetEntry(i);
		hist->Fill(value);
	}
	canv->cd();
	hist->Draw();
	return;
}

void DrawBMass(const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
	TCanvas* canv = new TCanvas("canv", "B Mass", 400, 400);
	TH1D* hist = new TH1D("BMass", "B Mass", 100, 4400, 6200);
	TFile* tfile = TFile::Open(filename);
	TTree* ttree = (TTree*)tfile->Get(treename);
	ttree->SetBranchStatus("*", 0);
	ttree->SetBranchStatus("B_M", 1);
	Double_t value;
	ttree->SetBranchAddress("B_M", &value);
	for (Int_t i = 0; i < ttree->GetEntries(); i++) {
		ttree->GetEntry(i);
		hist->Fill(value);
	}
	canv->cd();
	hist->Draw();
	tfile->Close();
	return;
}



#endif
