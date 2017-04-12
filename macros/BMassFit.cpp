#include "/home/epp/phunch/Documents/Useful/Variable.h"

void BMassFit() {
    //Get the TTrees from the root files for the background and monte carlo data
    TFile *fMC = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_Signal_MC_MD.root");
    TTreeReader mcReader("KMuE/DecayTree", fMC);
    TTree *DTmc = mcReader.GetTree();
    DTmc->SetBranchStatus("*",0);
    DTmc->SetBranchStatus("B_M",1);
    
    Variable BMass;
    BMass.m_name = "B_M";
    BMass.ConvertNames();
    BMass.m_min = 4200;
    BMass.m_max = 6350;
    BMass.GetBranch(DTmc,kFALSE);
    
    BMass.CreateHists();
    BMass.ResetHists();
     
    Int_t i;
    Int_t ent = DTmc->GetEntries();
    for(i=0; i<ent; i++) {
         DTmc->GetEntry(i);
         BMass.m_hist_MC->Fill(BMass.m_var_mc);
    }
    
     //Plot the B Mass with the cuts on all variables applied
    new TCanvas("MC", "Monte Carlo",800,600);
    BMass.m_hist_MC->Draw();
}
