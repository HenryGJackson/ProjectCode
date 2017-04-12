#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>


#include "TTree.h"
#include "TEventList.h"
#include "TBranch.h"
#include "TFile.h"
#include "TVector3.h"
#include "TCut.h"
#include "TString.h"
#include "TLorentzVector.h"

#include "Variable.h"
#include "particlefromtree.hpp"
#include "GeneralFunctions.C"

Float_t GetAngle(TLorentzVector V1, TLorentzVector V2) {
	return(V1.Angle(V2.Vect()) );
}

Int_t CheckBranch(TBranch *branch, Double_t min, Double_t max) {
	Double_t value;
	branch->SetAddress(&value);
	Int_t high = 0;
	Int_t low = 0;
	for (Int_t i = 0; i < branch->GetEntries(); i++) {
		branch->GetEntry(i);
		if (value > max) high++;
		else if (value < min) low++;
	}
	if (high != 0 || low != 0) {
		std::cout << "Branch has Value Error with: " << low << " below the minimum " << min << std::endl;
		std::cout << "                             " << high << " above the maximum " << max << std::endl;
		return(1);
	}
	else return(0);
}

Int_t CheckBranchFromFile(const TString branch = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
	TFile* tfile = TFile::Open(filename);
	TTree* ttree = (TTree*)tfile->Get(treename);
	Float_t value;
	ttree->SetBranchStatus("*", 0);
	ttree->SetBranchStatus(branch, 1);
	ttree->SetBranchAddress(branch, &value);
	Float_t min = 0.0;
	Float_t max = 1.0;
	Int_t high = 0;
	Int_t low = 0;
	for (Int_t i = 0; i < ttree->GetEntries(); i++) {
		ttree->GetEntry(i);
		if (value > max) high++;
		else if (value < min) low++;
	}
	if (high != 0 || low != 0) {
		std::cout << "Branch has Value Error with: " << low << " below the minimum " << min << std::endl;
		std::cout << "                             " << high << " above the maximum " << max << std::endl;
		return(1);
	}
	else return(0);
}

void NewTree(const TString fileoption = "UPDATE", const TString treename = "KMuE/DecayTree", const TString filename = "/home/henry/Documents/Project/BuKMuE_Signal_MC_MD.root",
					   const TString newfilename = "NewTreesWithCuts.root", const TString newtreename = "BkgTree"){



	TFile* tfile = TFile::Open(filename);
	TTree* ttree = (TTree*)tfile->Get(treename);

	//Create List of Variables to Make cuts on
/*	std::cout << "\r" << "Creating Var List" << std::flush;
	Variable* seed = CreateVarList(ttree);						//Class from:   Variable.h
	if (seed == NULL) {
		std::cout << "Variable List is Empty\n";
		return;
	}

	TFile* newfile = new TFile(newfilename, fileoption);
	//Get clone of tree with cuts on non-derived variables
	std::cout << "\r" << "Making Cuts                 " << std::flush;
	TTree* newtree = MakeCuts(seed, ttree, newfile, newtreename);			//Function from:   GeneralFunctions.C
	std::cout << "Efficiency after Cuts: " << (Double_t)newtree->GetEntries() / ttree->GetEntries() << std::endl;
*/
//	DeleteList(seed);
/*	tfile->Close();
	newtree->Write();
	newfile->Close();

	return;*/

	//Setup List of Derived Variables
	Variable* aKMu = new Variable;
	aKMu->DerivedSetup("aKMu", 0.0005, 1, 0);

	Variable* aKe = new Variable;
	aKe->DerivedSetup("aKe", 0.0005, 1, 0);
	aKe->SetNext(aKMu);

	Variable* aMue = new Variable;
	aMue->DerivedSetup("aMue", 0.0005, 1, 0);
	aMue->SetNext(aKe);

	Float_t val1, val2, val3;
	Float_t max = 0.0;
	TBranch* BaKMu = newtree->Branch(aKMu->GetName(), &val1, aKMu->GetName());
	TBranch* BaKe = newtree->Branch(aKe->GetName(), &val2, aKe->GetName());
	TBranch* BaMue = newtree->Branch(aMue->GetName(), &val3, aMue->GetName());

	const bool istruth = false;
	Particle<Double_t> kaon("Kaon", ttree, istruth);			//Class from:   particlefromtree.cpp
	Particle<Double_t> muon("Mu", ttree, istruth);  			//Class from:   particlefromtree.cpp
	Particle<Double_t> electron("E", ttree, istruth);			//Class from:   particlefromtree.cpp

	newtree->SetBranchStatus("*",0);
	newtree->SetBranchStatus(aKMu->GetName(),1);
	newtree->SetBranchStatus(aKe->GetName(),1);
	newtree->SetBranchStatus(aMue->GetName(),1);

	//Loop through entries in the tree filling branches for the derived variables
	for (Int_t i = 0; i < newtree->GetEntries(); i++) {
		newtree->GetEntry(i);

		//Set Values to go into the branches
		aKMu->SetValBkg(GetAngle(kaon.getVec(), muon.getVec()));
		 aKe->SetValBkg(GetAngle(kaon.getVec(), electron.getVec()));
		aMue->SetValBkg(GetAngle(muon.getVec(), electron.getVec()));
		val1 = aKMu->GetBkgVal();
		val2 = aKe->GetBkgVal();
		val3 = aMue->GetBkgVal();

		//Fill Branches
		BaKMu->Fill();
		BaKe->Fill();
		BaMue->Fill();

		if (val1 > max) {
			max = val1;
	//		std::cout << "\r" << "Max aKMu Angle: " << max << std::flush;
		}
	//	if(i%100==0) std::cout << "\r" << "aKMu Angle: " << aKMu->GetBkgVal() << std::flush;
		std::cout << "\r" << "New Branches Progrss: " << std::setw(4) << i * 100 / (newtree->GetEntries() - 1) << " %." << std::flush;
	}
	std::cout << "\nMax aKMu Angle: " << max << std::endl;
//	if (CheckBranch(BaKMu, 0.0, max)) {
//		std::cout << "Exiting!\n";
//		return;
//	}
//	newtree->Write();
//	else DrawHist(BaKMu, 0.0, max);
//	DrawHist(BaKMu, 0, 1);
	//Get Clone of tree with cuts on derived variables
	newtree->SetBranchStatus("*",1);
//	newtree = MakeCuts(aMue, newtree, newfile, newtreename);					//Function from:   GeneralFunctions.C
//	std::cout << "Efficiency after Cuts: " << (Double_t)newtree->GetEntries() / ttree->GetEntries() << std::endl;

	tfile->Close();
	TFile* newfile = new TFile(newfilename, fileoption);
	newtree->Write();
	newfile->Close();
//	DeleteList(aMue);
//	if (CheckBranchFromFile()) {
//		std::cout << "Exiting!\n";
//		return;
//	}

	return;
}

