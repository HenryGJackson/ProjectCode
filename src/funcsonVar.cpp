//Fills the scatterplots and stores them in Variable::x::m_scat_...
void FillScats (Variable *x, Variable *other, Bool_t applycut = kFALSE) {
  std::cout << "Function FillScat" << std::endl;
  x->Bent = x->m_Bkg->GetEntries();
  x->MCent = x->m_MC->GetEntries();
  Int_t i;
  //find out which of the sets of data has fewer values
  x->diff = abs(x->Bent - x->MCent);
  if(x->Bent < x->MCent) {
    x->small_ent = x->Bent;
    x->big_ent = x->MCent;
  }
  else {
    x->small_ent = x->MCent;
    x->big_ent = x->Bent;
  }
  
  //Fill the scatter plots where cuts are applied if applycut = kTRUE
  for( i=0; i<x->big_ent; i++ ) {
    if(i<x->Bent) {
      x->m_Bkg->GetEntry(i);
      other->m_Bkg->GetEntry(i);
      
      if(applycut == kFALSE) x->m_scat_bkg->Fill(x->m_var_Bkg, other->m_var_Bkg);
      else if( (applycut == kTRUE)&&(x->cutup == kTRUE)&&(x->m_var_Bkg<x->m_cutVal)&&(other->cutup == kTRUE)&&(other->m_var_Bkg<other->m_cutVal) ) {
	x->m_scat_bkg->Fill(x->m_var_Bkg,other->m_var_Bkg);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kTRUE)&&(x->m_var_Bkg<x->m_cutVal)&&(other->cutup == kFALSE)&&(other->m_var_Bkg>other->m_cutVal) ) {
	x->m_scat_bkg->Fill(x->m_var_Bkg,other->m_var_Bkg);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kFALSE)&&(x->m_var_Bkg>x->m_cutVal)&&(other->cutup == kTRUE)&&(other->m_var_Bkg<other->m_cutVal) ) { 
	x->m_scat_bkg->Fill(x->m_var_Bkg,other->m_var_Bkg);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kFALSE)&&(x->m_var_Bkg>x->m_cutVal)&&(other->cutup == kFALSE)&&(other->m_var_Bkg>other->m_cutVal) ) { 
	x->m_scat_bkg->Fill(x->m_var_Bkg,other->m_var_Bkg);
      }
    }
    if(i<x->MCent) {
      x->m_MC->GetEntry(i);
      other->m_MC->GetEntry(i);
      if(applycut == kFALSE) x->m_scat_mc->Fill(x->m_var_mc, other->m_var_mc);
      else if( (applycut == kTRUE)&&(x->cutup == kTRUE)&&(x->m_var_mc<x->m_cutVal)&&(other->cutup == kTRUE)&&(other->m_var_mc<other->m_cutVal) ) {
	x->m_scat_mc->Fill(x->m_var_mc,other->m_var_mc);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kTRUE)&&(x->m_var_mc<x->m_cutVal)&&(other->cutup == kFALSE)&&(other->m_var_mc>other->m_cutVal) ) {
	x->m_scat_mc->Fill(x->m_var_mc,other->m_var_mc);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kFALSE)&&(x->m_var_mc>x->m_cutVal)&&(other->cutup == kTRUE)&&(other->m_var_mc<other->m_cutVal) ) { 
	x->m_scat_mc->Fill(x->m_var_mc,other->m_var_mc);
      }
      else if( (applycut == kTRUE)&&(x->cutup == kFALSE)&&(x->m_var_mc>x->m_cutVal)&&(other->cutup == kFALSE)&&(other->m_var_mc>other->m_cutVal) ) { 
	x->m_scat_mc->Fill(x->m_var_mc,other->m_var_mc);
      }
    }
  }
  
}

//Sets up the file "GoodCuts.txt" ready for the call to AnalyseCuts().
//if this function is not called prior to AnalyseCuts() then the cut data will be added to that of the previous run.
void SetupCut() {
    ofstream file;
    file.open("/home/epp/phunch/Documents/VariableInOut/GoodCuts.txt", std::ofstream::out | std::ofstream::trunc);
    file.close();
}

