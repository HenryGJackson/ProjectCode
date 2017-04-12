//Converts the name from string to const char*
void Variable::ConvertNames() {
  m_name_Bkg = (m_name + "_Bkg").c_str();
  m_name_MC = (m_name + "_MC").c_str();
  m_name2 = m_name.c_str();
}

//Prints the name of the variable to the command line
void Variable::PrintName() {
  std::cout << "Variable: " << m_name << std::endl;
}

//Get's the Branch of the variable Variable::m_name2 from tree
//Input: TTree *tree 		-The tree to get the TBranch from
//	 Bool_t Data		-kTRUE - Gets the branch from the Background File
//				-kFALSE - Gets the branch from the Monte Carlo File
//Stores the Branches in: Variable::m_Bkg  for  the data file
//			  Variable::m_MC    for  the Monte Carlo file
void Variable::GetBranch(TTree *tree, Bool_t Data) {
  
  if (tree == NULL) {
    std::cerr << "Function 'GetBranch(TTree*, Bool_t)' has been passed a NULL pointer to the tree" << std::endl;
    return;
  }
  if ( Data == kTRUE ) {
    m_Bkg = tree->GetBranch(m_name2);
    tree->SetBranchAddress(m_name2, &m_var_Bkg);
    if( m_Bkg == NULL ) {std::cout << "Background Branch Error: "; PrintName();}
  }
  else {
    m_MC = tree->GetBranch(m_name2);
    tree->SetBranchAddress(m_name2, &m_var_mc);
    if (m_MC == NULL ) {std::cout << "Monte Carlo Branch Error: "; PrintName();}
  }
  
}

//Gets the maximum and minimum data values from the user
void Variable::GetMaxMin() {
  PrintName();
  std::cout << "Minimum Value: "; std::cin >> m_min;
  std::cout << "Maximum Value; "; std::cin >> m_max;
}

//Initialises the histograms 
void Variable::CreateHists () {
  m_hist_Bkg = new TH1D(m_name_Bkg, m_name_Bkg, 100, m_min, m_max);
  m_hist_MC = new TH1D(m_name_MC, m_name_MC, 100, m_min, m_max);
}

//Initialisation of scatterplots
void Variable::CreateScats (Variable *other) {
  string namestr = m_name + " vs " + other->m_name;
  const char *bkg_name = (namestr+"_bkg").c_str();
  const char *mc_name = (namestr+"_mc").c_str();
  m_scat_bkg = new TH2D(bkg_name, bkg_name, 100, m_min, m_max, 100, other->m_min, other->m_max);
  m_scat_bkg->GetXaxis()->SetTitle(m_name.c_str());
  m_scat_bkg->GetYaxis()->SetTitle(other->m_name.c_str());
  m_scat_mc = new TH2D(mc_name, mc_name,  100, m_min, m_max, 100, other->m_min, other->m_max);
  m_scat_mc->GetXaxis()->SetTitle(m_name.c_str());
  m_scat_mc->GetYaxis()->SetTitle(other->m_name.c_str());
}


//Creates a new Histogram titled with the name of the variable. 
//Fills the histogram with both the real data and monte carlo data
//Has 100 bins between Double_t m_min and Double_t m_max.
//Stores the Histograms in m_hist_Bkg for the Background data
//  			   m_hist_MC for the Monte Carlo data
//
//REQUIRES call to Variable::GetBranch() before this function is called
void Variable::FillHist (Bool_t applycut) {
  std::cout << "Function FillHist" << std::endl;
  
  Bent = m_Bkg->GetEntries();
  MCent = m_MC->GetEntries();
  
  
  //find out which of the sets of data has fewer values
  diff = abs(m_Bkg->GetEntries() - m_MC->GetEntries());
  if(m_Bkg->GetEntries() < m_MC->GetEntries()) {
    small_ent = Bent;
    big_ent = MCent;
  }
  else {
    small_ent = MCent;
    big_ent = Bent;
  }
  
  //Fill the histograms 
  for( i=0; i<big_ent; i++ ) {
    if(i<Bent) {
      m_Bkg->GetEntry(i);
      //If no cut is to be applied
      if(applycut == kFALSE) m_hist_Bkg->Fill(m_var_Bkg);
      //Else if the values are to be cut above the cut value
      else if( (applycut == kTRUE)&&(cutup == kTRUE)&&(m_var_Bkg<m_cutVal) )      m_hist_Bkg->Fill(m_var_Bkg);
      //Else if the values are to be cut below the cut value
      else if( (applycut == kTRUE)&&(cutup == kFALSE)&&(m_var_Bkg>m_cutVal) )     m_hist_Bkg->Fill(m_var_Bkg);
      
      
    }
    if(i<MCent) {
      m_MC->GetEntry(i);
      //If no cut is to be applied
      if(applycut == kFALSE) m_hist_MC->Fill(m_var_mc);
      //Else if the values are to be cut above the cut value
      else if( (applycut == kTRUE)&&(cutup == kTRUE)&&(m_var_mc<m_cutVal) )      m_hist_MC->Fill(m_var_mc);
      //Else if the values are to be cut below the cut value
      else if( (applycut == kTRUE)&&(cutup == kFALSE)&&(m_var_mc>m_cutVal) )     m_hist_MC->Fill(m_var_mc);
    }
  }
  std::cout << "Histograms filled" << std::endl;
}

