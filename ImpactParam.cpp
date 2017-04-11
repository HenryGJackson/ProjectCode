//==========================================//
//A ROOT macro used to get data from a .root//
//    file and then calculate the Impact    //
//                Parameter                 //
//==========================================//
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include "TBranch.h"
#include "TTreeReader.h"
#include <TTreeReaderArray.h>

Double_t Mag(Double_t x, Double_t y) {
    return sqrt(x*x+y*y);
}

Double_t IP(Double_t Bpz, Double_t Btau, Double_t Kpz, Double_t Kpt) {
    Double_t mB = 5.3;
    Int_t c = 3e8;
    Double_t theta = atan2(Kpt,Kpz);
    Double_t L = (Bpz/mB)*c*Btau*1e-9;

    return L*sin(theta);
}

void ImpactParam()
{

    Int_t decision;
    Char_t cha;;
    std::cout << "Show Progress Check? (y=0/n=1-9)  ";
    std::cin >> decision;
    if (decision == 0) std::cout << "Beginning the Macro" << std::endl;


    Int_t i;
    //B Histograms
    TH1D *BPZmcHist = new TH1D("BPZ_MC", "Histogram of B PZ from Monte Carlo", 100, 0,500000);
    BPZmcHist->GetXaxis()->SetTitle("Momentum/ GeV");
    BPZmcHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BPZsigHist = new TH1D("BPZ_Signal", "Histogram of B PZ from Signal", 100, 0,500000);
    BPZsigHist->GetXaxis()->SetTitle("Momentum/ GeV");
    BPZsigHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BtauMCHist = new TH1D("Btau_MC", "Histogram of B Decay Time from Monte Carlo", 100, 0,0.01);
    BtauMCHist->GetXaxis()->SetTitle("Decay Time /s");
    BtauMCHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BtauSigHist = new TH1D("Btau_Signal", "Histogram of B Decay Time from Signal", 100, 0,0.01);
    BtauSigHist->GetXaxis()->SetTitle("Decay Time /s");
    BtauSigHist->GetYaxis()->SetTitle("Number of Entries");

    //Kaon Histograms
    TH1D *KPZmcHist = new TH1D("KPZ_MC", "Histogram of Kaon PZ from Monte Carlo", 100, 0,150000);
    KPZmcHist->GetXaxis()->SetTitle("Momentum/ GeV");
    KPZmcHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *KPZsigHist = new TH1D("KPZ_Signal", "Histogram of Kaon PZ from Signal", 100, 0,150000);
    KPZsigHist->GetXaxis()->SetTitle("Momentum/ GeV");
    KPZsigHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *KPTmcHist = new TH1D("KPT_MC", "Histogram of Kaon PT from Monte Carlo", 100, 0,20000);
    KPZmcHist->GetXaxis()->SetTitle("Momentum/ GeV");
    KPZmcHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *KPTsigHist = new TH1D("KPT_Signal", "Histogram of Kaon PT from Signal", 100, 0,20000);
    KPZsigHist->GetXaxis()->SetTitle("Momentum/ GeV");
    KPZsigHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *KIPmcHist = new TH1D("KIP_MC", "Histogram of Kaon Impact Parameter from Monte Carlo", 100, 0,2);
    KIPmcHist->GetXaxis()->SetTitle("Impact Parameter");
    KIPmcHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *KIPSigHist = new TH1D("KIP_Signal", "Histogram of Kaon Impact Parameter from Signal", 100, 0,2);
    KIPSigHist->GetXaxis()->SetTitle("Impact Parameter");
    KIPSigHist->GetYaxis()->SetTitle("Number of Entries");
    if (decision == 0) std::cout << "Histograms Created " << std::endl;


  //Get the tree from the root file for the monte carlo
    TFile *fMC = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_Signal_MC_MD.root");
    TTreeReader mcReader("KMuE/DecayTree", fMC);
    TTree *DTmc = mcReader.GetTree();
    if (decision == 0) std::cout << "Monte Carlo Tree Got " << std::endl;

  //Get the tree from the root file for the signal
    TFile *fS = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root");
    TTreeReader sigReader("KMuE/DecayTree",fS);
    TTree *DTsig = sigReader.GetTree();
    if (decision == 0) std::cout << "Signal Tree Got " << std::endl;

  //Create Branch objects and store the necessary branches within them
    TBranch *KPZmc = DTmc->GetBranch("Kaon_PZ");
    TBranch *KPZsig = DTsig->GetBranch("Kaon_PZ");

    Double_t BPZsigVal, BPZmcVal;
    Double_t BtauSigVal, BtauMCval;
    Double_t KPZsigVal, KPZmcVal;
    Double_t KPXsigVal, KPXmcVal;
    Double_t KPYsigVal, KPYmcVal;
    Double_t KPTsigVal, KPTmcVal;

    DTsig->SetBranchAddress("B_PZ", &BPZsigVal);
    DTmc->SetBranchAddress("B_PZ", &BPZmcVal);
    DTsig->SetBranchAddress("B_TAU", &BtauSigVal);
    DTmc->SetBranchAddress("B_TAU", &BtauMCval);
    DTsig->SetBranchAddress("Kaon_PZ", &KPZsigVal);
    DTmc->SetBranchAddress("Kaon_PZ", &KPZmcVal);
    DTsig->SetBranchAddress("Kaon_PX", &KPXsigVal);
    DTmc->SetBranchAddress("Kaon_PX", &KPXmcVal);
    DTsig->SetBranchAddress("Kaon_PY", &KPYsigVal);
    DTmc->SetBranchAddress("Kaon_PY", &KPYmcVal);
    if (decision == 0) std::cout << "Branches Created " << std::endl;

    Int_t entries = 100000;
    if ( KPZmc->GetEntries() < KPZsig->GetEntries() ) entries = KPZmc->GetEntries();
    else entries = KPZsig->GetEntries();
    if (entries > 100000) entries = 100000;

    std::cout << "Entering loop of " << entries << " Entries." << std::endl;
    for(i=0; i<entries; i++) {
	DTmc->GetEntry(i);
        DTsig->GetEntry(i);
	
	KPTmcVal = Mag(KPXmcVal,KPYmcVal);
	KPTsigVal = Mag(KPXsigVal,KPYsigVal);
        
        BPZmcHist->Fill(BPZmcVal);
        BPZsigHist->Fill(BPZsigVal);
        BtauMCHist->Fill(BtauMCval);
        BtauSigHist->Fill(BtauSigVal);
        KPZmcHist->Fill(KPZmcVal);
        KPZsigHist->Fill(KPZsigVal);
        KPTmcHist->Fill(KPTmcVal);
        KPTsigHist->Fill(KPTsigVal);
	
	KIPmcHist->Fill(IP(BPZmcVal, BtauMCval, KPZmcVal, KPTmcVal));
	KIPSigHist->Fill(IP(BPZsigVal, BtauSigVal, KPZsigVal, KPTsigVal));
        //get the impact parameter from the function IP (requires calculation of more variables)


     //   std::cout << "Event " << i << " has value: " << KPZsigVal << std::endl;
    }

    if (decision == 0) std::cout << "Histograms filled " << std::endl;

    TFile File("ImpPar.root","RECREATE");
    if (decision == 0) std::cout << "Root file created " << std::endl;

    BPZmcHist->Write();
    BPZsigHist->Write();
    BtauMCHist->Write();
    BtauSigHist->Write();
    KPZmcHist->Write();
    KPZsigHist->Write();
    KPTmcHist->Write();
    KPTsigHist->Write();
    KIPmcHist->Write();
    KIPSigHist->Write();
    if (decision == 0) std::cout << "Histograms Written to File 'ImpPar.root' " << std::endl;

    TBrowser *b = new TBrowser();

    File.Close();
    fMC->Close();
    fS->Close();
    delete KPZmcHist;
    delete KPZsigHist;
    delete BPZmcHist;
    delete BPZsigHist;
    delete BtauMCHist;
    delete BtauSigHist;
    delete KPTmcHist;
    delete KPTsigHist;
    delete KIPmcHist;
    delete KIPSigHist;
    
    if (decision == 0) std::cout << "Files closed and objects deleted\nENDING. " << std::endl;
}