//Gets the names of the variables to be analysed from the File: "variable_names.txt"
//"variable_names.txt" must have each variable name on a new line with the minimum then 
//	maximum values all seperated by a space.
Variable *GetVariables() {
    
    Int_t listnum = 1;
    
    //Get the TTrees from the root files for the background and monte carlo data
    TFile *fMC = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_Signal_MC_MD.root");
    TTreeReader mcReader("KMuE/DecayTree", fMC);
    TTree *DTmc = mcReader.GetTree();
    if (fMC == NULL) std::cerr << "Error Accessing BuKMuE_Signal_MC_MD.root" << std::endl;
    TFile *fBkg = TFile::Open("/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root");
    TTreeReader sigReader("KMuE/DecayTree",fBkg);
    TTree *DTBkg = sigReader.GetTree();
    if (fBkg == NULL) std::cerr << "Error Accessing BuKMuE_2011_MgUp.root" << std::endl;

    //Create two pointers to Variable objects, seed to point to the head of the 
    //   list and temp to point to each new Variable created.
    Variable *seed = new Variable;
    Variable *temp = NULL;

    string var_name;      //The name of the variable
    Double_t num;         //temporary storage for the max and min values

    //Open two instances of the file to read the variables from
    std::ifstream file("/home/epp/phunch/Documents/VariableInOut/variable_names.txt");
    std::ifstream file2("/home/epp/phunch/Documents/VariableInOut/variable_names.txt");

    if(file) {
      for (std::string line; getline(file,line); ) {
          //Get the Variable name from the file
        file2 >> var_name;
        if(var_name == "--STOP--") break;
        
        //if list is not empty add a new entry to the end of the list
        if(temp != NULL) {
          temp->m_next = new Variable;
          temp = temp->m_next;
        }
        //else if list is empty create a new one.
        else temp = seed;

        //Set the number in the list 
        temp->nInList = listnum;
        listnum++;
        
        //Ensure this new variable is the last in the list
        temp->m_next = NULL;

        temp->m_name = var_name;
        temp->ConvertNames();

        //Get the maximum and minimum values for the variable
        file2 >> num;
        temp->m_min = num;
        file2 >> num;
        temp->m_max = num;

        //Get the TBranches for this variable from the TTrees
        temp->GetBranch(DTBkg, kTRUE);
        if ( temp->m_Bkg == NULL) {
          std::cerr << "Couldn't access Background Branch" << std::endl;
          return NULL;
        }
        temp->GetBranch(DTmc, kFALSE);
        if ( temp->m_MC == NULL) {
          std::cerr << "Couldn't access Monte Carlo Branch" << std::endl;
          return NULL;
        }
      }
    }

    //Close the files used for reading the variable information from.
    file.close();
    file2.close();
    
    if(seed == NULL) {
        std::cerr << "ERROR: LIST EMPTY" << std::endl;
        return NULL;
    }

    //Return the head of the list
    return seed;
}

//Set the TEntryLists for all variables and their cuts
void SeteListAllVars (Variable *seed, TTree *MonteC, TTree *BackG) {
    //Create pointers to go through the list
    Variable *current = seed;
    
    while(current) {
        //Get the correct cut for the first variable and for the rest get the 
        //   reverse cut so that the entry lists will be filled with the entries
        //   to be cut.
        current->PrintName();
        if(current->nInList == 1) current->GetCut();
        else current->GetReverseCut();
        
        //Get the entry lists for MC and BKG data from TTrees
        current->SeteLists(MonteC,BackG);

        //Go to next Variable in the list
        current = current->m_next;
    }
}

//Requires that only the head Variable has an Entry List of entries to keep.
//The rest should have an Entry List of entries to cut which will subsequently
//   be subtracted from the list of events to keep on the head variable.
//If MCorBKG == kTRUE then func fills a TEntryList for Monte Carlo Data
//If MCorBKG == kFALSE then func fills a TEntryList for Background Data
TEntryList *GeteListAllVars (Variable *seed, Bool_t MCorBKG) {
    
    //Create an Entry List and choose the destination to get the initial list from
    TEntryList *elist;
    if(MCorBKG == kTRUE) elist = seed->MCeList; //Get the Monte Carlo list
    else elist = seed->BKGeList;                //Get the Background list
    
    //Create a Variable pointer to go through the list
    Variable *current = seed->m_next;
    
    while(current) {
        if(MCorBKG == kTRUE) elist->Subtract(current->MCeList);
        else elist->Subtract(current->BKGeList);
        current = current->m_next;
    }
    return elist;
}


void DisableBranches(TTree *MonteC, TTree *BackG, Variable *seed) {
    Variable *current = seed;
    
    //Disable all branches for both trees
    MonteC->SetBranchStatus("*",0);
    BackG->SetBranchStatus("*",0);
    
    //Enable the mass branches
    MonteC->SetBranchStatus("B_M",1);
    BackG->SetBranchStatus("B_M",1);
    
    while(current) {
        //Enable the branches for each of the variables in the list
        MonteC->SetBranchStatus(current->m_name2,1);
        BackG->SetBranchStatus(current->m_name2,1);
        current = current->m_next;
    }
}

//Delete the linked list with seed as the head of the list.
void DeleteList(Variable *seed) {
    Variable *current = seed;
    Variable *temp;
    while(current) {
        temp = current;
        current = current->m_next;
        delete(temp);
    }
}
