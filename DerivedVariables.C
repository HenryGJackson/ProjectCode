#include <stdlib.h>
#include <iostream>

#include "TLorentzVector.h"
#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

#include "particlefromtree.hpp"

Double_t GetMax(Double_t K, Double_t Mu, Double_t e) {
    Double_t max;
    if(K > Mu)  max = K;
    else        max = Mu;
    if(e > max) max = e;
    return max;
}

Double_t GetMin(Double_t K, Double_t Mu, Double_t e) {
    Double_t min;
    if(K < Mu)  min = K;
    else        min = Mu;
    if(e < min) min = e;
    return min;
}

Double_t GetAngle(TLorentzVector V1, TLorentzVector V2) {
	return(V1.Angle(V2.Vect()) );
}

void GetDerivedVariables(TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithPTandAngles.root",
           TString bfname = "/home/henry/Project/BuKMuE_2011_MgUp.root",
           TString btname = "KMuE/DecayTree"
//           TString sfname = "/home/henry/Project/BuKMuE_Signal_MC_MD.root",
//           TString stname = "KMuE/DecayTree"
) {

    //Open Files to read background and signal and get respective trees
    TFile* bfile = TFile::Open(bfname);
    TTree* btree = (TTree*)bfile->Get(btname);
//    TFile* sfile = TFile::Open(sfname);
//    TTree* stree = (TTree*)sfile->Get(stname);

    //Create output file and clone trees
    Int_t entBkg = btree->GetEntries();
//    Int_t entSig = stree->GetEntries();
    TFile* ofile = new TFile(ofname,"RECREATE");
    std::cout << "\rCloning Background Tree                     " << std::flush;
    TTree* bntree = btree->CloneTree(entBkg,"fast");
//    std::cout << "\rCloning Signal Tree                         " << std::flush;
//    TTree* sntree = stree->CloneTree(entSig);
    btree->SetBranchStatus("*",0);
    const bool istruth = false;
    Particle<Double_t> kaon("Kaon", btree, istruth);			//Class from:   particlefromtree.cpp
    Particle<Double_t> muon("Mu", btree, istruth);  			//Class from:   particlefromtree.cpp
    Particle<Double_t> electron("E", btree, istruth);			//Class from:   particlefromtree.cpp

    //Set up branches for reading data.
    //Must be double or error returned:
    Double_t KBkgPT, MuBkgPT, eBkgPT; //KSigPT, MuSigPT, eSigPT;
    btree->SetBranchStatus("Kaon_PT",1); btree->SetBranchAddress("Kaon_PT",&KBkgPT);
    btree->SetBranchStatus("Mu_PT",  1); btree->SetBranchAddress("Mu_PT",  &MuBkgPT);
    btree->SetBranchStatus("E_PT",   1); btree->SetBranchAddress("E_PT",   &eBkgPT);
    Double_t KIPChi2, MuIPChi2, eIPChi2;
    btree->SetBranchStatus("Kaon_IPCHI2_OWNPV", 1); btree->SetBranchAddress("Kaon_IPCHI2_OWNPV",  &KIPChi2);
    btree->SetBranchStatus("Mu_IPCHI2_OWNPV",   1); btree->SetBranchAddress("Mu_IPCHI2_OWNPV",   &MuIPChi2);
    btree->SetBranchStatus("E_IPCHI2_OWNPV",   1);  btree->SetBranchAddress("E_IPCHI2_OWNPV",   &eIPChi2);

//    stree->SetBranchStatus("*",0);
//    stree->SetBranchStatus("Kaon_PT",1); stree->SetBranchAddress("Kaon_PT",&KSigPT);
//    stree->SetBranchStatus("Mu_PT",  1); stree->SetBranchAddress("Mu_PT",  &MuSigPT);
//    stree->SetBranchStatus("E_PT",   1); stree->SetBranchAddress("E_PT",   &eSigPT);

    //Set up new branches to save data to.
    //NOTE Only works if these are float...???-> WHY???
    Double_t BkgMaxPT, BkgMinPT; //SigMaxPT, SigMinPT;
    Double_t ang1, ang2, ang3; //, ang4, ang5, ang6;
    Double_t MaxIPChi2, MinIPChi2;
    bntree->SetBranchStatus("*",0);
//    sntree->SetBranchStatus("*",0);
    TBranch* BbmaxPT = bntree->Branch("MaxPT", &BkgMaxPT, "MaxPT/D"); bntree->SetBranchStatus("MaxPT",1);
    TBranch* BbminPT = bntree->Branch("MinPT", &BkgMinPT, "MinPT/D"); bntree->SetBranchStatus("MinPT",1);
    TBranch* BaKMu   = bntree->Branch("aKMu",  &ang1, "aKMu/D");      bntree->SetBranchStatus("aKMu",1);
    TBranch* BaKe    = bntree->Branch("aKe",   &ang2, "aKe/D");       bntree->SetBranchStatus("aKe",1);
    TBranch* BaMue   = bntree->Branch("aMue",  &ang3, "aMue/D");      bntree->SetBranchStatus("aMue",1);
    TBranch* BmaxIPChi2 = bntree->Branch("MaxIPChi2", &MaxIPChi2, "MaxIPChi2/D"); bntree->SetBranchStatus("MaxIPChi2",1);
    TBranch* BminIPChi2 = bntree->Branch("MinIPChi2", &MinIPChi2, "MinIPChi2/D"); bntree->SetBranchStatus("MinIPChi2",1);

//    TBranch* BsmaxPT = sntree->Branch("MaxPT", &SigMaxPT, "MaxPT");
//    TBranch* BsminPT = sntree->Branch("MinPT", &SigMinPT, "MinPT");
//    TBranch* SaKMu   = sntree->Branch("aKMu",  &ang4,     "aKMu");
//    TBranch* SaKe    = sntree->Branch("aKe",   &ang5,     "aKe");
//    TBranch* SaMue   = sntree->Branch("aMue",  &ang6,     "aMue");

    //Create Particle objects, used to calculate angles

//    Particle<Double_t> kaonMC("Kaon", stree, istruth);			//Class from:   particlefromtree.cpp
//  	Particle<Double_t> muonMC("Mu", stree, istruth);  			//Class from:   particlefromtree.cpp
//  	Particle<Double_t> electronMC("E", stree, istruth);			//Class from:   particlefromtree.cpp

    Int_t ent= entBkg;
  //  if(entBkg > entSig) ent = entBkg;
  //  else                ent = entSig;
    Int_t i;
    //Fill new branches with data
    for(i = 0; i < ent; i++) {
        if(i < entBkg) {
            btree->GetEntry(i);
            std::cout << " " << KBkgPT << " ";
            BkgMaxPT = GetMax(KBkgPT,MuBkgPT,eBkgPT);
            BkgMinPT = GetMin(KBkgPT,MuBkgPT,eBkgPT);
            ang1 = GetAngle(kaon.getVec(), muon.getVec());
            ang2 = GetAngle(kaon.getVec(), electron.getVec());
            ang3 = GetAngle(muon.getVec(), electron.getVec());
            MaxIPChi2 = GetMax(KIPChi2, MuIPChi2, eIPChi2);
            MinIPChi2 = GetMin(KIPChi2, MuIPChi2, eIPChi2);
            std::cout << BkgMinPT << " ";
            BbmaxPT->Fill();
            BbminPT->Fill();
            BaKMu->Fill();
            BaKe->Fill();
            BaMue->Fill();
            BmaxIPChi2->Fill();
            BminIPChi2->Fill();
        }
        // if(i < entSig) {
        //     stree->GetEntry(i);
        //     SigMaxPT = (Double_t)GetMax(KSigPT,MuSigPT,eSigPT);
        //     SigMinPT = (Double_t)GetMin(KSigPT,MuSigPT,eSigPT);
        //     BsmaxPT->Fill();
        //     BsminPT->Fill();
        //     ang4 = (Double_t)GetAngle(kaonMC.getVec(), muonMC.getVec());
        // 		ang5 = (Double_t)GetAngle(kaonMC.getVec(), electronMC.getVec());
        // 		ang6 = (Double_t)GetAngle(muonMC.getVec(), electronMC.getVec());
        //     SaKMu->Fill();
        //     SaKe->Fill();
        //     SaMue->Fill();
        // }

        std::cout << "\rNew Branches Progrss: " << std::setw(4) << i * 100 / (ent - 1) << " %." << std::flush;
    }

    //Close files and save new trees to output file.

//    sfile->Close();
    bntree->SetBranchStatus("*",1);
//    sntree->SetBranchStatus("*",1);
//    bntree->SetName("BkgTree");
//    sntree->SetName("SigTree");
    bntree->Write();
//    sntree->Write();
    ofile->Close();
    bfile->Close();

    std::cout << "\rDONE                                        " << std::endl;
    return;
}
