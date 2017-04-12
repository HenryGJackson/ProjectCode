
//-----------------------------
//NOTE: ANALYSE NAMESPACE
//-----------------------------
void analyse::toggleBranches( TTree* tree ){

  tree->SetBranchStatus( "*", 0 );

  std::vector< std::string > branches = {
    "Kplus_P*", "piminus_P*", "muplus_P*", "muminus_P*"
  };

  for ( int i = 0; i < branches.size(); ++i ){
    tree->SetBranchStatus( branches[i].c_str(), 1 );
  }
  return ;
}

void analyse::DanCuts(const Double_t BDTcut, const TString inname = "",
                      const TString intreename = "", const TString outname = "",
                      TString p1label = "Kaon", TString p2label = "Mu", TString p3label = "E")
{
    const bool istruth = false;

    TFile* infile = TFile::Open(inname);
    if ( !infile ) return ;
    gROOT->cd();

    TTree* intree = (TTree*) infile->Get(intreename);
    if ( !intree ) return ;
    Int_t ID = 0;
    intree->SetBranchAddress( (p1label + "_ID"), &ID );
    const Long64_t entries = intree->GetEntries();

  //Reader variables
    TTreeReader reader(intreename,infile);
    TTreeReaderValue<bool> KaonMuon(reader,p1label+"_isMuon");
    TTreeReaderValue<bool> ElectronMuon(reader,p3label+"_isMuon");
    TTreeReaderValue<double> ProbK(reader,p1label+"_ProbNNk");
    TTreeReaderValue<double> ProbMu(reader,p2label+"_ProbNNmu");
    TTreeReaderValue<double> ProbMu2(reader,p3label+"_ProbNNmu"); //Use for BuKMuMu data
    TTreeReaderValue<double> Probe(reader,p3label+"_ProbNNe"); //Use for BuKMuE data
    TTreeReaderValue<int> KID(reader,p1label+"_ID");
    TTreeReaderValue<int> MuID(reader,p2label+"_ID");
    TTreeReaderValue<int> eID(reader,p3label+"_ID");
    TTreeReaderValue<bool> L0H(reader,"B_L0HadronDecision_TOS");
    TTreeReaderValue<bool> L0Mu(reader,"B_L0MuonDecision_TOS");
    TTreeReaderValue<bool> L0e(reader,"B_L0ElectronDecision_TOS");
    TTreeReaderValue<bool> Hlt1All(reader,"B_Hlt1TrackAllL0Decision_TOS");
    TTreeReaderValue<bool> Hlt1Mu(reader,"B_Hlt1TrackMuonDecision_TOS");
    TTreeReaderValue<bool> Hlt22Body(reader,"B_Hlt2Topo2BodyBBDTDecision_TOS");
    TTreeReaderValue<bool> Hlt2Mu2Body(reader,"B_Hlt2TopoMu2BodyBBDTDecision_TOS");
    TTreeReaderValue<bool> Hlt2Mu3Body(reader,"B_Hlt2TopoMu3BodyBBDTDecision_TOS");
   TTreeReaderValue<float> weight(reader, "BDT_weights");
 //   TTreeReaderValue<int> BKGCAT(reader, "B_BKGCAT"); //Needed if using MC tree
    TTreeReaderValue<double> aKMu(reader, "a"+p1label+p2label);
    TTreeReaderValue<double> aKe(reader, "a"+p1label+p3label);
    TTreeReaderValue<double> aMue(reader, "a"+p2label+p3label);
    // TTreeReaderValue<double> aKMu(reader, "aKMu");
    // TTreeReaderValue<double> aKe(reader, "aKe");
    // TTreeReaderValue<double> aMue(reader, "aMue");
    TTreeReaderValue<double> mKisMuMu(reader, "m"+p1label+"is"+p2label+p2label);
    TTreeReaderValue<double> mMueisMu(reader, "m"+p2label+p3label+"is"+p2label);
    TTreeReaderValue<double> mKeisMu(reader, "m"+p1label+p3label+"is"+p2label);
    TTreeReaderValue<double> mKisee(reader, "m"+p1label+"is"+p3label+p3label);
    TTreeReaderValue<double> Probpi(reader,p3label+"_ProbNNpi");

    int exl,chargeKMu,chargeKe,chargeMue;
    int ikept=0;

    TFile *cutfile = new TFile(outname,"RECREATE");
    TTree *cuttree = intree->CloneTree(0);

  for ( Long64_t i = 0; i < entries; ++i )
  {
    //initialse loop
    intree->GetEntry(i);
    reader.Next();
    exl=0;
    chargeKMu = 0; chargeKe = 0; chargeMue = 0;


    //Compare charges   K+ = 321 K- = -321    Mu- = 13 Mu+ = -13    E- = 11 E+ = -11
    if((*KID)>0&&(*MuID)<0){chargeKMu++;}
    if((*KID)<0&&(*MuID)>0){chargeKMu++;}
    if((*KID)>0&&(*eID)<0){chargeKe++;}
    if((*KID)<0&&(*eID)>0){chargeKe++;}
    if((*eID)>0&&(*MuID)>0){chargeMue++;}
    if((*eID)<0&&(*MuID)<0){chargeMue++;}

    //Exclude if kaon or electron misidentifed
    if(*KaonMuon) {exl++;}
    if(p3label == "E" && *ElectronMuon) {exl++;}
    if(p3label == "E" &&*Probpi>0.6){exl++;}
    //Exclude if kaon is not kaon-like and same for muon and electron
    if(*ProbK<0.1) {exl++;}
    if(*ProbMu<0.1) {exl++;}
    if(p3label == "E" && *Probe<0.1) {exl++;}
    else if(*ProbMu2<0.1) exl++;
    //Exclude if angle between any of the decay products is too small
    if(*aKMu<0.0005&&chargeKMu>0) {exl++;}
    if(*aKe<0.0005&&chargeKe>0) {exl++;}
    if(*aMue<0.0005&&chargeMue>0) {exl++;}
   if(*weight<BDTcut) {exl++;}
   // if(*BKGCAT!=10 && *BKGCAT != 50) {exl++;}
    //Exclude according to trigger
    if(*L0H==1||*L0Mu==1||*L0e==1)
    {
      if(*Hlt1All==1||*Hlt1Mu==1)
      {
        if(*Hlt22Body==0&&*Hlt2Mu2Body==0&&*Hlt2Mu3Body==0){exl++;}
      }
      else{exl++;}
    }
    else{exl++;}
    //Exclude events according to mass combinations

    if(((*mKisMuMu)>3000)&&((*mKisMuMu)<3200)&&((chargeKMu)==0)){exl++;}
    if(((*mMueisMu)>3000)&&((*mMueisMu)<3200)&&((chargeMue)==0)){exl++;}
    if(((*mKeisMu)>1825)&&((*mKeisMu)<1900)&&((chargeKe)==0)){exl++;}
    if(((*mKisee)>2800)&&((*mKisee)<3200)&&((chargeKe)==0)){exl++;}
    //Keep remaining events
    if(exl==0){cuttree->Fill();ikept++;}
    std::cout << "\r" << "Cut Progress: " << i*100 / (entries-1) << " %." << std::flush;
  }
  std::cout << std::endl;
  std::cout << ikept << " of " << entries << " events remaining after cuts..." << (Double_t)ikept*100/entries << " %." << std::endl;
  std::cout << "Cut at BDT output: " << BDTcut << std::endl;

  infile->Close();
  cutfile->Write();
  cutfile->Close();
  return ;
}