void Variable::ResetHists () {
  m_hist_MC->Reset();
  m_hist_Bkg->Reset();
}

void Variable::ResetScats () {
  m_scat_bkg->Reset();
  m_scat_mc->Reset();
}

//Creates an NTuple to store the values of the variables in
void Variable::CreateNTuple(){
  Bent = m_Bkg->GetEntries();
  MCent = m_MC->GetEntries();
  diff = abs(Bent - MCent);
  if(Bent < MCent) {
    small_ent = Bent;
    big_ent = MCent;
  }
  else {
    small_ent = MCent;
    big_ent = Bent;
  }
  
  for( i=0; i<big_ent; i++ ) {
    if(i<Bent) {
      m_Bkg->GetEntry(i);
      ntup->Fill(m_var_Bkg);
    }
    if(i<MCent) {
      m_MC->GetEntry(i);
      ntup->Fill(m_var_mc);
    }
  }
}


void Variable::WriteHist() {
  //  Double_t var_dat, var_mc;
  TFile myFile("BMESON.root","UPDATE");
  
//  m_Data->SetBranchAddress(m_name2, &var_dat);
  for( i=0; i<m_Bkg->GetEntries(); i++) {
    m_Bkg->GetEntry(i);
    m_hist_Bkg->Fill(m_var_Bkg);
  }

//  m_MC->SetBranchAddress(m_name2, &var_mc);
  for( i=0; i<m_MC->GetEntries(); i++) {
    m_MC->GetEntry(i);
    m_hist_MC->Fill(m_var_mc);
  }
  
  
  m_hist_Bkg->SetMarkerColor(kRed);
  m_hist_Bkg->Write();
  m_hist_MC->Write("same");
}







//Analyses cuts on the data both above and below the cut value
//The cut value varies with the number of increments passed to the function between the minimum and maximum values
//Only "Good Cuts" are recorded, these have ratio of proportion of values cut (MC/Bkg) less than the efficiency passed to the function
//Good Cuts are stored in the file "GoodCuts.txt" which must be initialised using SetupCut() 
//Stores the cut in the TCut Variable::cut
void Variable::AnalyseCuts_TCut (Double_t efficiency, Int_t increments) {
  std::cout << "Function: AnalyseCuts: ";
  PrintName();
  
  TString cutstr;
  
  Double_t val;
  Int_t j;
  Double_t best_cut_up = 0;
  Double_t best_cut_do = 0;
  Double_t best_eff_up = 100;
  Double_t best_eff_do = 100;
  Double_t eff_up = 101;
  Double_t eff_do = 101;
  Int_t MCent = m_MC->GetEntries();
  Int_t Bkgent = m_Bkg->GetEntries();
  Double_t invMCent = 1/MCent;
  Double_t invBkgent = 1/m_Bkg->GetEntries();
  Double_t real_inc = (m_max-m_min)/increments;
  
  for( j=1; j<increments; j++ ) {
    
 //     std::cout << "j = " << j << std::endl;
      MCcountdo = 0;
      MCcountup = 0;
      Bkgcountdo = 0;
      Bkgcountup = 0;
      val = m_min + (j*real_inc);
      std::cout << "j = " << j << "    val = " << val << std::endl;
      std::cout << "inc = " << increments << std::endl;
  //    std::cout << "Loop" << std::endl;
  
    //Count the number of entries higher (countup) and lower (countdo) than the cut value
      for( i=0; i<big_ent; i++) {
	if(i < MCent) {
	  m_MC->GetEntry(i);
	  if( m_var_mc < val ) MCcountdo ++;
	  else MCcountup++;
	}
	if(i < Bent) { 
	  m_Bkg->GetEntry(i);
	  if( m_var_Bkg < val ) Bkgcountdo ++;
	  else Bkgcountup++;
	}
      }
      
      //Calculate the efficiency of the cut
      //Calculated by finding the ratio of the proportion of entries that are cut off, MC/Bkg
      if(Bkgcountup!=0) {
//	std::cout << "Calc eff_up" << std::endl;
//	std::cout << MCcountup << " / " << MCent  << ") / (" << Bkgcountup  << " / " << Bkgent << std::endl;
	MCpropcutup = (MCcountup/MCent);
	Bkgpropcutup = (Bkgcountup/Bkgent);
	eff_up = MCpropcutup/Bkgpropcutup;
//	std::cout << "Calc eff_up check" << std::endl;
	if(eff_up < best_eff_up) {
	  best_eff_up = eff_up;
	  best_cut_up = val;
	}
      }
      if(Bkgcountdo!=0) {
//	std::cout << "Calc eff_do" << std::endl;
	MCpropcutdo = MCcountdo/MCent;
	Bkgpropcutdo = Bkgcountdo/Bkgent;
	
	eff_do = MCpropcutdo/Bkgpropcutdo;
	if(eff_do < best_eff_do) {
	  best_eff_do = eff_do;
	  best_cut_do = val;
	  
	}
	
      }
      
  }
  if( (best_eff_do < efficiency) || (best_eff_up < efficiency) ) goodCut = kTRUE;
  else goodCut = kFALSE;
    
  
  //Set the value of cutup, kTRUE means higher values than the cut value will be cut from the data
  //			    kFALSE means lower values than the cut value will be cut from the data
  if(goodCut == kTRUE) {
    ofstream file;
    file.open("/home/epp/phunch/Documents/VariableInOut/GoodCuts.txt", std::ofstream::app);
    file << "*****************************\n";
    file << m_name << "\n";
    TCut *cut_temp = new TCut();;
    
    if(best_eff_do < best_eff_up) {
      m_cutVal = best_cut_do;
      cutstr = "x<" + to_string(m_cutVal);
      cut_temp->SetTitle(cutstr);
      std::cout << "Cut: " << cut_temp->GetTitle() << std::endl;
      file << cutstr << "\n";
	  
    }
    else {
      m_cutVal = best_cut_up;
      cutstr = "x>" + to_string(m_cutVal);
      cut_temp->SetTitle(cutstr);
      std::cout << "Cut: " << cut_temp->GetTitle() << std::endl;
      file << cutstr << "\n";
         
    }
    file.close();
    cut = cut_temp;
  }
  
  std::cout << "AnalyseCuts Complete" << std::endl;
  
}

