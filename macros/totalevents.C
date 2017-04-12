#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include <iostream>

void totalevents() {
      TString path = "/storage/epp2/phsnab/BuKMuE/";
      TFile* file = TFile::Open(path+"BuKMuE_2011_MgUp.root");
      if(!file) {std::cout << "Couldn't open: " << "BuKMuE_2011_MgUp.root\n"; return;}
      TTree* tree = (TTree*)file->Get("KMuE/DecayTree");
      if(!tree) {std::cout << "Couldn't open tree: " << "BuKMuE_2011_MgUp.root\n"; return;}
      Int_t ents = 0;
      ents += tree->GetEntries();
      file->Close();

      file = TFile::Open(path+"BuKMuE_2011_MgDn.root");
      if(!file) {std::cout << "Couldn't open: " << "BuKMuE_2011_MgDn.root\n"; return;}
      tree = (TTree*)file->Get("KMuE/DecayTree");
      if(!tree) {std::cout << "Couldn't open tree: " << "BuKMuE_2011_MgDn.root\n"; return;}
      ents += tree->GetEntries();
      file->Close();

      file = TFile::Open(path+"BuKMuE_2012_MgDn.root");
      if(!file) {std::cout << "Couldn't open: " << "BuKMuE_2012_MgDn.root\n"; return;}
      tree = (TTree*)file->Get("KMuE/DecayTree");
      if(!tree) {std::cout << "Couldn't open tree: " << "BuKMuE_2012_MgDn.root\n"; return;}
      ents += tree->GetEntries();
      file->Close();

      file = TFile::Open(path+"BuKMuE_2012_MgUp.root");
      if(!file) {std::cout << "Couldn't open: " << "BuKMuE_2012_MgUp.root\n"; return;}
      tree = (TTree*)file->Get("KMuE/DecayTree");
      if(!tree) {std::cout << "Couldn't open tree: " << "BuKMuE_2012_MgUp.root\n"; return;}
      ents += tree->GetEntries();
      file->Close();

      std::cout << "Entries: " << ents << std::endl;
      return;
}
