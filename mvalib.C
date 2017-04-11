//using namespace TMVA;
void MVAanalysis::TMVAClassificationApplication( const TString methodName = "BDT", const TString fileoption = "UPDATE",
						const TString extension = "_MC", const TString fileName   = "AllDataWithCutsWithPT.root",
				    const TString treeName   = "KMuE/DecayTree" ,const TString weightPath = "",
					const TString outfilename = "")
{

  if ( 0 == fileName.Length() ||
       0 == treeName.Length() ||
       0 == methodName.Length() ) {
    std::cout << " Wrong configuration, (at least) one argument is empty " << std::endl;
  }

  // --- This loads the library
  TMVA::Tools::Instance();

  std::vector< TString > variables;
  variables.push_back("B_DiraAngle");
  variables.push_back("B_PT" );
  variables.push_back("B_ENDVERTEX_CHI2");
  variables.push_back("B_TAU");
  variables.push_back("B_P");
	variables.push_back("B_IPCHI2_OWNPV");
	variables.push_back("B_FDCHI2_OWNPV");
	variables.push_back("MaxPT");
	variables.push_back("MinPT");
	variables.push_back("MaxIPChi2");
	variables.push_back("MinIPChi2");

  std::vector< Double_t > values( variables.size());
//	std::vector<Float_t> values2( 2 );
  // --- Create the Reader object
  TMVA::Reader *reader = new TMVA::Reader( variables, "!Color:!Silent" );
	Float_t BMass;
	reader->AddSpectator("B_M", &BMass);
	//
  // --- Book the MVA methods
  TString weightFile;
	weightFile = weightPath + "weights/TMVAClassification_";
	weightFile = weightFile + methodName;
	weightFile = weightFile + ".weights.xml";

  reader->BookMVA( methodName + " method", weightFile );

   // --- Get input tree
   TFile* theFile = TFile::Open( fileName );
   TTree* theTree = (TTree*) theFile->Get( treeName );
	 Int_t cou = 0;
   for ( unsigned int ivar = 0; ivar < variables.size(); ivar++ ){
		 theTree->SetBranchAddress( variables[ivar] , &values[ivar] );
		//  if(ivar<(variables.size()-2)) theTree->SetBranchAddress( variables[ivar] , &values[ivar] );
		//  else {
		// 	 	(ivar==(variables.size()-1)) theTree->SetBranchAddress( variables[ivar] , &values2[cou] );
		// 		cou++;
		//  }

   }


   const Long64_t entries = theTree->GetEntries();

   std::cout << "--- Processing: " << entries << " events" << std::endl;

   TStopwatch sw;
   sw.Start();

   // --- Create output file containing weights
//	 TString outfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWeights" + methodName + ".root";
   TFile* outFile = new TFile( outfilename, fileoption );
   TTree* outTree = new TTree( "TMVAWeights_"+methodName+extension, "TMVAWeights_"+methodName+extension );

   Double_t theMVAValue = 0;
   outTree->Branch( "MVA_"+methodName, &theMVAValue, "MVA/D" );

   // --- Event loop
   for (Long64_t ievt=0; ievt < entries; ievt++ ){

     if ( ievt % 1000 == 0 ){
       std::cout << "\r" << "--- ... Processing event: " << ievt << ". " << (ievt+1)*100/entries << " % ";
			 std::cout << ". Value = " << theMVAValue << std::flush;
		 }

     theTree->GetEntry(ievt);

     theMVAValue = reader->EvaluateMVA( values, methodName + " method" );

     outTree->Fill();
   }

   // Get elapsed time
   sw.Stop();
   std::cout << "--- End of event loop: "; sw.Print();

   outFile->cd();
   outTree->Write();
   outFile->Close();

   theFile->Close();


   delete reader;

   std::cout << "==> TMVAClassificationApplication is done!" << std::endl << std::endl;

   return ;
}

