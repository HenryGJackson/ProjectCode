void efficiency(const TString method = "") {

	//Open the file for reading efficiency data from
	TFile* tfile = TFile::Open("TMVA.root");
	
	//Create path to histogram
	TString path = "dataset";
	path += "/Method_";
	path += method ;
	path += "/" ;
	path += method;
	path += "/MVA_";
	path += method;

	//Get Background efficiency and draw it on the LHS of a new TCanvas
	TH1D* hist_effB = (TH1D*)tfile->Get(path + "_effB");
	TCanvas *c1 = new TCanvas(method, method,800,400);
	c1->Divide(2);
	c1->cd(1);
	hist_effB->Draw();

	//Get number of entries and create new histogram to plot the Punzi FoM
	Double_t cut, eff, FoM;
	Double_t max_FoM = 0.0;
	Int_t N_Bkg = 1000;

	TH1D *hist_PFoM = new TH1D("PFOM_"+method, "Punzi Figure of Merit " + method, 100, 0.0, 0.05);

	//Get each entry in the BKG efficiency histogram and use this to fill the histogram for the Punzi FoM
	for (int i = 0; i < hist_effB->GetNbinsX(); i++) {
		eff = hist_effB->GetBinContent(i + 1);
		FoM = eff / (1.5 + sqrt(N_Bkg*eff));
		hist_PFoM->Fill(FoM);
		if (FoM > max_FoM) max_FoM = FoM;
	}

	//Draw the Punzi FoM Histogram on the RHS of the TCanvas
	c1->cd(2);
	hist_PFoM->SetAxisRange(0.0,max_FoM);
	hist_PFoM->Draw();

	//Get the maximum bin and print it's value
	Double_t peak_FoM = hist_PFoM->GetBinCenter(hist_PFoM->GetMaximumBin());
	
	//Get the Efficiency at PFoM with largest bin content
	eff = 0.5*(peak_FoM*sqrt(N_Bkg) + sqrt(peak_FoM*peak_FoM*N_Bkg + 6 * peak_FoM));
	eff = eff*eff;

	//Find out the bin with this efficiency in the BKG efficiency histogram
	Double_t eff_tmp, diff_tmp, cut_tmp;
	Double_t diff    = 100000.0;
	Double_t cut_val = 0.0;
	Double_t max_eff = 1.01*eff;
	Double_t min_eff = 0.99*eff;
	for (int i = 0; i < hist_effB->GetNbinsX(); i++) {
		eff_tmp = hist_effB->GetBinContent(i + 1);
		//If efficiency of this bin is within the range check whether to store its info
		if (eff_tmp > min_eff && eff_tmp < max_eff) {
			diff_tmp = abs(eff - eff_tmp);
			//If this bin is closer to the true efficiency than the last then store its info
			if (diff_tmp < diff) {
				cut_val = hist_effB->GetBinCenter(i + 1);
				diff = diff_tmp;
			}
			else break;
		}

		//On the last iteration print an error message if the efficiency wasn't found
		if (i == hist_effB->GetNbinsX() - 1 && cut_val == 0.0) {
			std::cout << "[ERROR] Couldn't find the cut value\n";
			return;
		}
	}
	
	//Draw lines on the histograms to show where these optimal values are
	//Punzi FoM:
	Double_t peak_val = hist_PFoM->GetBinContent(hist_PFoM->GetMaximumBin());
	Double_t min_edge = hist_PFoM->GetBinLowEdge(1);
	TLine *PFoM_vert = new TLine(peak_FoM, 0, peak_FoM, peak_val);
	TLine *PFoM_hori = new TLine(min_edge, peak_val, peak_FoM, peak_val);
	PFoM_vert->SetLineColor(kRed); PFoM_vert->Draw();
	PFoM_hori->SetLineColor(kRed); PFoM_hori->Draw();

	//Efficiency:
	c1->cd(1);
	min_edge = hist_effB->GetBinLowEdge(1);
	TLine *effB_vert = new TLine(cut_val, 0, cut_val, eff);
	TLine *effB_hori = new TLine(min_edge, eff, cut_val, eff);
	effB_vert->SetLineColor(kRed); effB_vert->Draw();
	effB_hori->SetLineColor(kRed); effB_hori->Draw();

	//Output information learned
	std::cout << "** " << method << " **" << std::endl;
	std::cout << "Optimum Punzi Figure of Merit:    " << peak_FoM << std::endl;
	std::cout << "Optimum cut at efficiency:        " << eff << std::endl;
	std::cout << "Optimum cut at classifier output: " << cut_val << std::endl;


	return;
}