void analyse::MakeAllCuts(Int_t dataset = 0) {
    TString inpath = "/home/epp/phunch/Documents/TMVA/RootFiles/original/";
    TString outpath = "/home/epp/phunch/Documents/TMVA/RootFiles/original/";

    TString btname  = "KMuE/DecayTree";

    TString f2name1 = "2011_MgUp_WithDerived.root";
    TString f2name2 = "2011_MgDn_WithDerived.root";
    TString f2name3 = "2012_MgUp_WithDerived.root";
    TString f2name4 = "2012_MgDn_WithDerived.root";
    TString dext1 = "_2011_MgUp";
    TString dext2 = "_2011_MgDn";
    TString dext3 = "_2012_MgUp";
    TString dext4 = "_2012_MgDn";
    TString btree = "DecayTree";

    if(dataset < 1 || dataset > 4) {
      std::cout << "Must Specify dataset 1-4.\n";
      return;
    }
    if(dataset == 1) {
        Double_t* punz1 = punzi::GetPunzi("BDT",outpath+"TMVA.root",outpath+dext1+".root",btree,dext1,kTRUE,kFALSE);
        if(!punz1) {return;}
        analyse::DanCuts(punz1[1],outpath+dext1+".root",btree,outpath+dext1+"_cut.root");
    }
    else if(dataset == 2) {
        Double_t* punz2 = punzi::GetPunzi("BDT",outpath+"TMVA.root",outpath+dext2+".root",btree,dext2,kTRUE,kFALSE);
        if(!punz2) {return;}
        analyse::DanCuts(punz2[1],outpath+dext2+".root",btree,outpath+dext2+"_cut.root");
    }
    else if(dataset == 3) {
        Double_t* punz3 = punzi::GetPunzi("BDT",outpath+"TMVA.root",outpath+dext3+".root",btree,dext3,kTRUE,kFALSE);
        if(!punz3) {return;}
        analyse::DanCuts(punz3[1],outpath+dext3+".root",btree,outpath+dext3+"_cut.root");
    }
    else {
        Double_t* punz4 = punzi::GetPunzi("BDT",outpath+"TMVA.root",outpath+dext4+".root",btree,dext4,kTRUE,kFALSE);
        if(!punz4) {return;}
        analyse::DanCuts(punz4[1],outpath+dext4+".root",btree,outpath+dext4+"_cut.root");
    }

    return;
}