void MVAanalysis::GetAll() {
	TMVAClassificationApplication("BDT","RECREATE","_BKG","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithDerived.root","BkgTree");
	TMVAClassificationApplication("BDT","UPDATE","_MC","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithDerived.root","SigTree");
	/*
	TMVAClassificationApplication("BDT","RECREATE","_BKG","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","BkgTree");
	TMVAClassificationApplication("BDT","UPDATE","_MC","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","SigTree");
	TMVAClassificationApplication("MLP","RECREATE","_BKG","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","BkgTree");
	TMVAClassificationApplication("MLP","UPDATE","_MC","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","SigTree");
	TMVAClassificationApplication("BDTG","RECREATE","_BKG","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","BkgTree");
	TMVAClassificationApplication("BDTG","UPDATE","_MC","/home/epp/phunch/Documents/TMVA/RootFiles/AllDataWithCutsWithPT.root","SigTree");
	*/
	return;
}

Int_t MVAanalysis::TMVAClassification(TString bfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/OriginalDataWithPT.root", TString btreename = "BkgTree",
					   TString sfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/OriginalDataWithPT.root", TString streename = "SigTree",
					   TString outfileName = "/home/epp/phunch/Documents/TMVA/RootFiles/TMVAout_WithPT.root"/*,TString dataext = "_dat"*/)
{
   // The explicit loading of the shared libTMVA is done in TMVAlogon.C, defined in .rootrc
   // if you use your private .rootrc, or run from a different directory, please copy the
   // corresponding lines from .rootrc
   //---------------------------------------------------------------
   // This loads the library
   TMVA::Tools::Instance();

   // Default MVA methods to be trained + tested
   std::map<std::string,int> Use;

   // --- Cut optimisation
   Use["Cuts"]            = 0;
   Use["CutsD"]           = 0;
   Use["CutsPCA"]         = 0;
   Use["CutsGA"]          = 0;
   Use["CutsSA"]          = 0;
   //
   // --- 1-dimensional likelihood ("naive Bayes estimator")
   Use["Likelihood"]      = 0;
   Use["LikelihoodD"]     = 0; // the "D" extension indicates decorrelated input variables (see option strings)
   Use["LikelihoodPCA"]   = 0; // the "PCA" extension indicates PCA-transformed input variables (see option strings)
   Use["LikelihoodKDE"]   = 0;
   Use["LikelihoodMIX"]   = 0;
   //
   // --- Mutidimensional likelihood and Nearest-Neighbour methods
   Use["PDERS"]           = 0;
   Use["PDERSD"]          = 0;
   Use["PDERSPCA"]        = 0;
   Use["PDEFoam"]         = 0;
   Use["PDEFoamBoost"]    = 0; // uses generalised MVA method boosting
   Use["KNN"]             = 0; // k-nearest neighbour method
   //
   // --- Linear Discriminant Analysis
   Use["LD"]              = 0; // Linear Discriminant identical to Fisher
   Use["Fisher"]          = 0;
   Use["FisherG"]         = 0;
   Use["BoostedFisher"]   = 0; // uses generalised MVA method boosting
   Use["HMatrix"]         = 0;
   //
   // --- Function Discriminant analysis
   Use["FDA_GA"]          = 0; // minimisation of user-defined function using Genetics Algorithm
   Use["FDA_SA"]          = 0;
   Use["FDA_MC"]          = 0;
   Use["FDA_MT"]          = 0;
   Use["FDA_GAMT"]        = 0;
   Use["FDA_MCMT"]        = 0;
   //
   // --- Neural Networks (all are feed-forward Multilayer Perceptrons)
   Use["MLP"]         =0;  //  = 1; // Recommended ANN
   Use["MLPBFGS"]         = 0; // Recommended ANN with optional training method
   Use["MLPBNN"]          = 0; // Recommended ANN with BFGS training method and bayesian regulator
   Use["CFMlpANN"]        = 0; // Depreciated ANN from ALEPH
   Use["TMlpANN"]         = 0; // ROOT's own ANN
   Use["DNN"]             = 0; // improved implementation of a NN
   //
   // --- Support Vector Machine
   Use["SVM"]             = 0;
   //
   // --- Boosted Decision Trees
   Use["BDT"]             = 1; // uses Adaptive Boost
   Use["BDTG"]       = 0;  //  = 1; // uses Gradient Boost
   Use["BDTB"]            = 0; // uses Bagging
   Use["BDTD"]            = 0; // decorrelation + Adaptive Boost
   Use["BDTF"]            = 0; // allow usage of fisher discriminant for node splitting
   //
   // --- Friedman's RuleFit method, ie, an optimised series of cuts ("rules")
   Use["RuleFit"]         = 0;
   // ---------------------------------------------------------------

   std::cout << std::endl;
   std::cout << "==> Start TMVAClassification" << std::endl;

   //Get the signal tree
//   TString sfilename = "/home/epp/phunch/Project/Data/BuKMuE_Signal_MC_MD.root";
//   TString sfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/BMassCut.root";
   TFile* sigfile = TFile::Open(sfilename);
   if (!sigfile) {
	   std::cout << "[ERROR] Could not load Signal input file\n";
	   return(EXIT_FAILURE);
   }
//   sigfile->cd("KMuE");
//   TTree *signal = (TTree*)gDirectory->Get("DecayTree");
   TTree *signal = (TTree*)gDirectory->Get(streename);

   //Get the Background tree
//   TString bfilename = "/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root";
 //  TString bfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/BMassCut.root";	//Remember to comment out line 153
   TFile* bkgfile = TFile::Open(bfilename);
   if (!bkgfile) {
	   std::cout << "[ERROR] Could not load Background input file\n";
	   return(EXIT_FAILURE);
   }
//   bkgfile->cd("KMuE");
//  TTree *background = (TTree*)gDirectory->Get("DecayTree");
   TTree *background = (TTree*)gDirectory->Get(btreename);

   // Create a ROOT output file where TMVA will store ntuples, histograms, etc.
//   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   // Create the factory object. Later you can choose the methods
   // whose performance you'd like to investigate. The factory is
   // the only TMVA object you have to interact with
   //
   // The first argument is the base of the name of all the
   // weightfiles in the directory weight/
   //
   // The second argument is the output file for the training results
   // All TMVA output can be suppressed by removing the "!" (not) in
   // front of the "Silent" argument in the option string
   TMVA::Factory *factory = new TMVA::Factory( "TMVAClassification", outputFile,
                                               "!V:!Silent:Color:DrawProgressBar:Transformations=I;D;P;G,D:AnalysisType=Classification" );

   TMVA::DataLoader *dataloader=new TMVA::DataLoader("dataset"/*+dataext*/);
   // If you wish to modify default settings
   // (please check "src/Config.h" to see all available global options)
   //    (TMVA::gConfig().GetVariablePlotting()).fTimesRMS = 8.0;
   //    (TMVA::gConfig().GetIONames()).fWeightFileDir = "myWeightDirectory";

   // Define the input variables that shall be used for the MVA training
   // note that you may also use variable expressions, such as: "3*var1/var2*abs(var3)"
   // [all types of expressions that can also be parsed by TTree::Draw( "expression" )]

   // HERE ARE THE VARIABLES TO BE USED -----------------------------------------------------------------------------------------
   dataloader->AddVariable( "B_DiraAngle" , 'F' );
   dataloader->AddVariable( "B_PT", 'F' );
   dataloader->AddVariable( "B_ENDVERTEX_CHI2", 'F' );
   dataloader->AddVariable( "B_TAU", 'F' );
   dataloader->AddVariable("B_P", 'F');
   dataloader->AddVariable("B_IPCHI2_OWNPV", 'F');
   dataloader->AddVariable("B_FDCHI2_OWNPV", 'F');
	 dataloader->AddVariable("MaxPT", 'F');
   dataloader->AddVariable("MinPT", 'F');
	 dataloader->AddVariable("MaxIPChi2", 'F');
	 dataloader->AddVariable("MinIPChi2", 'F');
//   dataloader->AddVariable("Kaon_ProbNNk", 'F');
//   dataloader->AddVariable("Kaon_ProbNNpi", 'F');
//   dataloader->AddVariable("Mu_ProbNNmu", 'F');
//   dataloader->AddVariable("Mu_ProbNNe", 'F');
//   dataloader->AddVariable("E_ProbNNmu", 'F');
//   dataloader->AddVariable("E_ProbNNe", 'F');




   // You can add so-called "Spectator variables", which are not used in the MVA training,
   // but will appear in the final "TestTree" produced by TMVA. This TestTree will contain the
   // input variables, the response values of all trained MVAs, and the spectator variables

   dataloader->AddSpectator( "B_M", 'F' );


   // global event weights per tree (see below for setting event-wise weights)
   Double_t signalWeight     = 1.0;
   Double_t backgroundWeight = 1.0;

   // You can add an arbitrary number of signal or background trees
   dataloader->AddSignalTree    ( signal,     signalWeight     );
   dataloader->AddBackgroundTree( background, backgroundWeight );

   // To give different trees for training and testing, do as follows:
   //    dataloader->AddSignalTree( signalTrainingTree, signalTrainWeight, "Training" );
   //    dataloader->AddSignalTree( signalTestTree,     signalTestWeight,  "Test" );

   // Use the following code instead of the above two or four lines to add signal and background
   // training and test events "by hand"
   // NOTE that in this case one should not give expressions (such as "var1+var2") in the input
   //      variable definition, but simply compute the expression before adding the event
   //
   //     // --- begin ----------------------------------------------------------
   //     std::vector<Double_t> vars( 4 ); // vector has size of number of input variables
   //     Float_t  treevars[4], weight;
   //
   //     // Signal
   //     for (UInt_t ivar=0; ivar<4; ivar++) signal->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   //     for (UInt_t i=0; i<signal->GetEntries(); i++) {
   //        signal->GetEntry(i);
   //        for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //        // add training and test events; here: first half is training, second is testing
   //        // note that the weight can also be event-wise
   //        if (i < signal->GetEntries()/2.0) dataloader->AddSignalTrainingEvent( vars, signalWeight );
   //        else                              dataloader->AddSignalTestEvent    ( vars, signalWeight );
   //     }
   //
   //     // Background (has event weights)
   //     background->SetBranchAddress( "weight", &weight );
   //     for (UInt_t ivar=0; ivar<4; ivar++) background->SetBranchAddress( Form( "var%i", ivar+1 ), &(treevars[ivar]) );
   //     for (UInt_t i=0; i<background->GetEntries(); i++) {
   //        background->GetEntry(i);
   //        for (UInt_t ivar=0; ivar<4; ivar++) vars[ivar] = treevars[ivar];
   //        // add training and test events; here: first half is training, second is testing
   //        // note that the weight can also be event-wise
   //        if (i < background->GetEntries()/2) dataloader->AddBackgroundTrainingEvent( vars, backgroundWeight*weight );
   //        else                                dataloader->AddBackgroundTestEvent    ( vars, backgroundWeight*weight );
   //     }
         // --- end ------------------------------------------------------------
   //
   // --- end of tree registration

   // Set individual event weights (the variables must exist in the original TTree)
   //    for signal    : dataloader->SetSignalWeightExpression    ("weight1*weight2");
   //    for background: dataloader->SetBackgroundWeightExpression("weight1*weight2");

   //dataloader->SetBackgroundWeightExpression( "weight" );

   // Apply additional cuts on the signal and background samples (can be different)
   TCut mycuts = ""; // for example: TCut mycuts = "abs(var1)<0.5 && abs(var2-0.5)<1";
   TCut mycutb = ""; // for example: TCut mycutb = "abs(var1)<0.5";

   // Tell the dataloader how to use the training and testing events
   //
   // If no numbers of events are given, half of the events in the tree are used
   // for training, and the other half for testing:
   //    dataloader->PrepareTrainingAndTestTree( mycut, "SplitMode=random:!V" );
   // To also specify the number of testing events, use:
//       dataloader->PrepareTrainingAndTestTree( mycuts,
//                                            "nTrain_Signal=3000:nTrain_Background=3000:NTest_Signal=3000:NTest_Background=3000:SplitMode=Random:!V" );


   dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
					   "nTrain_Signal=1000:nTrain_Background=1000:SplitMode=Random:NormMode=NumEvents:!V" );

   /* TB
      dataloader->PrepareTrainingAndTestTree( mycuts, mycutb,
      "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" );
   */

   // ---- Book MVA methods
   //
   // Please lookup the various method configuration options in the corresponding cxx files, eg:
   // src/MethoCuts.cxx, etc, or here: http://tmva.sourceforge.net/optionRef.html
   // it is possible to preset ranges in the option string in which the cut optimisation should be done:
   // "...:CutRangeMin[2]=-1:CutRangeMax[2]=1"...", where [2] is the third input variable

   // Cut optimisation
   if (Use["Cuts"])
      factory->BookMethod( dataloader, TMVA::Types::kCuts, "Cuts",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart" );

   if (Use["CutsD"])
      factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsD",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=Decorrelate" );

   if (Use["CutsPCA"])
      factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsPCA",
                           "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart:VarTransform=PCA" );

   if (Use["CutsGA"])
      factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsGA",
                           "H:!V:FitMethod=GA:CutRangeMin[0]=-10:CutRangeMax[0]=10:VarProp[1]=FMax:EffSel:Steps=30:Cycles=3:PopSize=400:SC_steps=10:SC_rate=5:SC_factor=0.95" );

   if (Use["CutsSA"])
      factory->BookMethod( dataloader, TMVA::Types::kCuts, "CutsSA",
                           "!H:!V:FitMethod=SA:EffSel:MaxCalls=150000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   // Likelihood ("naive Bayes estimator")
   if (Use["Likelihood"])
      factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "Likelihood",
                           "H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmoothBkg[1]=10:NSmooth=1:NAvEvtPerBin=50" );

   // Decorrelated likelihood
   if (Use["LikelihoodD"])
      factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodD",
                           "!H:!V:TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=Decorrelate" );

   // PCA-transformed likelihood
   if (Use["LikelihoodPCA"])
      factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodPCA",
                           "!H:!V:!TransformOutput:PDFInterpol=Spline2:NSmoothSig[0]=20:NSmoothBkg[0]=20:NSmooth=5:NAvEvtPerBin=50:VarTransform=PCA" );

   // Use a kernel density estimator to approximate the PDFs
   if (Use["LikelihoodKDE"])
      factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodKDE",
                           "!H:!V:!TransformOutput:PDFInterpol=KDE:KDEtype=Gauss:KDEiter=Adaptive:KDEFineFactor=0.3:KDEborder=None:NAvEvtPerBin=50" );

   // Use a variable-dependent mix of splines and kernel density estimator
   if (Use["LikelihoodMIX"])
      factory->BookMethod( dataloader, TMVA::Types::kLikelihood, "LikelihoodMIX",
                           "!H:!V:!TransformOutput:PDFInterpolSig[0]=KDE:PDFInterpolBkg[0]=KDE:PDFInterpolSig[1]=KDE:PDFInterpolBkg[1]=KDE:PDFInterpolSig[2]=Spline2:PDFInterpolBkg[2]=Spline2:PDFInterpolSig[3]=Spline2:PDFInterpolBkg[3]=Spline2:KDEtype=Gauss:KDEiter=Nonadaptive:KDEborder=None:NAvEvtPerBin=50" );

   // Test the multi-dimensional probability density estimator
   // here are the options strings for the MinMax and RMS methods, respectively:
   //      "!H:!V:VolumeRangeMode=MinMax:DeltaFrac=0.2:KernelEstimator=Gauss:GaussSigma=0.3" );
   //      "!H:!V:VolumeRangeMode=RMS:DeltaFrac=3:KernelEstimator=Gauss:GaussSigma=0.3" );
   if (Use["PDERS"])
      factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERS",
                           "!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600" );

   if (Use["PDERSD"])
      factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSD",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=Decorrelate" );

   if (Use["PDERSPCA"])
      factory->BookMethod( dataloader, TMVA::Types::kPDERS, "PDERSPCA",
                           "!H:!V:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.3:NEventsMin=400:NEventsMax=600:VarTransform=PCA" );

   // Multi-dimensional likelihood estimator using self-adapting phase-space binning
   if (Use["PDEFoam"])
      factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoam",
                           "!H:!V:SigBgSeparate=F:TailCut=0.001:VolFrac=0.0666:nActiveCells=500:nSampl=2000:nBin=5:Nmin=100:Kernel=None:Compress=T" );

   if (Use["PDEFoamBoost"])
      factory->BookMethod( dataloader, TMVA::Types::kPDEFoam, "PDEFoamBoost",
                           "!H:!V:Boost_Num=30:Boost_Transform=linear:SigBgSeparate=F:MaxDepth=4:UseYesNoCell=T:DTLogic=MisClassificationError:FillFoamWithOrigWeights=F:TailCut=0:nActiveCells=500:nBin=20:Nmin=400:Kernel=None:Compress=T" );

   // K-Nearest Neighbour classifier (KNN)
   if (Use["KNN"])
      factory->BookMethod( dataloader, TMVA::Types::kKNN, "KNN",
                           "H:nkNN=20:ScaleFrac=0.8:SigmaFact=1.0:Kernel=Gaus:UseKernel=F:UseWeight=T:!Trim" );

   // H-Matrix (chi2-squared) method
   if (Use["HMatrix"])
      factory->BookMethod( dataloader, TMVA::Types::kHMatrix, "HMatrix", "!H:!V:VarTransform=None" );

   // Linear discriminant (same as Fisher discriminant)
   if (Use["LD"])
      factory->BookMethod( dataloader, TMVA::Types::kLD, "LD", "H:!V:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher discriminant (same as LD)
   if (Use["Fisher"])
      factory->BookMethod( dataloader, TMVA::Types::kFisher, "Fisher", "H:!V:Fisher:VarTransform=None:CreateMVAPdfs:PDFInterpolMVAPdf=Spline2:NbinsMVAPdf=50:NsmoothMVAPdf=10" );

   // Fisher with Gauss-transformed input variables
   if (Use["FisherG"])
      factory->BookMethod( dataloader, TMVA::Types::kFisher, "FisherG", "H:!V:VarTransform=Gauss" );

   // Composite classifier: ensemble (tree) of boosted Fisher classifiers
   if (Use["BoostedFisher"])
      factory->BookMethod( dataloader, TMVA::Types::kFisher, "BoostedFisher",
                           "H:!V:Boost_Num=20:Boost_Transform=log:Boost_Type=AdaBoost:Boost_AdaBoostBeta=0.2:!Boost_DetailedMonitoring" );

   // Function discrimination analysis (FDA) -- test of various fitters - the recommended one is Minuit (or GA or SA)
   if (Use["FDA_MC"])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MC",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:SampleSize=100000:Sigma=0.1" );

   if (Use["FDA_GA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_GA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:PopSize=300:Cycles=3:Steps=20:Trim=True:SaveBestGen=1" );

   if (Use["FDA_SA"]) // can also use Simulated Annealing (SA) algorithm (see Cuts_SA options])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_SA",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=SA:MaxCalls=15000:KernelTemp=IncAdaptive:InitialTemp=1e+6:MinTemp=1e-6:Eps=1e-10:UseDefaultScale" );

   if (Use["FDA_MT"])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=2:UseImprove:UseMinos:SetBatch" );

   if (Use["FDA_GAMT"])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_GAMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=GA:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:Cycles=1:PopSize=5:Steps=5:Trim" );

   if (Use["FDA_MCMT"])
      factory->BookMethod( dataloader, TMVA::Types::kFDA, "FDA_MCMT",
                           "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x3:ParRanges=(-1,1);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20" );

   // TMVA ANN: MLP (recommended ANN) -- all ANNs in TMVA are Multilayer Perceptrons
   if (Use["MLP"])
      factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator" );

   if (Use["MLPBFGS"])
      factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBFGS", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:!UseRegulator" );

   if (Use["MLPBNN"])
      factory->BookMethod( dataloader, TMVA::Types::kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator" ); // BFGS training with bayesian regulators


   // improved neural network implementation
   if (Use["DNN"])
   {
//       TString layoutString ("Layout=TANH|(N+100)*2,LINEAR");
//       TString layoutString ("Layout=SOFTSIGN|100,SOFTSIGN|50,SOFTSIGN|20,LINEAR");
//       TString layoutString ("Layout=RELU|300,RELU|100,RELU|30,RELU|10,LINEAR");
//       TString layoutString ("Layout=SOFTSIGN|50,SOFTSIGN|30,SOFTSIGN|20,SOFTSIGN|10,LINEAR");
//       TString layoutString ("Layout=TANH|50,TANH|30,TANH|20,TANH|10,LINEAR");
//       TString layoutString ("Layout=SOFTSIGN|50,SOFTSIGN|20,LINEAR");
       TString layoutString ("Layout=TANH|100,TANH|50,TANH|10,LINEAR");

       TString training0 ("LearningRate=1e-1,Momentum=0.0,Repetitions=1,ConvergenceSteps=300,BatchSize=20,TestRepetitions=15,WeightDecay=0.001,Regularization=NONE,DropConfig=0.0+0.5+0.5+0.5,DropRepetitions=1,Multithreading=True");
       TString training1 ("LearningRate=1e-2,Momentum=0.5,Repetitions=1,ConvergenceSteps=300,BatchSize=30,TestRepetitions=7,WeightDecay=0.001,Regularization=L2,Multithreading=True,DropConfig=0.0+0.1+0.1+0.1,DropRepetitions=1");
       TString training2 ("LearningRate=1e-2,Momentum=0.3,Repetitions=1,ConvergenceSteps=300,BatchSize=40,TestRepetitions=7,WeightDecay=0.0001,Regularization=L2,Multithreading=True");
       TString training3 ("LearningRate=1e-3,Momentum=0.1,Repetitions=1,ConvergenceSteps=200,BatchSize=70,TestRepetitions=7,WeightDecay=0.0001,Regularization=NONE,Multithreading=True");

       TString trainingStrategyString ("TrainingStrategy=");
       trainingStrategyString += training0 + "|" + training1 + "|" + training2 + "|" + training3;


//       TString nnOptions ("!H:V:VarTransform=Normalize:ErrorStrategy=CROSSENTROPY");
       TString nnOptions ("!H:V:ErrorStrategy=CROSSENTROPY:VarTransform=G:WeightInitialization=XAVIERUNIFORM");
//       TString nnOptions ("!H:V:VarTransform=Normalize:ErrorStrategy=CHECKGRADIENTS");
       nnOptions.Append (":"); nnOptions.Append (layoutString);
       nnOptions.Append (":"); nnOptions.Append (trainingStrategyString);

       factory->BookMethod(dataloader, TMVA::Types::kDNN, "DNN", nnOptions ); // NN
   }



   // CF(Clermont-Ferrand)ANN
   if (Use["CFMlpANN"])
      factory->BookMethod( dataloader, TMVA::Types::kCFMlpANN, "CFMlpANN", "!H:!V:NCycles=2000:HiddenLayers=N+1,N"  ); // n_cycles:#nodes:#nodes:...

   // Tmlp(Root)ANN
   if (Use["TMlpANN"])
      factory->BookMethod( dataloader, TMVA::Types::kTMlpANN, "TMlpANN", "!H:!V:NCycles=200:HiddenLayers=N+1,N:LearningMethod=BFGS:ValidationFraction=0.3"  ); // n_cycles:#nodes:#nodes:...

   // Support Vector Machine
   if (Use["SVM"])
      factory->BookMethod( dataloader, TMVA::Types::kSVM, "SVM", "Gamma=0.25:Tol=0.001:VarTransform=Norm" );

   // Boosted Decision Trees
   if (Use["BDTG"]) // Gradient Boost
      factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTG",
                           "!H:!V:NTrees=1000:MinNodeSize=2.5%:BoostType=Grad:Shrinkage=0.10:UseBaggedBoost:BaggedSampleFraction=0.5:nCuts=20:MaxDepth=2" );

   if (Use["BDT"])  // Adaptive Boost
      factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDT",
                           "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTB"]) // Bagging
      factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTB",
                           "!H:!V:NTrees=400:BoostType=Bagging:SeparationType=GiniIndex:nCuts=20" );

   if (Use["BDTD"]) // Decorrelation + Adaptive Boost
      factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTD",
                           "!H:!V:NTrees=400:MinNodeSize=5%:MaxDepth=3:BoostType=AdaBoost:SeparationType=GiniIndex:nCuts=20:VarTransform=Decorrelate" );

   if (Use["BDTF"])  // Allow Using Fisher discriminant in node splitting for (strong) linearly correlated variables
      factory->BookMethod( dataloader, TMVA::Types::kBDT, "BDTMitFisher",
                           "!H:!V:NTrees=50:MinNodeSize=2.5%:UseFisherCuts:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:SeparationType=GiniIndex:nCuts=20" );

   // RuleFit -- TMVA implementation of Friedman's method
   if (Use["RuleFit"])
      factory->BookMethod( dataloader, TMVA::Types::kRuleFit, "RuleFit",
                           "H:!V:RuleFitModule=RFTMVA:Model=ModRuleLinear:MinImp=0.001:RuleMinDist=0.001:NTrees=20:fEventsMin=0.01:fEventsMax=0.5:GDTau=-1.0:GDTauPrec=0.01:GDStep=0.01:GDNSteps=10000:GDErrScale=1.02" );

   // For an example of the category classifier usage, see: TMVAClassificationCategory

   // --------------------------------------------------------------------------------------------------

   // ---- Now you can optimize the setting (configuration) of the MVAs using the set of training events

   // ---- STILL EXPERIMENTAL and only implemented for BDT's !
   // factory->OptimizeAllMethods("SigEffAt001","Scan");
   // factory->OptimizeAllMethods("ROCIntegral","FitGA");

   // --------------------------------------------------------------------------------------------------

   // ---- Now you can tell the factory to train, test, and evaluate the MVAs

   // Train MVAs using the set of training events
   factory->TrainAllMethods();

   // ---- Evaluate all MVAs using the set of test events
   factory->TestAllMethods();

   // ----- Evaluate and compare performance of all configured MVAs
   factory->EvaluateAllMethods();

   // --------------------------------------------------------------

   // Save the output
   outputFile->Close();

   std::cout << "==> Wrote root file: " << outputFile->GetName() << std::endl;
   std::cout << "==> TMVAClassification is done!" << std::endl;

   delete factory;
   delete dataloader;
   // Launch the GUI for the root macros
   if (!gROOT->IsBatch()) TMVA::TMVAGui( outfileName );

   return 0;
}

