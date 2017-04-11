#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TMVAClassificationApplication.C"

Int_t GetBigger(Int_t a, Int_t b) {
  if(a < b) return b;
  else      return a;
}

void TMVACut(TTree* bNewTree, TTree* sigNewTree, TString MVAmethod = "BDT"){
    std::cout << "\r" << "Progress Check: " << "Beginning                                           " << std::flush;

//    TString wfilename  = "/home/henry/Documents/Project/rootcode/TMVA/TMVAWeights.root";  //For My PC
    TString wfilename  = "TMVAWeights.root";  //For CSC Machines
    TString bwtreename = "TMVAWeightsBkg";
    TString swtreename = "TMVAWeightsMC";

    Int_t bEnts       = bNewTree->GetEntries();
    Int_t sigEnts     = sigNewTree->GetEntries();

    std::cout << "\r" << "Progress Check: " << "Opening Files and Getting Trees                     " << std::flush;

     TFile* wFile = TFile::Open( wfilename );
     if(wFile==NULL) {std::cout << "Couldn't Open Weights File\n"; return;}
     TTree* bwTree = (TTree*)wFile->Get( bwtreename );
     if(bwTree==NULL) {std::cout << "Couldn't Load Background Weights Tree\n"; return;}
     TTree* swTree = (TTree*)wFile->Get( swtreename );
     if(swTree==NULL) {std::cout << "Couldn't Load Signal Weights Tree\n"; return;}

     std::cout << "\r" << "Progress Check: " << "Setting Branch Addresses                           " << std::flush;
     Double_t bWeight, sigWeight;
     bwTree->SetBranchAddress("MVA", &bWeight);
     swTree->SetBranchAddress("MVA", &sigWeight);



     std::cout << "\r" << "Progress Check: " << "Creating New Branches                              " << std::flush;
     Float_t sw, bw;
     TString bBraName = MVAmethod + "_weightsBkg";
     TString sigBraName = MVAmethod + "_weightsSig";
     TBranch* wbNewBranch = bNewTree->Branch(bBraName, &bw, bBraName);
     TBranch* wsNewBranch = sigNewTree->Branch(sigBraName, &sw, sigBraName);

     Int_t i;
     Int_t bigEnts = GetBigger(bEnts,sigEnts);
     for(i = 0; i < bigEnts; i++) {
            if(i < bEnts) {
                  bwTree->GetEntry(i);
                  bw = bWeight;
                  wbNewBranch->Fill();
            }
            if(i < sigEnts) {
                  swTree->GetEntry(i);
                  std::cout << sigWeight;
                  sw = sigWeight;
                  wsNewBranch->Fill();
            }
            std::cout << "\r" << "New Branches Progrss: " << std::setw(4) << i * 100 / (bigEnts - 1) << " %.                                 " << std::flush;
     }

     wFile->Close();



     return;
}

void GetTMVATrees( //TString bfilename = "/home/henry/Documents/Project/BuKMuE_2011_MgUp.root", TString btreename = "KMuE/DecayTree", //For My PC
                  //TString sfilename = "/home/henry/Documents/Project/BuKMuE_Signal_MC_MD.root", TString streename = "KMuE/DecayTree" ){//For My PC
                  TString bfilename = "BuKMuE_2011_MgUp.root", TString btreename = "KMuE/DecayTree", //For My Area on CSC Machines
                  TString sfilename = "BuKMuE_Signal_MC_MD.root", TString streename = "KMuE/DecayTree" ){//For My Area on CSC Machines
                    TString nfilename  = "TreesWithMVAResponse.root";

      TFile* bFile = TFile::Open( bfilename );
      if(bFile==NULL) {std::cout << "Couldn't Open Background File\n"; return;}
      TTree* bTree = (TTree*)bFile->Get( btreename );
      if(bTree==NULL) {std::cout << "Couldn't Load Background Tree\n"; return;}
      TFile* sigFile = TFile::Open( sfilename );
      if(sigFile==NULL) {std::cout << "Couldn't Open Signal File\n"; return;}
      TTree* sigTree = (TTree*)sigFile->Get( streename );
      if(sigTree==NULL) {std::cout << "Couldn't Load Signal Tree\n"; return;}

      TString fileoption = "RECREATE";

      Int_t bEnts       = bTree->GetEntries();
      Int_t sigEnts     = sigTree->GetEntries();
      TFile* nFile      = new TFile(nfilename, fileoption);
      std::cout << "\r" << "Progress Check: " << "Cloning Background Tree                            " << std::flush;
      TTree* bNewTree   = bTree->CloneTree( bEnts );
      std::cout << "\r" << "Progress Check: " << "Cloning Signal Tree                                " << std::flush;
      TTree* sigNewTree = sigTree->CloneTree( sigEnts );
      std::cout << "\r" << "Progress Check: " << "Closing Files                                      " << std::flush;
      bFile->Close();
      sigFile->Close();


      string method;
      string method1 = "MLP";
      string method2 = "BDT";
      string method3 = "BDTG";
      vector<string> methods{ method1, method2, method3 };


      Int_t i;
      for(i = 0; i < 3; i++) {
          std::cout << " --- Method: " << method[i] << " " << i+1 << "/3\n";

          if(i==0) TMVAClassificationApplication(bfilename,btreename,"TMVAweightsBkg","RECREATE",method[i]);
          else     TMVAClassificationApplication(bfilename,btreename,"TMVAweightsBkg","UPDATE",method[i]);
          TMVAClassificationApplication(sfilename,streename,"TMVAweightsMC","UPDATE",method[i]);
          if(i==0) TMVACut(bNewTree,sigNewTree,method[i]);
          else     TMVACut(bNewTree,sigNewTree,method[i]);
      }

      std::cout << "\r" << "Progress Check: " << "Writing Background Tree to: " << nfilename << "  " << std::flush;
      bNewTree->SetName("BkgTree");
      bNewTree->Write();
      std::cout << "\r" << "Progress Check: " << "Writing Signal Tree to: " << nfilename << ""   << std::flush;
      sigNewTree->SetName("SigTree");
      sigNewTree->Write();
      nFile->Close();
      std::cout << "\r" << "Written Trees to: " << nfilename << "                           " << std::flush;
      std::cout << std::endl;


      return;

}