void analyse::MakeTreesWithAllData(Int_t infile = 0, Bool_t trainBDT = kTRUE) {
    TString inpath = "/storage/epp2/phsnab/BuKMuE/";
    TString outpath = "/home/epp/phunch/Documents/TMVA/RootFiles/original/";
    TString btname  = "KMuE/DecayTree";
    TString sfname = "/home/epp/phunch/Documents/TMVA/RootFiles/original/MC_withBKGCATcut.root";
    TString stname = "DecayTree";
    TString btree = "DecayTree";
    TString rec = "RECREATE";
    TString upd = "UPDATE";
    TString method = "BDT";
    TString bkg = "_BKG";
    TString sig = "_MC";
    TString sf2name = "/home/epp/phunch/Documents/TMVA/RootFiles/original/MC_withBKGCATcutWithDerived.root";

    TStopwatch sw;
    sw.Start();

    if(infile < 1 || infile > 4) {std::cout << "Must Specify entry file 1-4\n\n"; return;}
    TString dext3 = "_2012_MgUp";
    TString f2name3 = "2012_MgUp_WithDerived.root";
    analyse::GetDerivedVariables(sf2name,sfname,stname);
    if(trainBDT) {
      //  cut::CutDoubleVar("B_M", ">", 5500.0,outpath+dext3+"WithBCut_Derived.root",outpath+f2name3,btree);
        MVAanalysis::TMVAClassification(outpath+dext3+"WithBCut_Derived.root",btree,sf2name,btree,outpath+"TMVA.root");

    }
    if(infile == 1) {
        TString f1name1 = "BuKMuE_2011_MgUp_Updated.root";
        TString f2name1 = "2011_MgUp_WithDerived.root";
        TString dext1 = "_2011_MgUp";
        analyse::GetDerivedVariables(outpath+f2name1,inpath+f1name1,btname);
        //CutBMass(kTRUE,outpath+dext1+"WithBCut_Derived.root",outpath+f2name1,btree);
        //TMVAClassification(outpath+dext1+"WithBCut_Derived.root",btree,sf2name,btree,outpath+"TMVA"+f2name1,dext1);
        MVAanalysis::TMVAClassificationApplication(method,rec,bkg,outpath+f2name1,btree,"dataset/",outpath+"Weights"+dext1+".root");
        TMVAtrees::GetTMVATrees(outpath+f2name1,btree,outpath+"Weights"+dext1+".root",outpath+dext1+".root");
        std::cout << "\n\n*** COMPLETE ***\nTree Written to: " << outpath << dext1 <<".root" << "\nExiting..\n";
    }
    else if(infile == 2) {
        TString f1name2 = "BuKMuE_2011_MgDn.root";
        TString f2name2 = "2011_MgDn_WithDerived.root";
        TString dext2 = "_2011_MgDn";
        analyse::GetDerivedVariables(outpath+f2name2,inpath+f1name2,btname);
        //CutBMass(kTRUE,outpath+dext2+"WithBCut_Derived.root",f2name2,btree);
        //TMVAClassification(outpath+dext2+"WithBCut_Derived.root",btree,sf2name,btree,outpath+"TMVA"+f2name2,dext2);
        MVAanalysis::TMVAClassificationApplication(method,rec,bkg,outpath+f2name2,btree,"dataset/",outpath+"Weights"+dext2+".root");
        TMVAtrees::GetTMVATrees(outpath+f2name2,btree,outpath+"Weights"+dext2+".root",outpath+dext2+".root");
        std::cout << "\n\n*** COMPLETE ***\nTree Written to: " << outpath << dext2 <<".root" << "\nExiting..\n";
    }
    else if(infile == 3) {
        TString f1name3 = "BuKMuE_2012_MgUp.root";
        GetDerivedVariables(outpath+f2name3,inpath+f1name3,btname);
        // CutBMass(kTRUE,outpath+dext3+"WithBCut_Derived.root",outpath+f2name3,btree);
        // TMVAClassification(outpath+dext3+"WithBCut_Derived.root",btree,sf2name,btree,outpath+"TMVA"+f2name3,dext3);
        MVAanalysis::TMVAClassificationApplication(method,rec,bkg,outpath+f2name3,btree,"dataset/",outpath+"Weights"+dext3+".root");
        TMVAtrees::GetTMVATrees(outpath+f2name3,btree,outpath+"Weights"+dext3+".root",outpath+dext3+".root");
        std::cout << "\n\n*** COMPLETE ***\nTree Written to: " << outpath << dext3 <<".root" << "\nExiting..\n";
    }
    else if(infile == 4) {
        TString f1name4 = "BuKMuE_2012_MgDn.root";
        TString f2name4 = "2012_MgDn_WithDerived.root";
        TString dext4 = "_2012_MgDn";
        analyse::GetDerivedVariables(outpath+f2name4,inpath+f1name4,btname);
        // CutBMass(kTRUE,outpath+dext4+"WithBCut_Derived.root",outpath+f2name4,btree);
        // TMVAClassification(outpath+dext4+"WithBCut_Derived.root",btree,sf2name,btree,outpath+"TMVA"+f2name4,dext4);
        MVAanalysis::TMVAClassificationApplication(method,rec,bkg,outpath+f2name4,btree,"dataset/",outpath+"Weights"+dext4+".root");
        TMVAtrees::GetTMVATrees(outpath+f2name4,btree,outpath+"Weights"+dext4+".root",outpath+dext4+".root");
        std::cout << "\n\n*** COMPLETE ***\nTree Written to: " << outpath << dext4 <<".root" << "\nExiting..\n";
    }
    sw.Stop();
    std::cout << "Time Elapsed = "; sw.Print();
    std::cout << std::endl;
    std::cout << std::endl;
    return;
}