//Analyses cuts on the data both above and below the cut value
//The cut value varies with the number of increments passed to the function between the minimum and maximum values
//Only "Good Cuts" are recorded, these have ratio of proportion of values cut (MC/Bkg) less than the efficiency passed to the function
//Good Cuts are stored in the file "GoodCuts.txt" which must be initialised using SetupCut() 
//Fills a new TBranch with only the entries after the cut
void Variable::AnalyseCuts_vals (Double_t efficiency, Int_t increments) {
  std::cout << "Function: AnalyseCuts: ";
  PrintName();
  
  Double_t val;
  Int_t j;
  Double_t best_cut_up = 0;
  Double_t best_cut_do = 0;
  Double_t best_eff_up = 100;
  Double_t best_eff_do = 100;
  Double_t eff_up = 101;
  Double_t eff_do = 101;
  Int_t MCent = m_MC->GetEntries();
  Int_t Bkgent = m_Bkg->GetEntries();
  Int_t big_ents = MCent;
  Double_t invMCent = 1/MCent;
  Double_t invBkgent = 1/m_Bkg->GetEntries();
  Double_t real_inc = (m_max-m_min)/increments;
  std::cout << "Bkg Entries:	" << Bkgent << std::endl;
  std::cout << "MC Entries:	" << MCent << std::endl;
  effScatUp = new TGraph();
  effScatDo = new TGraph();
      
  if( MCent < Bkgent ) big_ents = Bkgent;
  
  for( j=1; j<increments; j++ ) {
    
 //     std::cout << "j = " << j << std::endl;
      MCcountdo = 0;
      MCcountup = 0;
      Bkgcountdo = 0;
      Bkgcountup = 0;
      val = m_min + (j*real_inc);
  //    std::cout << "j = " << j << "    val = " << val << std::endl;
  //    std::cout << "inc = " << increments << std::endl;
  //    std::cout << "Loop" << std::endl;
      
  
    //Count the number of entries higher (countup) and lower (countdo) than the cut value
      for( i=0; i<big_ents; i++) {
        if(i < MCent) {
          m_MC->GetEntry(i);
          if( m_var_mc < val ) MCcountdo ++;
          else MCcountup++;
        }
        if(i < Bkgent) {
          m_Bkg->GetEntry(i);
          if( m_var_Bkg < val ) Bkgcountdo ++;
          else Bkgcountup++;
        }
      }
      
      //Calculate the efficiency of the cut
      //Calculated by finding the ratio of the proportion of entries that are cut off, MC/Bkg
      if(Bkgcountup!=0&&MCcountup!=0) {
    //	std::cout << "Calc eff_up" << std::endl;
    //	std::cout << MCcountup << " / " << MCent  << ") / (" << Bkgcountup  << " / " << Bkgent << std::endl;
        MCpropcutup = (MCcountup/MCent);
        Bkgpropcutup = (Bkgcountup/Bkgent);
        eff_up = MCpropcutup/Bkgpropcutup;
        effScatUp->SetPoint(j-1,MCpropcutup,1-Bkgpropcutup);
        effScatUp->GetXaxis()->SetTitle("Monte Carlo Proportion Cut");
        effScatUp->GetYaxis()->SetTitle("1 - Background Proportion Cut");
    //	std::cout << "eff_up " << eff_up << " = " << MCpropcutup << " / " << Bkgpropcutup << std::endl;
    //	std::cout << "Calc eff_up check" << std::endl;
        if(eff_up < best_eff_up) {
          best_eff_up = eff_up;
          best_cut_up = val;
        }
      }
      else {std::cout << "cut up error" << std::endl;}

      if(Bkgcountdo!=0&&MCcountdo!=0) {
        MCpropcutdo = MCcountdo/MCent;
        Bkgpropcutdo = Bkgcountdo/Bkgent;
        effScatDo->SetPoint(j-1,MCpropcutdo,1-Bkgpropcutdo);
        effScatDo->GetXaxis()->SetTitle("Monte Carlo Proportion Cut");
        effScatDo->GetYaxis()->SetTitle("1 - Background Proportion Cut");
        eff_do = MCpropcutdo/Bkgpropcutdo;
        //	std::cout << "eff_do " << eff_do << " = " << MCpropcutdo << " / " << Bkgpropcutdo << std::endl;
        if(eff_do < best_eff_do) {
          best_eff_do = eff_do;
          best_cut_do = val;

        }
	
      }
      else {std::cout << "cut down error" << std::endl;}
  } // j loop
  if( best_eff_do < best_eff_up ) {cutup = kFALSE; m_cutVal = best_cut_do; cut_eff = best_eff_do;}
  else {cutup = kTRUE; m_cutVal = best_cut_up; cut_eff = best_eff_up;}
  
  if( cut_eff < efficiency ) goodCut = kTRUE;
  else goodCut = kFALSE;
  
  //Store the value of the cut and whether its effective in the file "GoodCuts.txt"
  ofstream file;
  file.open("/home/epp/phunch/Documents/VariableInOut/GoodCuts.txt", std::ofstream::app);
  file << "*****************************\n";
  file << m_name << "\n";
  file << m_cutVal << "\n";
  if (cutup == kTRUE) file << "TRUE\n";
  else file << "FALSE\n";
  file << cut_eff << "\n";
  
  if (goodCut == kFALSE) std::cout << "No Good Cuts" << std::endl;
}