void BothNewTrees() {
//	NewTree("RECREATE","BkgTree","/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithMVAResponse.root",
//					"/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithCuts.root","BkgTree");
//	return;
	NewTree("UPDATE","SigTree","/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithMVAResponse.root",
									"/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithCuts.root","SigTree");
	return;
}
void HistFromFileDouble(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
	TFile* tfile = TFile::Open(filename);
	TTree* ttree = (TTree*)tfile->Get(treename);
	ttree->SetBranchStatus("*", 0);
	ttree->SetBranchStatus(variable, 1);
	Double_t value;
	Double_t max = 0.0;
	Double_t min = 1000.0;
	Double_t sum = 0.0;
	Int_t count = 0;
	ttree->SetBranchAddress(variable, &value);
	TH1D* hist = new TH1D("hist", variable, 100, 0.0, 10000.0);
	Int_t error;
	for (Int_t i = 0; i < ttree->GetEntries(); i++) {
		error = ttree->GetEntry(i);
		if (error < 1) {
			std::cout << "Error getting entry\n";
			return;
		}
		if (value > max || value < min) {
			if (value > max) max = value;
			else			min = value;
			std::cout << "\r" << "Max Value = " << max << ", Min Value = " << min << std::flush;
		}
		if (value > 4) count++;
		hist->Fill(value);
	}
	std::cout << std::endl;
	std::cout << count << " values above 4\n";
	TCanvas* canv = new TCanvas("canv", variable, 400, 400);
	canv->cd();
	hist->Draw();
	return;
}

void HistFromFileFloat(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
	TFile* tfile = TFile::Open(filename);
	TTree* ttree = (TTree*)tfile->Get(treename);
	ttree->SetBranchStatus("*", 0);
	ttree->SetBranchStatus(variable, 1);
	Float_t value;
	Float_t max = 0.0;
	Float_t min = 1000.0;
	Float_t sum = 0.0;
	Int_t count = 0;
	ttree->SetBranchAddress(variable, &value);
	TH1D* hist = new TH1D("hist", variable, 100, 0.0, 1.0);
	Int_t error;
	for (Int_t i = 0; i < ttree->GetEntries(); i++) {
		error = ttree->GetEntry(i);
		if (error < 1) {
			std::cout << "Error getting entry\n";
			return;
		}
		if (value > max || value < min) {
			if(value > max) max = value;
			else			min = value;
			std::cout << "\r" << "Max Value = " << max << ", Min Value = " << min << std::flush;
		}
		if (value > 4) count++;
		hist->Fill(value);
	}
	std::cout << std::endl;
	std::cout << count << " values above 4\n";
	TCanvas* canv = new TCanvas("canv", variable, 400, 400);
	canv->cd();
	hist->Draw();
	return;
}