void analyse::DoBoth(Int_t dataset = 0) {
    TStopwatch sw;
    sw.Start();
    analyse::MakeTreesWithAllData(dataset);
    analyse::MakeAllCuts(dataset);
    sw.Stop();
    std::cout << "Time Elapsed = "; sw.Print();
    std::cout << std::endl;
    return;
}

void analyse::makealltrees() {
    TStopwatch sw;
    sw.Start();
    analyse::MakeTreesWithAllData(1,kTRUE);
    analyse::MakeTreesWithAllData(2,kFALSE);
    analyse::MakeTreesWithAllData(3,kFALSE);
    analyse::MakeTreesWithAllData(4,kFALSE);
    sw.Stop();
    std::cout << "Time Elapsed = "; sw.Print();
    std::cout << std::endl;
    return;
}

void analyse::cutalldata() {
    TStopwatch sw;
    sw.Start();
    analyse::MakeAllCuts(1);
    analyse::MakeAllCuts(2);
    analyse::MakeAllCuts(3);
    analyse::MakeAllCuts(4);
    sw.Stop();
    std::cout << "Time Elapsed = "; sw.Print();
    std::cout << std::endl;
    return;
}

void analyse::everything() {
    TStopwatch sw;
    sw.Start();
    analyse::makealltrees();
    analyse::cutalldata();
    sw.Stop();
    std::cout << "Time Elapsed = "; sw.Print();
    std::cout << std::endl << std::endl;
    return;
}