//Performs a cut on the variables and refills the histograms.
//Requires a call to AnalyseCuts() before this function is called (fromFile == kFALSE)
//Or requires that the file "GoodCuts.txt" exists
void Variable::GetCut () {
  
  string line;
  ifstream file;
  file.open("/home/epp/phunch/Documents/VariableInOut/GoodCuts.txt", std::ofstream::in);
  if(file) {
    
    while(line != m_name) getline(file,line);
    getline(file,line);
    m_cutVal = std::stod(line);
    getline(file,line);
    if( line == "TRUE") cutup = kTRUE;
    else cutup = kFALSE;
  }
}

//Gets the cut information from the file GoodCuts.txt but sets the value of 
//   cutup to the opposite as what is written in the file
void Variable::GetReverseCut() {
    string line;
  ifstream file;
  file.open("/home/epp/phunch/Documents/VariableInOut/GoodCuts.txt", std::ofstream::in);
  if(file) {
    
    while(line != m_name) getline(file,line);
    getline(file,line);
    m_cutVal = std::stod(line);
    getline(file,line);
    if( line == "TRUE") cutup = kFALSE;
    else cutup = kTRUE;
  }
}
  
//Gets the entry lists according to the cut stored in Variable::m_cutVal and
//   Variable::cutup
void Variable::SeteLists(TTree *MonteC, TTree *BackG) {
    
    //Get the right names in the right format (const char *) for the functions
    string strMC = m_name + "eListMC";
    string forw_strMC = ">>" + strMC;
    string strBKG = m_name + "eListBKG";
    string forw_strBKG = ">>" + strBKG;
    string val_str = std::to_string(m_cutVal);
    string cut_str;
    if(cutup == kTRUE) cut_str = m_name + "<" + val_str;
    else cut_str = m_name + ">" + val_str;
    
    //Get the entry lists for each tree and create an object for each
    MonteC->Draw(forw_strMC.c_str(),cut_str.c_str(),"entrylist");
    BackG->Draw(forw_strBKG.c_str(),cut_str.c_str(),"entrylist");
    
    //Set the entry list pointers to the objects created
    gDirectory->GetObject(strMC.c_str(),MCeList);
    gDirectory->GetObject(strBKG.c_str(),BKGeList);
}
