#include "GeneralFunctions.C"
#include <iostream>
#include <fstream>
#include <stdlib.h>  
#include "Variable.h"
#include "particlefromtree.hpp"

void EnableBranches(TTree* tree, Variable* seed) {
	Variable *temp = seed;
	while (temp) {
		tree->SetBranchStatus(temp->GetName(), 1);
	}
	return;
}

Variable* CreateVarList(TTree* BkgTree, TTree* MCTree) {
	//Open two instances of the file to read the variables from
	std::ifstream file("/home/epp/phunch/Documents/VariableInOut/NamesAndCuts.txt");
	std::ifstream file2("/home/epp/phunch/Documents/VariableInOut/NamesAndCuts.txt");

	//Create two pointers to Variable objects, seed to point to the head of the 
	//   list and temp to point to each new Variable created.
	Variable *seed = new Variable;
	Variable *temp = NULL;
	TString  name_tmp;
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

		//Set the branch status and address for this variable
	//	BkgTree->SetBranchStatus(temp->GetName(), 1);
		BkgTree->SetBranchAddress(temp->GetName(), &temp->bkg_value);
	//	if (temp->HasMC()) MCTree->SetBranchStatus(temp->GetName(), 1);
		if (temp->HasMC()) MCTree->SetBranchAddress(temp->GetName(), &temp->mc_value);
	}

	file.close();
	file2.close();
	return(seed);
}



void ExcludeEvents() {

	//Get trees for Bkg and MC Data
	TTree* tBkg = GetTree(kTRUE);
	TTree* tMC  = GetTree(kFALSE);
	

	//Create Entry lists for MC and Bkg Data
//	TEventList* elBkg = new TEventList("BkgList", "List of Background entries to keep", tBkg);
//	TEventList* elMC  = new TEventList("MCList", "List of Monte Carlo entries to keep", tMC);
	TTree* tBkgOut = tBkg->CloneTree(0);

	//Set up derived variables
	Variable* aKMu = new Variable;
	aKMu->DerivedSetup("KMuAngle", 0.0005, 1, 0, tBkg);
	Variable* aKe = new Variable;
	aKMu->DerivedSetup("KeAngle", 0.0005, 1, 0, tBkg);
	aKe->SetNext(aKMu);
	Variable* aMue = new Variable;
	aKMu->DerivedSetup("MueAngle", 0.0005, 1, 0, tBkg);
	aMue->SetNext(aKe);

	//Disable all branches
	//tBkg->SetBranchStatus("*", 0);
	//tMC->SetBranchStatus("*", 0);

	//Re-enable branches of derived variables
	//EnableBranches(tBkgOut, aMue);
	
	//Create Linked list of variables which enables the relevant branches
	Variable* seed = CreateVarList(tBkg, tMC);
	if (seed == NULL) {
		std::cout << "Variable List is Empty\n";
		return;
	}

	//Add derived variables to the start of the list
	aKMu->SetNext(seed);
	seed = aMue;
	
	//Creates instances of variable that are derived from multiple branches in the trees.
	const bool istruth = false;
	Particle<Double_t> kaon("Kaon", tBkg, istruth);
	Particle<Double_t> muon("Mu", tBkg, istruth);
	Particle<Double_t> electron("E", tBkg, istruth);
	Particle<Double_t> kaonMC("Kaon", tMC, istruth);
	Particle<Double_t> muonMC("Mu", tMC, istruth);
	Particle<Double_t> electronMC("E", tMC, istruth);
	
	//Find out which dataset has more entries
	Long64_t big_ents;
	if (tBkg->GetEntries() > tMC->GetEntries()) big_ents = tBkg->GetEntries();
	else										big_ents = tMC->GetEntries();
	std::cout << "Number of BKG Entries: " << tBkg->GetEntries() << std::endl;
	std::cout << "Number of MC Entries:  " << tMC->GetEntries() << std::endl;

	//Create Variable pointer to move through the list
	Variable* current = seed;

	//Create Variables to tell whether to exclude the entry
	Int_t bkg_excl;
//	Int_t mc_excl;
	Int_t i;
	//Fill the Entry lists 
	for (i = 0; i < big_ents; i++) {
		bkg_excl = 0;
//		mc_excl = 0;
//		if(i % 10000 == 0) std::cout << "i = " << i << std::endl;

		//Go through derived variables calculating their values
		if (i < tBkg->GetEntries()) {
			aKMu->SetValBkg( ( kaon.getVec() ).Angle( ( muon.getVec()     ).Vect() ) );
			 aKe->SetValBkg( ( kaon.getVec() ).Angle( ( electron.getVec() ).Vect() ) );
			aMue->SetValBkg( ( muon.getVec() ).Angle( ( electron.getVec() ).Vect() ) );
		}
		
		//Go through the list of variables excluding events that don't pass the cut
		current = seed;
		while (current) {
			//Background
			if (i < tBkg->GetEntries()) {
	//			tBkg->GetEntry(i);
//				if (i % 10000 == 0) std::cout << "BKG VALUE: " << current->GetBkgVal() << std::endl;
				if (current->GetCutDir() != 0 && current->GetBkgVal() < current->GetCutVal()) {
					bkg_excl++;
				}
				else if (current->GetCutDir() == 0 && current->GetBkgVal() > current->GetCutVal()) {
					bkg_excl++;
				}
			}
			//Monte Carlo
	//		if (i < tMC->GetEntries() && current->HasMC()) {
	//			tMC->GetEntry(i);
//				if (i % 10000 == 0) std::cout << "MC VALUE:  " << current->GetMCVal() << std::endl;
	//			if (current->GetCutDir() != 0 && current->GetMCVal() < current->GetCutVal()) {
	//				mc_excl++;
	//			}
	//			else if (current->GetCutDir() == 0 && current->GetMCVal() > current->GetCutVal()) {
	//				mc_excl++;
	//			}
			}
			current = current->GetNext();
		}//End while loop over variables

		if (i < tBkg->GetEntries()) {
			if (bkg_excl == 0) {
	//			elBkg->Enter(i, tBkg);
				tBkgOut->Fill();
				if (current->IsDerived()) {
					current->GetBranch()->Fill();
				}
			}
		}
	//	else std::cout << "Background Entry:  " << i << " Excluded\n";
	//	if (i < tMC->GetEntries()) {
	//		if (mc_excl == 0)  elMC->Enter(i, tMC);
	//	}
	//	else std::cout << "Monte Carlo Entry: " << i << " Excluded\n";
	}//End of for loop over entries
	
	current = seed;
	while (current) {
		if (current->IsDerived()) {
			current->GetBranch()->Draw();
		}
	}

	TFile* newfile = new TFile("Test.root", "RECREATE");
	tBkgOut->Write();
	
//	std::cout << "BKG still has Entries:  " << elBkg->GetN() << std::endl;
//	std::cout << "MC still has Entries:   " << elMC->GetN() << std::endl;
//	std::cout << "Background Efficiency:  " << (Double_t)elBkg->GetN() / tBkg->GetEntries() << std::endl;
//	std::cout << "Monte Carlo Efficiency: " << (Double_t)elMC->GetN() / tMC->GetEntries() << std::endl;

//	tBkg->SetEntryList(elBkg);
//	std::cout << tBkg->GetEntries() << std::endl;
	return;
}

//Copy tree and add new branches with derived variables then save new tree to file. 
//then perform MVA on new tree with cuts on variables.
//