void analyse::GetDerivedVariables(TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithPTandAngles.root",
           TString bfname = "/home/henry/Project/BuKMuE_2011_MgUp.root",
           TString btname = "KMuE/DecayTree", TString p1label = "Kaon", TString p2label = "Mu", TString p3label = "E"
) {

    //Open Files to read background and signal and get respective trees
    // TFile* bfile = TFile::Open(bfname);
    // TTree* btree = (TTree*)bfile->Get(btname);
    TTree* btree = load::tree(bfname,btname);
    if(!btree) return;

    //Create output file and clone trees
    Int_t entBkg = btree->GetEntries();
    TFile* ofile = new TFile(ofname,"RECREATE");
    std::cout << "\rCloning Background Tree                     " << std::flush;
    TTree* bntree = btree->CloneTree(entBkg,"fast");

    Bool_t hasPT = kFALSE;
    Bool_t hasIPChi2 = kFALSE;
    Bool_t hasAngles = kFALSE;
    Bool_t hassMassCombos = kFALSE;
    if(bntree->GetBranch("MaxPT")) hasPT = kTRUE;
    if(bntree->GetBranch("MaxIPChi2")) hasIPChi2 = kTRUE;
    if(bntree->GetBranch("aKMu")) hasAngles = kTRUE;
    if(bntree->GetBranch("mMue")) hasPT = kTRUE;


    //Create Particle objects, used to calculate angles
    btree->SetBranchStatus("*",0);
    const bool istruth = false;
    Particle<Double_t> kaon(p1label, btree, istruth);			//Class from:   particlefromtree.cpp
    Particle<Double_t> muon(p2label, btree, istruth);  			//Class from:   particlefromtree.cpp
    Particle<Double_t> electron(p3label, btree, istruth);			//Class from:   particlefromtree.cpp


    //Set up branches for reading data.
    //Must be double or error returned:
    //Transverse Momentum:
    Double_t KBkgPT, MuBkgPT, eBkgPT;
    btree->SetBranchStatus(p1label+"_PT",  1); btree->SetBranchAddress(p1label+"_PT",  &KBkgPT);
    btree->SetBranchStatus(p2label+"_PT",  1); btree->SetBranchAddress(p2label+"_PT",  &MuBkgPT);
    btree->SetBranchStatus(p3label+"_PT",   1); btree->SetBranchAddress(p3label+"_PT",   &eBkgPT);
    //Impact Parameter Chi Squared
    Double_t KIPChi2, MuIPChi2, eIPChi2;
    btree->SetBranchStatus(p1label+"_IPCHI2_OWNPV", 1); btree->SetBranchAddress(p1label+"_IPCHI2_OWNPV",  &KIPChi2);
    btree->SetBranchStatus(p2label+"_IPCHI2_OWNPV",   1); btree->SetBranchAddress(p2label+"_IPCHI2_OWNPV",   &MuIPChi2);
    btree->SetBranchStatus(p3label+"_IPCHI2_OWNPV",   1);  btree->SetBranchAddress(p3label+"_IPCHI2_OWNPV",   &eIPChi2);

    //Set up new branches to save data to.
    //NOTE Only works if these are float...???-> WHY???
    bntree->SetBranchStatus("*",0);
    //Transverse Momentum:
    Double_t BkgMaxPT, BkgMinPT;
    TBranch *BbmaxPT, *BbminPT, *BaKMu, *BaKe, *BaMue, *BmaxIPChi2, *BminIPChi2;
    TBranch *BmMue, *BmKe, *BmKisMuMu, *BmKeisMu, *BmKisee, *BmMueisMu;
    if(!hasPT) {
          BbmaxPT = bntree->Branch("MaxPT", &BkgMaxPT, "MaxPT/D"); bntree->SetBranchStatus("MaxPT",1);
          BbminPT = bntree->Branch("MinPT", &BkgMinPT, "MinPT/D"); bntree->SetBranchStatus("MinPT",1);
    }
    //Relative Track Angles
    Double_t ang1, ang2, ang3;
    if(!hasAngles) {
          BaKMu   = bntree->Branch("a"+p1label+p2label,  &ang1, "a"+p1label+p2label+"/D");      bntree->SetBranchStatus("a"+p1label+p2label,1);
          BaKe    = bntree->Branch("a"+p1label+p3label,   &ang2, "a"+p1label+p3label+"/D");       bntree->SetBranchStatus("a"+p1label+p3label,1);
          BaMue   = bntree->Branch("a"+p2label+p3label,  &ang3, "a"+p2label+p3label+"/D");      bntree->SetBranchStatus("a"+p2label+p3label,1);
    }
    //Impact Parameter Chi Squared
    Double_t MaxIPChi2, MinIPChi2;
    if(!hasIPChi2) {
         BmaxIPChi2 = bntree->Branch("MaxIPChi2", &MaxIPChi2, "MaxIPChi2/D"); bntree->SetBranchStatus("MaxIPChi2",1);
         BminIPChi2 = bntree->Branch("MinIPChi2", &MinIPChi2, "MinIPChi2/D"); bntree->SetBranchStatus("MinIPChi2",1);
    }
    //Mass combinations
    Double_t mMue,mKe,mKisMuMu,mKeisMu,mKisee,mMueisMu;
    const Double_t MPI = 139.6;
    const Double_t MK = 493.7;
    const Double_t MMU = 105.7;
    const Double_t ME = 0.511;
    TLorentzVector VKisMu,VKise,VeisMu;
    if(!hassMassCombos) {
          BmMue = bntree->Branch("m"+p2label+p3label, &mMue, "m"+p2label+p3label+"/D"); bntree->SetBranchStatus("m"+p2label+p3label,1);
          BmKe = bntree->Branch("m"+p1label+p3label, &mKe, "m"+p1label+p3label+"/D"); bntree->SetBranchStatus("m"+p1label+p3label,1);
          BmKisMuMu = bntree->Branch("m"+p1label+"is"+p2label+p2label, &mKisMuMu, "m"+p1label+"is"+p2label+p2label+"/D"); bntree->SetBranchStatus("m"+p1label+"is"+p2label+p2label,1);
          BmKeisMu = bntree->Branch("m"+p1label+p3label+"is"+p2label, &mKeisMu, "m"+p1label+p3label+"is"+p2label+"/D"); bntree->SetBranchStatus("m"+p1label+p3label+"is"+p2label,1);
          BmKisee = bntree->Branch("m"+p1label+"is"+p3label+p3label, &mKisee, "m"+p1label+"is"+p3label+p3label+"/D"); bntree->SetBranchStatus("m"+p1label+"is"+p3label+p3label,1);
          BmMueisMu = bntree->Branch("m"+p2label+p3label+"is"+p2label, &mMueisMu, "m"+p2label+p3label+"is"+p2label+"/D"); bntree->SetBranchStatus("m"+p2label+p3label+"is"+p2label,1);
    }
    Int_t ent= entBkg;
    Int_t i;
    //Fill new branches with data
    if(!hasPT || !hasIPChi2 || !hasAngles || !hassMassCombos) {
          for(i = 0; i < ent; i++) {
              if(i < entBkg) {
                  btree->GetEntry(i);
                  if(!hasPT){
                        //Transverse Momentum:
                        BkgMaxPT = angle::GetMax(KBkgPT,MuBkgPT,eBkgPT);
                        BkgMinPT = angle::GetMin(KBkgPT,MuBkgPT,eBkgPT);
                        BbmaxPT->Fill();
                        BbminPT->Fill();
                  }
                  if(!hasAngles) {
                        //Relative Track Angles
                        ang1 = angle::GetAngle(kaon.getVec(), muon.getVec());
                        ang2 = angle::GetAngle(kaon.getVec(), electron.getVec());
                        ang3 = angle::GetAngle(muon.getVec(), electron.getVec());
                        BaKMu->Fill();
                        BaKe->Fill();
                        BaMue->Fill();
                  }
                  if(!hasIPChi2) {
                        //Impact Parameter Chi Squared
                        MaxIPChi2 = angle::GetMax(KIPChi2, MuIPChi2, eIPChi2);
                        MinIPChi2 = angle::GetMin(KIPChi2, MuIPChi2, eIPChi2);
                        BmaxIPChi2->Fill();
                        BminIPChi2->Fill();
                  }
                  if(!hassMassCombos) {
                        //Mass combinations
                        VKisMu.SetXYZM((kaon.getVec()).Px(),(kaon.getVec()).Py(),(kaon.getVec()).Pz(),MMU);
                        VKise.SetXYZM((kaon.getVec()).Px(),(kaon.getVec()).Py(),(kaon.getVec()).Pz(),ME);
                        VeisMu.SetXYZM((electron.getVec()).Px(),(electron.getVec()).Py(),(electron.getVec()).Pz(),MMU);
                        mKe = (kaon.getVec()+electron.getVec()).M();
                        mMue = (muon.getVec()+electron.getVec()).M();
                        mKisMuMu = (VKisMu+muon.getVec()).M();
                        mKeisMu = (kaon.getVec()+VeisMu).M();
                        mKisee = (VKise+electron.getVec()).M();
                        mMueisMu = (muon.getVec()+VeisMu).M();
                        BmKe->Fill();
                        BmMue->Fill();
                        BmKisMuMu->Fill();
                        BmKeisMu->Fill();
                        BmKisee->Fill();
                        BmMueisMu->Fill();
                  }
              }
              std::cout << "\rNew Branches Progrss: " << std::setw(4) << i * 100 / (ent - 1) << " %." << std::flush;
          }
    }
    else std::cout << "\nTree already has the derived variables\n";
    //Close files and save new trees to output file.
    bntree->SetBranchStatus("*",1);
    bntree->Write();
    ofile->Close();
   // bfile->Close();

    std::cout << "\rDONE                                        " << std::endl;
    return;
}


