//===========================================//
//A ROOT macro used to analyse the background//
//of the reconstruction of the mass of a B   //
//Meson           			     //
//===========================================//
#include "TFile.h"
#include "TTree.h"
#include <iostream>
#include "TBranch.h"
#include "TTreeReader.h"
#include <TTreeReaderArray.h>


void MassBackG()
{

    Int_t decision;
    Char_t cha;;
    std::cout << "Show Progress Check? (y=0/n=1-9)  ";
    std::cin >> decision;
    if (decision == 0) std::cout << "Beginning the Macro" << std::endl;

    //Fitting function
    TF1* fitfunc = new TF1("fitfunc", "[0]*exp(-[1]*x+[2])", 4300, 6200);
    
    Int_t i;
    //B Histograms
    TH1D *BMbkgHighHist = new TH1D("BM_High", "Histogram of the mass of B above 5.5 GeV", 100, 5500, 6200);
    BMbkgHighHist->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    BMbkgHighHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BMbkgLowHist = new TH1D("BM_Low", "Histogram of the mass of B below 5.0 GeV", 100, 4300, 5000);
    BMbkgLowHist->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    BMbkgLowHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BMbkgMidHist = new TH1D("BM_Mid", "Histogram of the mass of B between 5.0, 5.5 GeV", 100, 5000, 5500);
    BMbkgMidHist->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    BMbkgMidHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *BMbkgHist = new TH1D("BM_Full", "Histogram of the mass of B", 100, 4300, 6200);
    BMbkgLowHist->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    BMbkgLowHist->GetYaxis()->SetTitle("Number of Entries");
    TH1D *bkgHist1 = new TH1D("BKG1", "Histogram of the background for the mass of B data", 100, 4300, 6200);
    bkgHist1->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    bkgHist1->GetYaxis()->SetTitle("Number of Entries");
    TH1D *bkgHist2 = new TH1D("BKG2", "Histogram of the background for the mass of B data", 100, 4300, 6200);
    bkgHist2->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    bkgHist2->GetYaxis()->SetTitle("Number of Entries");
    TH1D *bkgHist3 = new TH1D("BKG3", "Histogram of the background for the mass of B data", 100, 4300, 6200);
    bkgHist3->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    bkgHist3->GetYaxis()->SetTitle("Number of Entries");
    TH1D *sigHist1 = new TH1D("Signal1", "Histogram of just the signal of the mass of the B", 100, 4300, 6200);
    sigHist1->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    sigHist1->GetYaxis()->SetTitle("Number of Entries");
    TH1D *sigHist2 = new TH1D("Signal2", "Histogram of just the signal of the mass of the B", 100, 4300, 6200);
    sigHist2->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    sigHist2->GetYaxis()->SetTitle("Number of Entries");
    TH1D *sigHist3 = new TH1D("Signal3", "Histogram of just the signal of the mass of the B", 100, 4300, 6200);
    sigHist3->GetXaxis()->SetTitle("Mass/ GeV/c^{2}");
    sigHist3->GetYaxis()->SetTitle("Number of Entries");
    
    if (decision == 0) std::cout << "Histograms Created " << std::endl;


  //Get the tree from the root file for the monte carlo / signal
    TFile *fMC = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_Signal_MC_MD.root");
    TTreeReader mcReader("KMuE/DecayTree", fMC);
    TTree *DTmc = mcReader.GetTree();
    if (decision == 0) std::cout << "Monte Carlo Tree Got " << std::endl;

  //Get the tree from the root file for the data / background
    TFile *fS = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root");
    TTreeReader sigReader("KMuE/DecayTree",fS);
    TTree *DTsig = sigReader.GetTree();
    if (decision == 0) std::cout << "Signal Tree Got " << std::endl;

  //Create Branch objects and store the necessary branches within them
    TBranch *BMmc = DTmc->GetBranch("B_M");
    TBranch *BMsig = DTsig->GetBranch("B_M");

    Double_t BMassSig, BMassMC; 

    DTsig->SetBranchAddress("B_M", &BMassSig);
//    DTmc->SetBranchAddress("B_M", &BMassSig);
    if (decision == 0) std::cout << "Branches Created " << std::endl;

    Int_t entries = 200000;
 //   if ( BMmc->GetEntries() < BMsig->GetEntries() ) entries = BMmc->GetEntries();
 //   else entries = BMsig->GetEntries();
    if (BMsig->GetEntries() < (entries*1.1)) entries = BMsig->GetEntries();

    
    
    if (decision == 0) std::cout << "Entering loop of " << entries << " Entries." << std::endl;
    for(i=0; i<entries; i++) {
      DTmc->GetEntry(i);
      DTsig->GetEntry(i);
      
      BMbkgHist->Fill(BMassSig);
      if(BMassSig > 5500) BMbkgHighHist->Fill(BMassSig);
      else if(BMassSig<5000) BMbkgLowHist->Fill(BMassSig);
      else BMbkgMidHist->Fill(BMassSig);
        
      
    }

    if((BMbkgHighHist->GetEntries()+BMbkgLowHist->GetEntries()+BMbkgMidHist->GetEntries()) != entries) {
      std::cout << "Hist Error" << std::endl;
      return;
    }
    
    if (decision == 0) std::cout << "Histograms filled " << std::endl;

    TFile File("MassBKG.root","RECREATE");
    if (decision == 0) std::cout << "Root file created " << std::endl;

    
    BMbkgHighHist->Fit("fitfunc");
    Double_t HiPara0 = fitfunc->GetParameter(0);
    Double_t HiPara1 = fitfunc->GetParameter(1);
    Double_t HiPara2 = fitfunc->GetParameter(2);
    bkgHist1->FillRandom("fitfunc", entries);
    BMbkgLowHist->Fit("fitfunc");
    BMbkgHist->Fit("fitfunc");
    bkgHist2->FillRandom("fitfunc", entries);
    Double_t LoPara0 = fitfunc->GetParameter(0);
    Double_t LoPara1 = fitfunc->GetParameter(1);
    Double_t LoPara2 = fitfunc->GetParameter(2);
    bkgHist3->FillRandom("fitfunc", entries);
    
    
    for(i=0; i<BMbkgHist->GetNbinsX(); i++) {
      sigHist1->SetBinContent(i, (BMbkgHist->GetBinContent(i)-bkgHist1->GetBinContent(i)));
      sigHist2->SetBinContent(i, (BMbkgHist->GetBinContent(i)-bkgHist2->GetBinContent(i)));
      sigHist3->SetBinContent(i, (BMbkgHist->GetBinContent(i)-bkgHist3->GetBinContent(i)));
      
    }
    
    BMbkgHist->Write();
    BMbkgLowHist->Write();
    BMbkgMidHist->Write();
    BMbkgHighHist->Write();
    bkgHist1->Write();
    bkgHist2->Write();
    bkgHist3->Write();
    sigHist1->Write();
    sigHist2->Write();
    sigHist3->Write();
    if (decision == 0) std::cout << "Histograms Written to File 'MassBKG.root' " << std::endl;
    
    std::cout << "High Parameters: [0] = " << HiPara0 << std::endl;
    std::cout << "                 [1] = " << HiPara1 << std::endl;
    std::cout << "                 [2] = " << HiPara2 << std::endl;
    std::cout << "Low Parameters:  [0] = " << LoPara0 << std::endl;
    std::cout << "                 [1] = " << LoPara1 << std::endl;
    std::cout << "                 [2] = " << LoPara2 << std::endl;
    
    

    TBrowser *b = new TBrowser();

    File.Close();
    fMC->Close();
    fS->Close();
    delete BMbkgHighHist;
    delete BMbkgLowHist;
    delete BMbkgHist;
    delete bkgHist1;
    delete bkgHist2;
    delete sigHist1;
    delete sigHist2;
    delete sigHist3;
    if (decision == 0) std::cout << "Files closed and objects deleted\nENDING. " << std::endl;
}
