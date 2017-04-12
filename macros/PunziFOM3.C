#include "RooGlobalFunc.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooPlot.h"
using namespace RooFit;


//A Class that deals with the exponential and finds its integral
class exponent {
//Exponent of the form f(x) = a*e^{(x-c)*b}
	private:
		Double_t a; //Multiplier
		Double_t b;	//Exponent multiplier
		Double_t c; //Mass offset
	public:
		Double_t Evaluate(Double_t x); //Evaluate the exponential at x
		Double_t EvaluateIntegral(Double_t xmin, Double_t xmax); //Evaluate the integral of the exponent over the range xmin to xmax
		Double_t Geta(); //Get the value of a
		Double_t Getb(); //Get the value of b
		Double_t Getc(); //Get the value of the offset
		void aMultiply(Double_t multiplier); //multiply a by a factor multiplier
		void SetParams(Double_t a0, Double_t b0, Double_t c0); //Set the values of the parameters
		void PrintParams(); //Print the values of the parameters
};
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
Double_t GetBkgEvents(TString filename = "/home/epp/phunch/Project/Data/BuKMuE_2011_MgUp.root", TString treename = "KMuE/DecayTree",
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
Double_t *GetPunzi(const TString method = "", TString mvafilename = "RootFiles/TMVA.root", TString bkgfilename = "BMassCut.root",
				   TString bkgtreename = "BkgTree", TString ext = "", Bool_t calculate_Nbkg = kTRUE, Bool_t PlotFit = kFALSE) {

	std::cout << "Getting Punzi Figure of Merit\n";

	//Open the file for reading efficiency data from
	TFile* tfile = TFile::Open(mvafilename);
	if(!tfile) {std::cout << "Couldn't Get TMVA file.\nExiting...\n"; return(NULL);}
	//Create path to histogram
	TString path = "dataset" /*+ ext*/;
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
	TH1D *hist_PFoM = new TH1D("PFOM_" + method, "Punzi Figure of Merit " + method, hist_effB->GetNbinsX(), min_output, max_output);
	hist_PFoM->SetXTitle("Classifier Output");
	hist_PFoM->SetYTitle("Punzi Figure of Merit");
	hist_PFoM->GetYaxis()->SetTitleOffset(1.4);

	Int_t N_Bkg;
	//Get the number of background events in the signal region
	if (calculate_Nbkg) N_Bkg = GetBkgEvents(bkgfilename, bkgtreename, PlotFit);
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
	TLine *punzi_hori = new TLine(min_output, max_FoM, cut_val, max_FoM);
	punzi_vert->SetLineColor(kGreen); punzi_vert->Draw();
	punzi_hori->SetLineColor(kGreen); punzi_hori->Draw();

	//Draw lines on the efficiency hist to see the efficiencies at the optimum cut value
	TLine *effS_vert = new TLine(cut_val, 0, cut_val, hist_effS->GetBinContent(cut_bin));
	TLine *effS_hori = new TLine(min_output, hist_effS->GetBinContent(cut_bin), cut_val, hist_effS->GetBinContent(cut_bin));
	effS_vert->SetLineColor(kGreen);
	effS_hori->SetLineColor(kGreen);
	TLine *effB_vert = new TLine(cut_val, 0, cut_val, hist_effB->GetBinContent(cut_bin));
	TLine *effB_hori = new TLine(min_output, hist_effB->GetBinContent(cut_bin), cut_val, hist_effB->GetBinContent(cut_bin));
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

void GetAllPunzi(TString mvafilename = "RootFiles/TMVA.root",
							   TString bkgfilename = "BMassCut.root", TString bkgtreename = "BkgTree") {

		 GetPunzi("MLP",mvafilename,bkgfilename,bkgtreename);
		 GetPunzi("BDTG",mvafilename,bkgfilename,bkgtreename);
		 GetPunzi("BDT",mvafilename,bkgfilename,bkgtreename);
		 return;
}
