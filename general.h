#ifndef GENERALFUNCTIONS_H
#define GENERALFUNCTIONS_H

//NOTES FOR DAN!!!!!!!
//You'll need to change the path of the included files to wherever you store them
//There is an included file at the bottom of each .h file (this isn't proper practice but it works in ROOT)
//NOTES FOR DAN!!!!!!!

#include <string>
#include <stdlib.h>

#include "TFile.h"
#include "TTree.h"
#include "TEntryList.h"
#include "TH1D.h"
#include "TString.h"
#include "TLorentzVector.h"
#include "TBranch.h"
#include "TTreeReader.h"
#include "TChain.h"

//Libs found on phunch storage area "/storage/epp2/phunch/incl/"
//Can use the above as the path^
// #include "/home/henry/Project/TMVA/incl/Variable.h"
// #include "/home/henry/Project/TMVA/incl/particlefromtree.hpp"
#include "/storage/epp2/phunch/incl/Variable.h"
#include "/storage/epp2/phunch/incl/particlefromtree.hpp"
//Namespaces defined in this file: cut
//                                 general
//					     angle
namespace cut{
	//Copy subset of tree with selection "cutstr"
	TString copySubset(const TString cutstr = "B_PT > 10000", const TString treename = "KMuE/DecayTree",
				    const TString path = "", const TString filename = "BuKMuE_2011_MgUp.root", TString nfname = "");
	//Perform a cut on a double value.
	void CutDoubleVar(TString varname = "B_M", TString qualifier = ">", Double_t value = 5500.0,
		  	TString outfilename = "~/Project/TMVA/rootoutput/withcuts/AllDataCombined_cut",
			TString filename = "~/Project/TMVA/rootoutput/withcuts/AllDataCombined_afterCuts.root",
			TString treename = "DecayTree");
	//Perform a cut on a boolean value.
	void CutBoolVar(TString varname = "E_HasBremAdded", TString value = kFALSE, TString treename = "DecayTree",
			TString ifname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllDataCombined_afterCuts.root",
	            TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllData_HasBremTRUE.root");
	//Perform a cut on an integer value. (Requires modification to choose the values to cut)
	void CutIntVar(TString varname = "E_BremMultiplicity", TString tn = "DecayTree",
		      TString fn = "AllDataCombined_afterCuts",
			TString path = "/home/henry/Project/TMVA/rootoutput/withcuts/");
	//Make cuts on variables using the class from Variable.h.
	//Requires that Variable::var_name, Variable::cut_less_than, Variable::cut_val are all initialised.
	//Passing a linked list of instances of Variable will perform cuts on all these variables.
	TTree* MakeCuts(Variable* seed, TTree* ttree, TFile *newfile, TString newtreename = "DecayTree");
	//Set up a linked list of variables with their cuts set to be used as input to MakeCuts();
	Variable* BookCuts(TString vars = "B_M,B_PT", TString qualifiers = ">,>",
	            TString values = "5700.0,100.0", TString types = "double,double");
}

namespace general{
	//Return the SigTree or BkgTree from original datafiles (probably won't work)
	TTree* GetTree(Bool_t GetBkg);
	//Return the entrylist for SigTree or BkgTree from original datafiles (probably won't work)
	TEntryList* GetEList(TTree* tree, Bool_t GetBkg);
	//Create a linked list of Variable instances, taking values from the file "namesandcuts.txt"
	Variable* CreateVarList(TTree* BkgTree);
	//Delete the linked list of Variable instances that starts with seed.
	void DeleteList(Variable* seed);
	void TestDeleteList();
	//Draw Histogram using data from TBranch* branch. has boundaries min -> max.
	void DrawHist(TBranch* branch, Int_t min, Int_t max);
	//Draw the B mass in the range specified
	void DrawBMass(Double_t min = 4400, Double_t max = 6200, const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree");
	//Check the values in the branch are greater than min and less than max.
	Int_t CheckBranch(TBranch *branch, Double_t min, Double_t max);
	Int_t CheckBranchFromFile(const TString branch = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") ;
	//Makes cuts using cut::MakeCuts(). Adds derived variables and then cuts on them.
	void NewTree(const TString fileoption = "UPDATE", const TString treename = "KMuE/DecayTree", const TString filename = "/home/henry/Documents/Project/BuKMuE_Signal_MC_MD.root",
						   const TString newfilename = "NewTreesWithCuts.root", const TString newtreename = "BkgTree");
	//Calls NewTree() twice, once for the BKG tree and once for the SIG tree.
	void BothNewTrees();
	//Create histogram for the variable passed in
	void HistFromFileDouble(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree");
	void HistFromFileFloat(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree");
	//Sets Branch Status of Variables in the linked list that follows seed.
	void EnableBranches(TTree* tree, Variable* seed);
	//Combine trees into one.
	Int_t combinedata(TString fnames = "BLAH,BLAH", TString treename = "DecayTree",
	            TString newname = "BLAH.root", TString path = "~/Project/TMVA/rootoutput/withcuts/");
	//Broken
	// void ExcludeEvents();
}

namespace angle{
	Double_t GetMax(Double_t K, Double_t Mu, Double_t e);
	Double_t GetMin(Double_t K, Double_t Mu, Double_t e);
	Double_t GetAngle(TLorentzVector V1, TLorentzVector V2);
}

namespace load{
	TTree* tree(TString filename, TString treename);
}

#endif
