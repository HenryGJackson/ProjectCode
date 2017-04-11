#ifndef MVALIB_H
#define MVALIB_H

//NOTES FOR DAN!!!!!!!
//You'll need to change the path of the included files to wherever you store them
//There is an included file at the bottom of each .h file (this isn't proper practice but it works in ROOT)
//NOTES FOR DAN!!!!!!!

#include <cstdlib>
#include <vector>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <iomanip>
#include <ctime>

#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TStopwatch.h"
#include "TChain.h"
#include "TObjString.h"

#include "TMVA/Tools.h"
#include "TMVA/Reader.h"
#include "TMVA/MethodCuts.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/TMVAGui.h"
#include "TMVA/DataSet.h"

//Namespaces defined in this file: MVAanalysis
//                                 MVAtrees
//Singular functions defined in this file: GetBigger()
//                                         PrintTime()
//                                         combinedate();

namespace MVAanalysis{
      //Apply BDT to file
      void TMVAClassificationApplication( const TString methodName = "BDT", const TString fileoption = "UPDATE",
		const TString extension = "_MC", const TString fileName   = "AllDataWithCutsWithPT.root",
		const TString treeName   = "KMuE/DecayTree" ,const TString weightPath = "",
		const TString outfilename = "");
      //Get BDT weight trees for both sig and bkg
      void GetAll();
      //Train BDT
      int TMVAClassification(TString bfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/OriginalDataWithPT.root", TString btreename = "BkgTree",
            TString sfilename = "/home/epp/phunch/Documents/TMVA/RootFiles/OriginalDataWithPT.root", TString streename = "SigTree",
            TString outfileName = "/home/epp/phunch/Documents/TMVA/RootFiles/TMVAout_WithPT.root"/*,TString dataext = "_dat"*/);
}

//Return the larger value of a and b
Int_t GetBigger(Int_t a, Int_t b);
//Print the current time
void PrintTime();

namespace TMVAtrees{
        void TMVACut(TTree* bNewTree, TString MVAmethod = "BDT", TString wfilename = "gd.ty");
        //Get trees with TMVA weights added as a new branch
        void GetTMVATrees( TString bfilename = "/home/henry/Project/rootcode/TMVA/rootoutput/TMVA.root", TString btreename = "BkgTree", //For My PC
                      TString wfilename = "bfe.root", TString nfilename  = "/home/henry/Project/rootcode/TMVA/rootoutput/DataWithPT_MVAResponse.root");

}



//Libs found on phunch storage area "/storage/epp2/phunch/src/"
//Can use the above as the path^
// #include "/home/henry/Project/TMVA/src/mvalib.C"
#include "/storage/epp2/phunch/src/mvalib.C"

#endif
