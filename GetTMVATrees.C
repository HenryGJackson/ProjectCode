#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
//#include "NewTree.C"
//#include "TMVAClassificationApplication.C"

Int_t GetBigger(Int_t a, Int_t b) {
  if(a < b) return b;
  else      return a;
}

void PrintTime() {
  clock_t start = clock();
  clock_t diff = clock() - start;
  Int_t msec = diff / 1000;
  std::cout << "\nTime: " << msec << std::endl;
}

void TMVACut(TTree* bNewTree, TString MVAmethod = "BDT", TString wfilename = "gd.ty"){
    std::cout << "\r" << "Progress Check: " << "Beginning                                           " << std::flush;

//    TString wfilename  = "/home/henry/Documents/Project/rootcode/TMVA/TMVAWeights.root";  //For My PC
//    TString wfilename  = "/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWeights"+MVAmethod+".root";  //For CSC Machines
    TString bwtreename = "TMVAWeights_"+MVAmethod;
    // TString swtreename = "TMVAWeights_"+MVAmethod+"_MC";

    Int_t bEnts       = bNewTree->GetEntries();
//    Int_t sigEnts     = sigNewTree->GetEntries();

    std::cout << "\r" << "Progress Check: " << "Opening Files and Getting Trees                     " << std::flush;

     TFile* wFile = TFile::Open( wfilename );
     if(wFile==NULL) {std::cout << "Couldn't Open Weights File\n"; return;}
     TTree* bwTree = (TTree*)wFile->Get( bwtreename );
     if(bwTree==NULL) {std::cout << "Couldn't Load Background Weights Tree\n"; return;}
//     TTree* swTree = (TTree*)wFile->Get( swtreename );
//     if(swTree==NULL) {std::cout << "Couldn't Load Signal Weights Tree\n"; return;}

     std::cout << "\r" << "Progress Check: " << "Setting Branch Addresses                           " << std::flush;
     Double_t bWeight;// sigWeight;
     bwTree->SetBranchAddress("MVA_"+MVAmethod, &bWeight);
//     swTree->SetBranchAddress("MVA_"+MVAmethod, &sigWeight);



     std::cout << "\r" << "Progress Check: " << "Creating New Branches                              " << std::flush;
     Float_t bw;// sw;
     TString bBraName = MVAmethod + "_weights";
//     TString sigBraName = MVAmethod + "_weights";
     TBranch* wbNewBranch = bNewTree->Branch(bBraName, &bw, bBraName);
//     TBranch* wsNewBranch = sigNewTree->Branch(sigBraName, &sw, sigBraName);

     Int_t i;
     Int_t bigEnts = bEnts; //GetBigger(bEnts,sigEnts);
     for(i = 0; i < bigEnts; i++) {
            if(i < bEnts) {
                  bwTree->GetEntry(i);
                  bw = bWeight;
                  wbNewBranch->Fill();
            }
            // if(i < sigEnts) {
            //       swTree->GetEntry(i);
            //       std::cout << sigWeight;
            //       sw = sigWeight;
            //       wsNewBranch->Fill();
            // }
            std::cout << "\r" << "New Branches Progrss: " << i * 100 / (bigEnts - 1) << " %.                                 " << std::flush;
     }

     wFile->Close();



     return;
}

void GetTMVATrees( TString bfilename = "/home/henry/Project/rootcode/TMVA/rootoutput/TMVA.root", TString btreename = "BkgTree", //For My PC
              //    TString sfilename = "/home/henry/Documents/Project/BuKMuE_Signal_MC_MD.root", TString streename = "KMuE/DecayTree" ){//For My PC
  //                TString bfilename = "BuKMuE_2011_MgUp.root", TString btreename = "KMuE/DecayTree", //For My Area on CSC Machines
  //                TString sfilename = "/home/henry/Project/rootcode/TMVA/rootoutput/TMVAout_WithPT.root", TString streename = "SigTree",
                TString wfilename = "bfe.root", TString nfilename  = "/home/henry/Project/rootcode/TMVA/rootoutput/DataWithPT_MVAResponse.root"){//For My Area on CSC Machines


      TFile* bFile = TFile::Open( bfilename );
      if(bFile==NULL) {std::cout << "Couldn't Open Background File\n"; return;}
      TTree* bTree = (TTree*)bFile->Get( btreename );
      if(bTree==NULL) {std::cout << "Couldn't Load Background Tree\n"; return;}
//      TFile* sigFile = TFile::Open( sfilename );
  //    if(sigFile==NULL) {std::cout << "Couldn't Open Signal File\n"; return;}
//      TTree* sigTree = (TTree*)sigFile->Get( streename );
//      if(sigTree==NULL) {std::cout << "Couldn't Load Signal Tree\n"; return;}

      TString fileoption = "RECREATE";

      clock_t start = clock(), diff;
      Int_t bEnts       = bTree->GetEntries();
//      Int_t sigEnts     = sigTree->GetEntries();
      TFile* nFile      = new TFile(nfilename, fileoption);
      std::cout << "\r" << "Progress Check: " << "Cloning Background Tree                            " << std::flush;
      TTree* bNewTree   = bTree->CloneTree( bEnts ,"fast");
      std::cout << "\r" << "Progress Check: " << "Cloning Signal Tree                                " << std::flush;
//      TTree* sigNewTree = sigTree->CloneTree( sigEnts );
//      std::cout << "\r" << "Progress Check: " << "Closing Files                                      " << std::flush;
      bFile->Close();
//      sigFile->Close();
      diff = clock() - start;
      Int_t msec = diff;
      std::cout << "\nTime: " << msec/1000 << std::endl;

//      TMVACut(bNewTree,"MLP");
      TMVACut(bNewTree,"BDT",wfilename);
//      TMVACut(bNewTree,sigNewTree,"BDTG");

      std::cout << "\r" << "Progress Check: " << "Writing Background Tree to: " << nfilename << "  " << std::flush;
      nFile->cd();
//      bNewTree->SetName("BkgTree");
      bNewTree->Write();
      std::cout << "\r" << "Progress Check: " << "Writing Signal Tree to: " << nfilename << ""   << std::flush;
//      sigNewTree->SetName("SigTree");
//      sigNewTree->Write();
      nFile->Close();
      std::cout << "\r" << "Written Trees to: " << nfilename << "                           " << std::flush;
      std::cout << std::endl;


      return;

}

//void AllCuts() {
//    GetTMVATrees("/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","BkgTree",
  //                "/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","SigTree",
    //              "/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWeightsBDT.root");
  //  NewTree("RECREATE","BkgTree","TreesWithMVAResponse.root");
  //  NewTree("UPDATE","SigTree","TreesWithMVAResponse.root","NewTreesWithCuts.root","SigTree");
  //  return;
//}