//-----------------------------
//NOTE: EXPONENT CLASS
//-----------------------------
Double_t exponent::Evaluate(Double_t x) {
	x = x - c;
	return(a * exp(x * b));
}
Double_t exponent::EvaluateIntegral(Double_t xmin, Double_t xmax) {
	xmin = xmin - c;
	xmax = xmax - c;
	return((Evaluate(xmax) - Evaluate(xmin))/b);
}
Double_t exponent::Geta() { return(a); }
Double_t exponent::Getb() { return(b); }
Double_t exponent::Getc() { return(c); }
void exponent::aMultiply(Double_t multiplier) { a = a*multiplier; return; }
void exponent::SetParams(Double_t a0, Double_t b0, Double_t c0) {
	a = a0;
	b = b0;
	c = c0;
	return;
}
void exponent::PrintParams() {
	std::cout << "a =      " << a << std::endl;
	std::cout << "b =      " << b << std::endl;
	std::cout << "offset = " << c << std::endl;
	return;
}


//***********************************************************
//					5500 < mB < 6200						*
// Then outputs the number of events in the signal region   *
// with range:												*
//					5000 < mB < 5500						*
// To plot the fit of the B Mass set the input parameter:   *
//					 PlotFit = kTRUE						*
//***********************************************************
Double_t punzi::GetBkgEvents(TString filename = "/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root", TString treename = "KMuE/DecayTree",
					  Bool_t PlotFit = kFALSE) {

	Double_t BMass;
	Double_t m_min = 5363;
	Double_t m_max = 6200;
	Double_t mass_reg_min = 5006;
	TH1D *hist_mB = new TH1D("BM_High", "Histogram of the mass of B above 5.5 GeV", 1000, m_min, m_max);
	hist_mB->SetXTitle("B Mass /GeV"); hist_mB->SetYTitle("Number of Entries");

//	TString filename = "/home/epp/phunch/Documents/TMVA/NewTreesWithCuts.root";
//	TString treename = "DecayTree";
//	TString filename = "/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root";
//	TString treename = "KMuE/DecayTree";
//	TString filename = "/home/epp/phunch/Documents/TMVA/RootFiles/BMassCut.root";
//	TString treename = "BkgTree";
	TFile *bkgfile = TFile::Open(filename);
	TTreeReader bkgReader(treename, bkgfile);
	TTree *bkgtree = bkgReader.GetTree();

	bkgtree->SetBranchStatus("*", kFALSE);  //Disable all branches
	bkgtree->SetBranchStatus("B_M", kTRUE); //enable the branch we want
	bkgtree->SetBranchAddress("B_M", &BMass);

	//Fill histogram of B Mass above the B Mass region (>5500)
	Int_t ents = bkgtree->GetEntries();
	Int_t new_ents = 0;
	Int_t mass_range_ents = 0;
	for (int i = 0; i < ents; i++) {
		bkgtree->GetEntry(i);
		if (BMass > m_min && BMass < m_max) {
			hist_mB->Fill(BMass);
			new_ents++;
		}
		else if (BMass > mass_reg_min && BMass < m_min) {
			mass_range_ents++;
		}
	}

	//Use RooFit to fit the B Mass with an exponential decay
	RooRealVar m("m", "mass", m_min, m_max);
	RooRealVar C("C", "exp_param", -0.001, -0.01, 0.0);
	RooDataHist hBMass("hBMass", "hBMass", m, Import(*hist_mB));
	RooExponential bkgExp("bkgExp", "Exponential for fitting B Mass", m, C);
	bkgExp.fitTo(hBMass);

	if (PlotFit) {
		//Plot histogram with fit
		RooPlot* frame = m.frame(Title("B Mass Histogram with RooExponential Fit"));
		hBMass.plotOn(frame);
		bkgExp.plotOn(frame);
		TCanvas *c1 = new TCanvas("BMass", "B Mass Histogram", 400, 400);
		frame->Draw();
	}

	//Create exponent object to analyse fit
	exponent *myexp = new exponent;
	myexp->SetParams(1.0, C.getValV(), 0.0);

	//Evaluate the integral in the current test region (5500 < mB < 6200)
//	myexp->PrintParams();
	Double_t integ_above = myexp->EvaluateIntegral(m_min, m_max);

	//Evaluate the integral in the B Mass region (5000 < mB < 5500)
	Double_t integ_at = myexp->EvaluateIntegral(mass_reg_min, m_min);

	//Use
	//		N(m_a,m_b) / N(m_c,m_d)  =  integral{m_a->m_b}f(m)dm / integral{m_c->m_d}f(m)dm
	//to find the nuber of events in the signal region
	Double_t n_ents_at_Bmass = new_ents*integ_at / integ_above;

	std::cout << "*** " << mass_reg_min << " < m_B < " << m_min << " ***" << std::endl;
	std::cout << "Integrated events (RooExponential): " << n_ents_at_Bmass << ", Error = " << 100 * (n_ents_at_Bmass - mass_range_ents) / mass_range_ents << " %" << std::endl;
	std::cout << "Actual events:                      " << mass_range_ents << std::endl;

	bkgfile->Close();
	return(n_ents_at_Bmass);
}