void TMVAtrees::TMVACut(TTree* bNewTree, TString MVAmethod = "BDT", TString wfilename = "gd.ty"){
    std::cout << "\r" << "Progress Check: " << "Beginning                                           " << std::flush;

//    TString wfilename  = "/home/henry/Documents/Project/rootcode/TMVA/TMVAWeights.root";  //For My PC
//    TString wfilename  = "/home/henry/Project/rootcode/TMVA/rootoutput/AllDataWeights"+MVAmethod+".root";  //For CSC Machines
    TString bwtreename = "TMVAWeights_"+MVAmethod;//+"_BKG";
 //   TString swtreename = "TMVAWeights_"+MVAmethod+"_MC";

    Int_t bEnts       = bNewTree->GetEntries();
//    Int_t sigEnts     = sigNewTree->GetEntries();

    std::cout << "\r" << "Progress Check: " << "Opening Files and Getting Trees                     " << std::flush;

     TFile* wFile = TFile::Open( wfilename );
     if(wFile==NULL) {std::cout << "Couldn't Open Weights File\n"; return;}
     TTree* bwTree = (TTree*)wFile->Get( bwtreename );
     if(bwTree==NULL) {std::cout << "Couldn't Load Background Weights Tree " << bwtreename << "\n"; return;}
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

void TMVAtrees::GetTMVATrees( TString bfilename = "/home/henry/Project/rootcode/TMVA/rootoutput/TMVA.root", TString btreename = "BkgTree", //For My PC
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
      TMVAtrees::TMVACut(bNewTree,"BDT",wfilename);
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
