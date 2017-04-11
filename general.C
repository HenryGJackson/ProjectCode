
//NOTE CUT NAMESPACE
		//Copy a subset of the tree with the selection cutstr
		TString cut::copySubset(const TString cutstr = "B_PT > 10000", const TString treename = "KMuE/DecayTree",
		                      const TString path = "", const TString filename = "BuKMuE_2011_MgUp.root", TString nfname = ""){

		  TFile* tfile = TFile::Open( path + filename );
		  TTree* ttree = (TTree*) tfile->Get( treename );

		  TEventList events("event_list","events");
		  ttree->Draw(">>event_list",cutstr);

		  TFile* newfile = new TFile(path + nfname,"RECREATE");
		  TTree* newtree = ttree->CloneTree(0);

		  if(events.GetN()) {
			  for ( int i = 0; i < events.GetN(); ++i ){
			    ttree->GetEntry( events.GetEntry(i) );
			    newtree->Fill() ;
			  }
		  }
		  else {std::cout << "No Events Left After Cuts\nExiting...\n\n"; return("EXIT");}
		  tfile->Close();

		  if(newtree->GetEntries() < 1) {std::cout << "Error filling tree\nExiting...\n\n"; return("EXIT");}
		  newtree->Write();
		  newfile->Close();

		  return(nfname) ;
		}

		//Cut the B Mass above the value passed in
		void cut::CutDoubleVar(TString varname = "B_M", TString qualifier = ">", Double_t value = 5700.0,
		  TString outfilename = "~/Project/TMVA/rootoutput/withcuts/AllDataCombined_cut",
			TString filename = "~/Project/TMVA/rootoutput/withcuts/AllDataCombined_afterCuts.root",
			TString treename = "DecayTree") {

			outfilename += varname + ".root";
		  	Bool_t overwrite = kTRUE;
			TString newname = treename;
			TString fileoption;
			if (overwrite) fileoption = "RECREATE";
			else		   fileoption = "UPDATE";

			//Get Tree from file
			TFile *file = TFile::Open(filename);
			if(!file) {std::cout << "Could not load " << filename << std::endl; return;}
			TTreeReader Reader(treename, file);
			TTree *ttree = Reader.GetTree();
			if(!ttree) {std::cout << "Could not ttree " << treename << std::endl; return;}

			//Create B Mass Variable object
			Variable* BMass = new Variable;
			BMass->SetName(varname);
			BMass->SetCutVal(value);
			if(qualifier == ">") 	BMass->SetCutDir(1);
			else				BMass->SetCutDir(0);
			BMass->SetNext(NULL);


			//Open new file and save new tree with cuts to file
			TFile* newfile = new TFile(outfilename, fileoption);
			TTree* newtree = cut::MakeCuts(BMass, ttree, newfile, newname);

			//Write tree and close files
			file->Close();
			newtree->Write();
			newfile->Close();

			delete BMass;
			return;
		}

		//Cut Variable "E_HasBremAdded == kFALSE"
		void cut::CutBoolVar(TString varname = "E_HasBremAdded", TString value = kFALSE, TString treename = "DecayTree",
		                     TString ifname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllDataCombined_afterCuts.root",
		                     TString ofname = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/AllData_HasBremTRUE.root")
		{

		  TFile* file = TFile::Open(ifname);
			TTree* tree = (TTree*)file->Get(treename);

			tree->SetBranchStatus("*",1);
			Bool_t tmp;
			tree->SetBranchAddress("E_HasBremAdded",&tmp);

			TFile* ofile = new TFile(ofname, "RECREATE");
			TTree* ntree = tree->CloneTree(0);

			for(Int_t i = 0; i < tree->GetEntries(); i++) {
				tree->GetEntry(i);
				if(tmp == value) {
					ntree->Fill();
				}
			}

			file->Close();
			ntree->Write();
			ofile->Close();
			return;
		}

		void cut::CutIntVar(TString varname = "B_BKGCAT", TString tn = "DecayTree",
		                    TString fn = "AllDataCombined_afterCuts",
					  TString path = "/home/henry/Project/rootcode/TMVA/rootoutput/withcuts/") {
		    TString newfn = path + fn + "_BKGCAT10_50.root";
		    fn = path + fn + ".root";

		    TFile* file = TFile::Open(fn);
		    if(!file) {std::cout << "[error] Couldn't load input file\n"; return;}
		    TTree* tree = (TTree*)file->Get(tn);
		    if(!tree) {std::cout << "[error] Couldn't load input tree\n"; return;}

		    TFile* nfile = new TFile(newfn,"RECREATE");
		    TTree* ntree = tree->CloneTree(0);

		    Int_t value;
		    tree->SetBranchAddress(varname,&value);

		    Int_t i;
		    for(i = 0; i < tree->GetEntries(); i++) {
		        tree->GetEntry(i);
		        if(value == 10 || value == 50) {
		            ntree->Fill();
		        }
		        std::cout << "\r" << "Make Cuts Progrss: " << std::setw(4) << i * 100 / (tree->GetEntries() - 1) << " %." << std::flush;
		    }
		    std::cout << std::endl;

		    file->Close();
		    nfile->cd();
		    ntree->Write();
		    nfile->Close();

		    return;
		}

		TTree* cut::MakeCuts(Variable* seed, TTree* ttree, TFile *newfile, TString newtreename = "DecayTree") {

			//Clone Empty Tree
			newfile->cd();
			TTree* newtree = ttree->CloneTree(0);
			newtree->SetName(newtreename);

			Variable* temp = seed;
			Int_t variable_count = 0;
			TCut cuts;

			//Loop through variables setting up the cut then adding it to the cuts to be applied
			while (temp) {
				temp->SetCut();
				cuts = cuts + temp->GetCut();
				temp = temp->GetNext();
			}

			//Get the event list with the cuts
			TEventList events("event_list", "events");
			ttree->Draw(">>event_list", cuts);
			Int_t ent = events.GetN();
			if(ent) {
				//Fill the new tree with the cuts applied
				for (int i = 0; i < events.GetN(); ++i) {
					ttree->GetEntry(events.GetEntry(i));
					newtree->Fill();
					std::cout << "\r" << "Make Cuts Progrss: " << std::setw(4) << i * 100 / (events.GetN() - 1) << " %." << std::flush;
				}
				std::cout << "\n";
				newfile->cd();
				newtree->Write();
			}
			else {std::cout << "Tree has 0 entries after cuts\nExiting...\n\n"; return(NULL);}
			return(newtree);
		}

		//Creates a list of Variable objects which can be used with cut::MakeCuts().
		Variable* cut::BookCuts(TString svars = "B_M,B_PT",  TString squalifiers = ">,>",
		                    TString svalues = "5700.0,100.0", TString stypes = "double,double") {

		    std::cout << "cut::BookCuts: Progress: 0%";
		    TObjArray* vars = svars.Tokenize(",");
		    if(!vars) {std::cout << "Couldn't Tokenize svars\nExiting...\n\n"; return(NULL);}
		    TObjArray* types = stypes.Tokenize(",");
		    if(!types) {std::cout << "Couldn't Tokenize svars\nExiting...\n\n"; return(NULL);}
		    TObjArray* values = svalues.Tokenize(",");
		    if(!values) {std::cout << "Couldn't Tokenize svars\nExiting...\n\n"; return(NULL);}
		    TObjArray* qualifiers = squalifiers.Tokenize(",");
		    if(!qualifiers) {std::cout << "Couldn't Tokenize svars\nExiting...\n\n"; return(NULL);}
		    if(vars->GetEntries() != types->GetEntries() || vars->GetEntries() != values->GetEntries() || vars->GetEntries() != qualifiers->GetEntries()) {
		        std::cout << "[ERROR] strings passed to cut::BookCuts() not of equal length\nExiting...\n\n";
		        return(NULL);
		    }

		    std::cout << "\rcut::BookCuts: Progress: 10%" << std::flush;
		    TObjString* var = (TObjString*)vars->First();
		    TObjString* type = (TObjString*)types->First();
		    TObjString* value = (TObjString*)values->First();
		    TObjString* qualifier = (TObjString*)qualifiers->First();

		    Variable* thisvar = new Variable();
		    thisvar->SetName(var->GetString());
		    thisvar->SetCutVal((value->GetString()).Atof());
		    thisvar->SetType(type->GetString());
		    if     (qualifier->GetString() == "<" || qualifier->GetString() == "==") thisvar->SetCutDir(1);
		    else if(qualifier->GetString() == ">" || qualifier->GetString() == "!=") thisvar->SetCutDir(0);
		    else {std::cout << "Qualifier error in cut::BookCuts()\nExiting...\n"; return(NULL);}
		    Variable* seed = thisvar;

		    if(vars->GetEntries() > 1) {
		        var = (TObjString*)vars->After(var);
		        type = (TObjString*)types->After(type);
		        value = (TObjString*)values->After(value);
		        qualifier = (TObjString*)qualifiers->After(qualifier);

		        std::cout << "\rcut::BookCuts: Progress: 30%" << std::flush;
		        Int_t i = 1;
		        while(var) {
		            thisvar = new Variable();
		            thisvar->SetNext(seed);
		            thisvar->SetName(var->GetString());
		            thisvar->SetCutVal((value->GetString()).Atof());
		            thisvar->SetType(type->GetString());
				if     (qualifier->GetString() == "<" || qualifier->GetString() == "==") thisvar->SetCutDir(1);
	  		      else if(qualifier->GetString() == ">" || qualifier->GetString() == "!=") thisvar->SetCutDir(0);
	  		      else {std::cout << "Qualifier error in cut::BookCuts()\nExiting...\n"; return(NULL);}
		            seed = thisvar;

		            var = (TObjString*)vars->After(var);
		            type = (TObjString*)types->After(type);
		            value = (TObjString*)values->After(value);
		            qualifier = (TObjString*)qualifiers->After(qualifier);
		            std::cout << "\rcut::BookCuts: Progress: " << i * 70/(vars->GetEntries()-1) << "%" << std::flush;
		        }
		        std::cout << std::endl;
		    }
		    return(seed);
		}


//NOTE: END OF CUT NAMESPACE



//NOTE: START OF GENERAL NAMESPACE
		TTree* general::GetTree(Bool_t GetBkg) {
			if (GetBkg) {
				TFile* infile = TFile::Open("BuKMuE_2011_MgUp.root");
				if (!infile) return(NULL);
				TTree* intree = (TTree*)infile->Get("KMuE/DecayTree");
				if (!intree) return(NULL);
				return(intree);
			}
			else {
				TFile* infile = TFile::Open("BuKMuE_Signal_MC_MD.root");
				if (!infile) return(NULL);
				TTree* intree = (TTree*)infile->Get("KMuE/DecayTree");
				if (!intree) return(NULL);
				return(intree);
			}
		}

		TEntryList* general::GetEList(TTree* tree, Bool_t GetBkg) {
			if (GetBkg) {
				TEntryList* list = new TEntryList("BkgList", "List of Background entries to keep", tree);
				for (int i = 0; i < tree->GetEntries(); i++) {
					list->Enter(i, tree);
				}
				if (list->GetN() == 0) return(NULL);
				return(list);
			}
			else {
				TEntryList* list = new TEntryList("MCList", "List of Monte Carlo entries to keep", tree);
				for (int i = 0; i < tree->GetEntries(); i++) {
					list->Enter(i, tree);
				}
				if (list->GetN() == 0) return(NULL);
				return(list);
			}
		}

		Variable* general::CreateVarList(TTree* BkgTree) {
			//Open two instances of the file to read the variables from
			TString filestr = "/home/henry/Project/rootcode/TMVA/cutinfo/NamesAndCuts.txt";
			std::ifstream file(filestr);
			std::ifstream file2(filestr);

			//Create two pointers to Variable objects, seed to point to the head of the
			//   list and temp to point to each new Variable created.
			Variable *seed = new Variable;
			Variable *temp = NULL;
			TString  name_tmp;
			TString type;
			Double_t min;
			Double_t max;
			Double_t cut_tmp;
			Int_t	 cut_int;
			Int_t    data_int;

			std::string line; getline(file, line);
			Int_t vars = atoi(line.c_str());
			if (vars == 0) return(NULL);

			file2.seekg(2, ios::beg);
			for (Int_t i = 0; i < vars; i++) {

				//if list is not empty add a new entry to the end of the list
				if (temp != NULL) {
					temp->SetNext(new Variable);
					temp = temp->GetNext();
				}
				//else if list is empty create a new one.
				else temp = seed;
				temp->NotDerived();
				//Get the Variable name from the file
				getline(file, line);
				name_tmp = line;
				temp->SetName(name_tmp);
				//		std::cout << "Variable Name: " << temp->GetName() << std::endl;

				//Set the limits of this variable
				getline(file, line);
				min = atof(line.c_str());
				getline(file, line);
				max = atof(line.c_str());
				temp->SetLimits(min, max);
				//		std::cout << "Limits: " << temp->GetMin() << " - " << temp->GetMax() << std::endl;

				//Set the cut value of thhis variable
				getline(file, line);
				cut_tmp = atof(line.c_str());
				temp->SetCutVal(cut_tmp);
				//		std::cout << "Cut Value: " << temp->GetCutVal() << std::endl;

				//Tell whether to cut above or below the cut value
				getline(file, line);
				cut_int = atoi(line.c_str());
				temp->SetCutDir(cut_int);
				//		std::cout << "The Cut is Less than the Cut Value: " << temp->GetCutDir() << std::endl;

				getline(file, line);
				data_int = atoi(line.c_str());
				temp->SetHasMC(data_int);

				getline(file, line);
				temp->SetType(line);
				if(temp->GetType() == "double") BkgTree->SetBranchAddress(temp->GetName(), &temp->bkg_value);
				else if(temp->GetType() == "bool") BkgTree->SetBranchAddress(temp->GetName(), &temp->bool_value);
				else BkgTree->SetBranchAddress(temp->GetName(), &temp->mc_value);
				//Set the branch status and address for this variable
				//	BkgTree->SetBranchStatus(temp->GetName(), 1);
				//	if (temp->HasMC()) MCTree->SetBranchStatus(temp->GetName(), 1);
				//	if (temp->HasMC()) MCTree->SetBranchAddress(temp->GetName(), &temp->mc_value);
			}

			file.close();
			file2.close();
			return(seed);
		}

		void general::DeleteList(Variable* seed) {
				Variable* tmp;
				Variable* cur = seed;
				while(cur) {
					tmp = cur->GetNext();
					free(cur);
					cur = tmp;
				}
				return;
		}

		void general::TestDeleteList() {
			TFile* tfile = TFile::Open("TreesWithMVAResponse.root");
			TTree* ttree = (TTree*)tfile->Get("BkgTree");
			Variable* seed = general::CreateVarList(ttree);
			DeleteList(seed);
		}





		void general::DrawHist(TBranch* branch, Int_t min, Int_t max) {
			TCanvas* canv = new TCanvas("canv", "canv", 400, 400);
			TH1D* hist = new TH1D("hist", "hist", 100, min, max);
			Double_t value;
			branch->SetAddress(&value);
			for (Int_t i = 0; i < branch->GetEntries(); i++) {
				branch->GetEntry(i);
				hist->Fill(value);
			}
			canv->cd();
			hist->Draw();
			return;
		}

		void general::DrawBMass(Double_t min = 4400, Double_t max = 6200, TString filename = "NewTreesWithCuts", const TString treename = "DecayTree") {
			TCanvas* canv = new TCanvas(filename, filename + " B Mass", 600, 600);
			TH1D* hist = new TH1D(filename, filename + " B Mass", 100, min, max);
			if(!canv || !hist) {std::cout << "Error creating objects\n"; return;}

			filename = filename + ".root";

			TFile* tfile = TFile::Open(filename);
			if(!tfile) {std::cout << "Could not load: " << filename << std::endl; return;}
			TTree* ttree = (TTree*)tfile->Get(treename);
			if(!ttree) {std::cout << "Could not load: " << treename << std::endl; return;}

			ttree->SetBranchStatus("*", 0);
			ttree->SetBranchStatus("B_M", 1);
			Double_t value;
			ttree->SetBranchAddress("B_M", &value);

			for (Int_t i = 0; i < ttree->GetEntries(); i++) {
				ttree->GetEntry(i);
				hist->Fill(value);
			}

			canv->cd();
			hist->Draw();
			tfile->Close();
			return;
		}

		Int_t general::CheckBranch(TBranch *branch, Double_t min, Double_t max) {
			Double_t value;
			branch->SetAddress(&value);
			Int_t high = 0;
			Int_t low = 0;
			for (Int_t i = 0; i < branch->GetEntries(); i++) {
				branch->GetEntry(i);
				if (value > max) high++;
				else if (value < min) low++;
			}
			if (high != 0 || low != 0) {
				std::cout << "Branch has Value Error with: " << low << " below the minimum " << min << std::endl;
				std::cout << "                             " << high << " above the maximum " << max << std::endl;
				return(1);
			}
			else return(0);
		}

		Int_t general::CheckBranchFromFile(const TString branch = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
			TFile* tfile = TFile::Open(filename);
			TTree* ttree = (TTree*)tfile->Get(treename);
			Float_t value;
			ttree->SetBranchStatus("*", 0);
			ttree->SetBranchStatus(branch, 1);
			ttree->SetBranchAddress(branch, &value);
			Float_t min = 0.0;
			Float_t max = 1.0;
			Int_t high = 0;
			Int_t low = 0;
			for (Int_t i = 0; i < ttree->GetEntries(); i++) {
				ttree->GetEntry(i);
				if (value > max) high++;
				else if (value < min) low++;
			}
			if (high != 0 || low != 0) {
				std::cout << "Branch has Value Error with: " << low << " below the minimum " << min << std::endl;
				std::cout << "                             " << high << " above the maximum " << max << std::endl;
				return(1);
			}
			else return(0);
		}

		void general::NewTree(const TString fileoption = "UPDATE", const TString treename = "KMuE/DecayTree", const TString filename = "/home/henry/Documents/Project/BuKMuE_Signal_MC_MD.root",
							   const TString newfilename = "NewTreesWithCuts.root", const TString newtreename = "BkgTree"){



			TFile* tfile = TFile::Open(filename);
			TTree* ttree = (TTree*)tfile->Get(treename);

			//Create List of Variables to Make cuts on
			std::cout << "\r" << "Creating Var List" << std::flush;
			Variable* seed = general::CreateVarList(ttree);						//Class from:   Variable.h
			if (seed == NULL) {
				std::cout << "Variable List is Empty\n";
				return;
			}

			TFile* newfile = new TFile(newfilename, fileoption);
			//Get clone of tree with cuts on non-derived variables
			std::cout << "\r" << "Making Cuts                 " << std::flush;
			TTree* newtree = cut::MakeCuts(seed, ttree, newfile, newtreename);			//Function from:   GeneralFunctions.C
			std::cout << "Efficiency after Cuts: " << (Double_t)newtree->GetEntries() / ttree->GetEntries() << std::endl;

		//	DeleteList(seed);
		/*	tfile->Close();
			newtree->Write();
			newfile->Close();

			return;*/

			//Setup List of Derived Variables
			Variable* aKMu = new Variable;
			aKMu->DerivedSetup("aKMu", 0.0005, 1, 0);

			Variable* aKe = new Variable;
			aKe->DerivedSetup("aKe", 0.0005, 1, 0);
			aKe->SetNext(aKMu);

			Variable* aMue = new Variable;
			aMue->DerivedSetup("aMue", 0.0005, 1, 0);
			aMue->SetNext(aKe);

			Float_t val1, val2, val3;
			Float_t max = 0.0;
			TBranch* BaKMu = newtree->Branch(aKMu->GetName(), &val1, aKMu->GetName());
			TBranch* BaKe = newtree->Branch(aKe->GetName(), &val2, aKe->GetName());
			TBranch* BaMue = newtree->Branch(aMue->GetName(), &val3, aMue->GetName());

			const bool istruth = false;
			Particle<Double_t> kaon("Kaon", ttree, istruth);			//Class from:   particlefromtree.cpp
			Particle<Double_t> muon("Mu", ttree, istruth);  			//Class from:   particlefromtree.cpp
			Particle<Double_t> electron("E", ttree, istruth);			//Class from:   particlefromtree.cpp

			newtree->SetBranchStatus("*",0);
			newtree->SetBranchStatus(aKMu->GetName(),1);
			newtree->SetBranchStatus(aKe->GetName(),1);
			newtree->SetBranchStatus(aMue->GetName(),1);

			//Loop through entries in the tree filling branches for the derived variables
			for (Int_t i = 0; i < newtree->GetEntries(); i++) {
				newtree->GetEntry(i);

				//Set Values to go into the branches
				aKMu->SetValBkg(angle::GetAngle(kaon.getVec(), muon.getVec()));
				 aKe->SetValBkg(angle::GetAngle(kaon.getVec(), electron.getVec()));
				aMue->SetValBkg(angle::GetAngle(muon.getVec(), electron.getVec()));
				val1 = aKMu->GetBkgVal();
				val2 = aKe->GetBkgVal();
				val3 = aMue->GetBkgVal();

				//Fill Branches
				BaKMu->Fill();
				BaKe->Fill();
				BaMue->Fill();

				if (val1 > max) {
					max = val1;
			//		std::cout << "\r" << "Max aKMu Angle: " << max << std::flush;
				}
			//	if(i%100==0) std::cout << "\r" << "aKMu Angle: " << aKMu->GetBkgVal() << std::flush;
				std::cout << "\r" << "New Branches Progrss: " << std::setw(4) << i * 100 / (newtree->GetEntries() - 1) << " %." << std::flush;
			}
			std::cout << "\nMax aKMu Angle: " << max << std::endl;
		//	if (CheckBranch(BaKMu, 0.0, max)) {
		//		std::cout << "Exiting!\n";
		//		return;
		//	}
		//	newtree->Write();
		//	else DrawHist(BaKMu, 0.0, max);
		//	DrawHist(BaKMu, 0, 1);
			//Get Clone of tree with cuts on derived variables
			newtree->SetBranchStatus("*",1);
		//	newtree = MakeCuts(aMue, newtree, newfile, newtreename);					//Function from:   GeneralFunctions.C
		//	std::cout << "Efficiency after Cuts: " << (Double_t)newtree->GetEntries() / ttree->GetEntries() << std::endl;

			tfile->Close();
			newtree->Write();
			newfile->Close();
		//	DeleteList(aMue);
		//	if (CheckBranchFromFile()) {
		//		std::cout << "Exiting!\n";
		//		return;
		//	}

			return;
		}

		void general::BothNewTrees() {
		//	NewTree("RECREATE","BkgTree","/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithMVAResponse.root",
		//					"/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithCuts.root","BkgTree");
		//	return;
			NewTree("UPDATE","SigTree","/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithMVAResponse.root",
											"/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWithCuts.root","SigTree");
			return;
		}
		void general::HistFromFileDouble(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
			TFile* tfile = TFile::Open(filename);
			TTree* ttree = (TTree*)tfile->Get(treename);
			ttree->SetBranchStatus("*", 0);
			ttree->SetBranchStatus(variable, 1);
			Double_t value;
			Double_t max = 0.0;
			Double_t min = 1000.0;
			Double_t sum = 0.0;
			Int_t count = 0;
			ttree->SetBranchAddress(variable, &value);
			TH1D* hist = new TH1D("hist", variable, 100, 0.0, 10000.0);
			Int_t error;
			for (Int_t i = 0; i < ttree->GetEntries(); i++) {
				error = ttree->GetEntry(i);
				if (error < 1) {
					std::cout << "Error getting entry\n";
					return;
				}
				if (value > max || value < min) {
					if (value > max) max = value;
					else			min = value;
					std::cout << "\r" << "Max Value = " << max << ", Min Value = " << min << std::flush;
				}
				if (value > 4) count++;
				hist->Fill(value);
			}
			std::cout << std::endl;
			std::cout << count << " values above 4\n";
			TCanvas* canv = new TCanvas("canv", variable, 400, 400);
			canv->cd();
			hist->Draw();
			return;
		}

		void general::HistFromFileFloat(const TString variable = "aKMu", const TString filename = "NewTreesWithCuts.root", const TString treename = "DecayTree") {
			TFile* tfile = TFile::Open(filename);
			TTree* ttree = (TTree*)tfile->Get(treename);
			ttree->SetBranchStatus("*", 0);
			ttree->SetBranchStatus(variable, 1);
			Float_t value;
			Float_t max = 0.0;
			Float_t min = 1000.0;
			Float_t sum = 0.0;
			Int_t count = 0;
			ttree->SetBranchAddress(variable, &value);
			TH1D* hist = new TH1D("hist", variable, 100, 0.0, 1.0);
			Int_t error;
			for (Int_t i = 0; i < ttree->GetEntries(); i++) {
				error = ttree->GetEntry(i);
				if (error < 1) {
					std::cout << "Error getting entry\n";
					return;
				}
				if (value > max || value < min) {
					if(value > max) max = value;
					else			min = value;
					std::cout << "\r" << "Max Value = " << max << ", Min Value = " << min << std::flush;
				}
				if (value > 4) count++;
				hist->Fill(value);
			}
			std::cout << std::endl;
			std::cout << count << " values above 4\n";
			TCanvas* canv = new TCanvas("canv", variable, 400, 400);
			canv->cd();
			hist->Draw();
			return;
		}

		void general::EnableBranches(TTree* tree, Variable* seed) {
			Variable *temp = seed;
			while (temp) {
				tree->SetBranchStatus(temp->GetName(), 1);
			}
			return;
		}

		Int_t general::combinedata(TString fnames = "BLAH,BLAH", TString treename = "DecayTree",
		            TString newname = "BLAH.root", TString path = "~/Project/TMVA/rootoutput/withcuts/") {
			if(fnames == "") {std::cout << "No File Names Submitted\n" << fnames << std::endl; return(1);}

			TObjArray* names = fnames.Tokenize(",");
		      if(!names->GetEntries()) {std::cout << "Number of variables incorrect\nExiting...\n\n"; return(1);}

		      TChain* chain = new TChain(treename);
		      TObjString* name = (TObjString*)names->First();
		      while(name) {
		            chain->Add(path + name->GetString());
				std::cout << "Added " << name->GetString() << " to chain\n";
		            name = (TObjString*)names->After(name);
		      }
		    // TString fn1  = "withcuts/_2011_MgUp_cut.root";
		    // TString fn2  = "withcuts/_2011_MgDn_cut.root";
		    // TString fn3  = "withcuts/_2012_MgUp_cut.root";
		    // TString fn4  = "withcuts/_2012_MgDn_cut.root";
		    // TString path = "/home/henry/Project/rootcode/TMVA/rootoutput/";
		    //
		    // TString tn  = "DecayTree";
		    //
		    // TChain* mychain = new TChain(tn);
		    // mychain->Add(path+fn1);
		    // mychain->Add(path+fn2);
		    // mychain->Add(path+fn3);
		    // mychain->Add(path+fn4);

		    // TString fnnew = path + "withcuts/AllDataCombined_afterCuts.root";
		    TFile *nfile = new TFile(path + newname, "RECREATE");
		    std::cout << "Cloning Chain with " << chain->GetEntries() << " entries.\n";
		    chain->CloneTree(-1,"fast");
		    nfile->Write();
		    nfile->Close();
		    std::cout << "Chain Written to: " << newname << std::endl;
		    return(0);
		}

		// void general::ExcludeEvents() {
		//
		// 	//Get trees for Bkg and MC Data
		// 	TTree* tBkg = GetTree(kTRUE);
		// 	TTree* tMC  = GetTree(kFALSE);
		//
		//
		// 	//Create Entry lists for MC and Bkg Data
		// //	TEventList* elBkg = new TEventList("BkgList", "List of Background entries to keep", tBkg);
		// //	TEventList* elMC  = new TEventList("MCList", "List of Monte Carlo entries to keep", tMC);
		// 	TTree* tBkgOut = tBkg->CloneTree(0);
		//
		// 	//Set up derived variables
		// 	Variable* aKMu = new Variable;
		// 	aKMu->DerivedSetup("KMuAngle", 0.0005, 1, 0);
		// 	Variable* aKe = new Variable;
		// 	aKMu->DerivedSetup("KeAngle", 0.0005, 1, 0);
		// 	aKe->SetNext(aKMu);
		// 	Variable* aMue = new Variable;
		// 	aKMu->DerivedSetup("MueAngle", 0.0005, 1, 0);
		// 	aMue->SetNext(aKe);
		//
		// 	//Disable all branches
		// 	//tBkg->SetBranchStatus("*", 0);
		// 	//tMC->SetBranchStatus("*", 0);
		//
		// 	//Re-enable branches of derived variables
		// 	//EnableBranches(tBkgOut, aMue);
		//
		// 	//Create Linked list of variables which enables the relevant branches
		// 	Variable* seed = general::CreateVarList(tBkg);
		// 	if (seed == NULL) {
		// 		std::cout << "Variable List is Empty\n";
		// 		return;
		// 	}
		//
		// 	//Add derived variables to the start of the list
		// 	aKMu->SetNext(seed);
		// 	seed = aMue;
		//
		// 	//Creates instances of variable that are derived from multiple branches in the trees.
		// 	const bool istruth = false;
		// 	Particle<Double_t> kaon("Kaon", tBkg, istruth);
		// 	Particle<Double_t> muon("Mu", tBkg, istruth);
		// 	Particle<Double_t> electron("E", tBkg, istruth);
		// 	Particle<Double_t> kaonMC("Kaon", tMC, istruth);
		// 	Particle<Double_t> muonMC("Mu", tMC, istruth);
		// 	Particle<Double_t> electronMC("E", tMC, istruth);
		//
		// 	//Find out which dataset has more entries
		// 	Long64_t big_ents;
		// 	if (tBkg->GetEntries() > tMC->GetEntries()) big_ents = tBkg->GetEntries();
		// 	else										big_ents = tMC->GetEntries();
		// 	std::cout << "Number of BKG Entries: " << tBkg->GetEntries() << std::endl;
		// 	std::cout << "Number of MC Entries:  " << tMC->GetEntries() << std::endl;
		//
		// 	//Create Variable pointer to move through the list
		// 	Variable* current = seed;
		//
		// 	//Create Variables to tell whether to exclude the entry
		// 	Int_t bkg_excl;
		// //	Int_t mc_excl;
		// 	Int_t i;
		// 	//Fill the Entry lists
		// 	for (i = 0; i < big_ents; i++) {
		// 		bkg_excl = 0;
		// //		mc_excl = 0;
		// //		if(i % 10000 == 0) std::cout << "i = " << i << std::endl;
		//
		// 		//Go through derived variables calculating their values
		// 		if (i < tBkg->GetEntries()) {
		// 			aKMu->SetValBkg( ( kaon.getVec() ).Angle( ( muon.getVec()     ).Vect() ) );
		// 			 aKe->SetValBkg( ( kaon.getVec() ).Angle( ( electron.getVec() ).Vect() ) );
		// 			aMue->SetValBkg( ( muon.getVec() ).Angle( ( electron.getVec() ).Vect() ) );
		// 		}
		//
		// 		//Go through the list of variables excluding events that don't pass the cut
		// 		current = seed;
		// 		while (current) {
		// 			//Background
		// 			if (i < tBkg->GetEntries()) {
		// 	//			tBkg->GetEntry(i);
		// //				if (i % 10000 == 0) std::cout << "BKG VALUE: " << current->GetBkgVal() << std::endl;
		// 				if (current->GetCutDir() != 0 && current->GetBkgVal() < current->GetCutVal()) {
		// 					bkg_excl++;
		// 				}
		// 				else if (current->GetCutDir() == 0 && current->GetBkgVal() > current->GetCutVal()) {
		// 					bkg_excl++;
		// 				}
		// 			}
		// 			//Monte Carlo
		// 	//		if (i < tMC->GetEntries() && current->HasMC()) {
		// 	//			tMC->GetEntry(i);
		// //				if (i % 10000 == 0) std::cout << "MC VALUE:  " << current->GetMCVal() << std::endl;
		// 	//			if (current->GetCutDir() != 0 && current->GetMCVal() < current->GetCutVal()) {
		// 	//				mc_excl++;
		// 	//			}
		// 	//			else if (current->GetCutDir() == 0 && current->GetMCVal() > current->GetCutVal()) {
		// 	//				mc_excl++;
		// 	//			}
		// 			}
		// 			current = current->GetNext();
		// 		}//End while loop over variables
		//
		// 		if (i < tBkg->GetEntries()) {
		// 			if (bkg_excl == 0) {
		// 	//			elBkg->Enter(i, tBkg);
		// 				tBkgOut->Fill();
		// 				if (current->IsDerived()) {
		// 					current->GetBranch()->Fill();
		// 				}
		// 			}
		// 		}
		// 	//	else std::cout << "Background Entry:  " << i << " Excluded\n";
		// 	//	if (i < tMC->GetEntries()) {
		// 	//		if (mc_excl == 0)  elMC->Enter(i, tMC);
		// 	//	}
		// 	//	else std::cout << "Monte Carlo Entry: " << i << " Excluded\n";
		// 	}//End of for loop over entries
		//
		// 	current = seed;
		// 	while (current) {
		// 		if (current->IsDerived()) {
		// 			current->GetBranch()->Draw();
		// 		}
		// 	}
		//
		// 	TFile* newfile = new TFile("Test.root", "RECREATE");
		// 	tBkgOut->Write();
		//
		// //	std::cout << "BKG still has Entries:  " << elBkg->GetN() << std::endl;
		// //	std::cout << "MC still has Entries:   " << elMC->GetN() << std::endl;
		// //	std::cout << "Background Efficiency:  " << (Double_t)elBkg->GetN() / tBkg->GetEntries() << std::endl;
		// //	std::cout << "Monte Carlo Efficiency: " << (Double_t)elMC->GetN() / tMC->GetEntries() << std::endl;
		//
		// //	tBkg->SetEntryList(elBkg);
		// //	std::cout << tBkg->GetEntries() << std::endl;
		// 	return;
		// }
//NOTE END OF GENERAL NAMESPACE



//NOTE BEGINNiNG OF ANGLE NAMESPACE
		Double_t angle::GetMax(Double_t K, Double_t Mu, Double_t e) {
		    Double_t max;
		    if(K > Mu)  max = K;
		    else        max = Mu;
		    if(e > max) max = e;
		    return max;
		}

		Double_t angle::GetMin(Double_t K, Double_t Mu, Double_t e) {
		    Double_t min;
		    if(K < Mu)  min = K;
		    else        min = Mu;
		    if(e < min) min = e;
		    return min;
		}

		Double_t angle::GetAngle(TLorentzVector V1, TLorentzVector V2) {
			return(V1.Angle(V2.Vect()) );
		}
//NOTE END OF ANGLE NAMESPACE


//NOTE BEGINNING OF LOAD NAMESPACE
		TTree* load::tree(TString filename, TString treename) {
			TFile* file = TFile::Open(filename);
			if(!file) {std::cout << "Couldn't Load File: " << filename << std::endl; return(NULL);}
			TTree* t = (TTree*)file->Get(treename);
			if(!t) {std::cout << "Couldn't Load Tree: " << treename << " from file: " << filename << std::endl; return(NULL);}

			return(t);
		}


//NOTE END OF LOAD NAMESPACE