//*******************************************************************
//    Function that plots the Punzi Figure of Merit for the			*
//    method name passed the function as method.					*
//    To use the function GetBkgEvents(), set:						*
//				calculate_Nbkg = kTRUE								*
//    To tell GetBkgEvents() to plot the B Mass fit, set:			*
//				   PlotFit = kTRUE									*
// Outputs an array where:	output[0] = maximum PFoM				*
//							output[1] = classifier output to cut at *
//							output[2] = Background efficiency at cut*
//							output[3] = Signal efficiency at cut	*
//*******************************************************************/
Double_t *punzi::GetPunzi(const TString method = "", TString mvafilename = "RootFiles/TMVA.root", TString bkgfilename = "BMassCut.root",
				   TString bkgtreename = "BkgTree", TString ext = "", Bool_t calculate_Nbkg = kTRUE, Bool_t PlotFit = kFALSE) {

	std::cout << "Getting Punzi Figure of Merit\n";

	//Open the file for reading efficiency data from
	TFile* tfile = TFile::Open(mvafilename);
	if(!tfile) {std::cout << "Couldn't Get TMVA file.\nExiting...\n"; return(NULL);}
	//Create path to histogram
	TString path = "dataset_dat" /*+ ext*/;
	path += "/Method_";
	path += method;
	path += "/";
	path += method;
	path += "/MVA_";
	path += method;

	Double_t cut, effS, effB, FoM;
	Double_t max_FoM = 0.0;

	//Read Efficiency Histograms:
	TH1D* hist_effB = (TH1D*)tfile->Get(path + "_effB");
	if(!hist_effB) {std::cout << "Couldn't Get Background efficiency histogram\nExiting...\n"; return(NULL);}
	hist_effB->SetXTitle("Classifier Output");
	hist_effB->SetYTitle("Efficiency");
	TH1D* hist_effS = (TH1D*)tfile->Get(path + "_effS");
	if(!hist_effS) {std::cout << "Couldn't Get Signal efficiency histogram\nExiting...\n"; return(NULL);}
	hist_effS->SetXTitle("Classifier Output");
	hist_effS->SetYTitle("Efficiency");

	//Create Punzi FoM Histogram
	Double_t min_output = hist_effB->GetXaxis()->GetBinLowEdge(1);
	Double_t max_output = hist_effB->GetXaxis()->GetBinLowEdge(hist_effB->GetNbinsX());
	max_output = max_output + hist_effB->GetXaxis()->GetBinWidth(hist_effB->GetNbinsX());
	TH1D *hist_PFoM = new TH1D("PFOM_" + method, "Punzi Figure of Merit " + method, hist_effB->GetNbinsX(), min_output*0.001, max_output*0.001);
	hist_PFoM->SetXTitle("Classifier Output");
	hist_PFoM->SetYTitle("Punzi Figure of Merit");
	hist_PFoM->GetYaxis()->SetTitleOffset(1.4);

	Int_t N_Bkg;
	//Get the number of background events in the signal region
	if (calculate_Nbkg) N_Bkg = punzi::GetBkgEvents(bkgfilename, bkgtreename, PlotFit);
	//To save time, the result of GetBkgEvents() is explicitly stated below.
	else			    N_Bkg = 442189;

	//Get each entry in the BKG efficiency histogram and use this to fill the histogram for the Punzi FoM
	for (int i = 0; i < hist_effB->GetNbinsX(); i++) {
		effB = hist_effB->GetBinContent(i + 1);
		effS = hist_effS->GetBinContent(i + 1);
		FoM = effS / (1.5 + sqrt(N_Bkg*effB));
		hist_PFoM->SetBinContent(i + 1, FoM);
		if (FoM > max_FoM /*&& effS > 0.5*/) max_FoM = FoM;
	}

	//Get the peak value of the Punzi FoM
	Int_t cut_bin = hist_PFoM->GetMaximumBin(); //FindFirstBinAbove(max_FoM);
	Double_t cut_val = hist_PFoM->GetBinCenter(cut_bin);

	//Draw the Background efficiency on the LHS of the TCanvas and Punzi FoM Histogram on the RHS
	TCanvas *c1 = new TCanvas(method, method, 800, 400);
	c1->Divide(2);
	c1->cd(1);
	hist_effB->Draw();
	hist_effS->SetLineColor(kRed);
	hist_effS->Draw("same");
	c1->cd(2);
	hist_PFoM->Draw();

	//Draw Lines on the histograms to see the max PFoM
	TLine *punzi_vert = new TLine(cut_val, 0, cut_val, max_FoM);
	TLine *punzi_hori = new TLine(min_output*0.001, max_FoM, cut_val, max_FoM);
	punzi_vert->SetLineColor(kGreen); punzi_vert->Draw();
	punzi_hori->SetLineColor(kGreen); punzi_hori->Draw();

	//Draw lines on the efficiency hist to see the efficiencies at the optimum cut value
	TLine *effS_vert = new TLine(cut_val, 0, cut_val, hist_effS->GetBinContent(cut_bin));
	TLine *effS_hori = new TLine(min_output*0.001, hist_effS->GetBinContent(cut_bin), cut_val, hist_effS->GetBinContent(cut_bin));
	effS_vert->SetLineColor(kGreen);
	effS_hori->SetLineColor(kGreen);
	TLine *effB_vert = new TLine(cut_val, 0, cut_val, hist_effB->GetBinContent(cut_bin));
	TLine *effB_hori = new TLine(min_output*0.001, hist_effB->GetBinContent(cut_bin), cut_val, hist_effB->GetBinContent(cut_bin));
	effB_vert->SetLineColor(kGreen);
	effB_hori->SetLineColor(kGreen);
	c1->cd(1);
	effS_vert->Draw(); effS_hori->Draw();
	effB_vert->Draw(); effB_hori->Draw();

	//Output information leanred
	std::cout << "** " << method << " **\n";
	std::cout << "Maximum Punzi Figure of Merit is:        " << max_FoM << std::endl;
	std::cout << "Background efficiency of optimum cut is: " << hist_effB->GetBinContent(cut_bin) << std::endl;
	std::cout << "Signal efficiency of optimum cut is:     " << hist_effS->GetBinContent(cut_bin) << std::endl;
	std::cout << "Ratio of efficiencies is:      Sig/Bkg = " << hist_effS->GetBinContent(cut_bin) / hist_effB->GetBinContent(cut_bin) << std::endl;

	Double_t *output = (Double_t*)malloc(4 * sizeof(Double_t));
	output[0] = max_FoM;
	output[1] = cut_val;
	output[2] = hist_effB->GetBinContent(cut_bin);
	output[3] = hist_effS->GetBinContent(cut_bin);

	tfile->Close();
	return(output);
}

void punzi::GetAllPunzi(TString mvafilename = "RootFiles/TMVA.root",
							   TString bkgfilename = "BMassCut.root", TString bkgtreename = "BkgTree") {

		 punzi::GetPunzi("MLP",mvafilename,bkgfilename,bkgtreename);
		 punzi::GetPunzi("BDTG",mvafilename,bkgfilename,bkgtreename);
		 punzi::GetPunzi("BDT",mvafilename,bkgfilename,bkgtreename);
		 return;
}
