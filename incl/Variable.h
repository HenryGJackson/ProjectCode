#ifndef VARIABLE_H
#define VARIABLE_H

#include "TLorentzVector.h"
#include "TTree.h"
#include "TBranch.h"
#include "TVector3.h"
#include "TCut.h"
#include "TString.h"
#include "TCanvas.h"
#include "TEventList.h"
#include <fstream>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace patch
{
    template < typename T > std::string to_string( const T& n )
    {
        std::ostringstream stm ;
        stm << n ;
        return stm.str() ;
    }
}

class Variable {
private:
	TString			var_name;
	Double_t		var_cut_val;
	Double_t		var_max;
	Double_t		var_min;
	Int_t			cut_less_than;    //if = 1 cut away the values greater than the cut value - for bool, 1 means keep if false
	Variable*		next_var = NULL;
	Int_t			hasMonteCarlo;    //if = 0 only bkg data available
	Bool_t			derived;
	TCut			mycut;
	TLorentzVector	myvect;
  TString var_type;

public:
  Bool_t bool_value;
	Double_t	bkg_value;
	Float_t	mc_value;

	void		SetName(TString name);
	void		SetCutVal(Double_t val);
	void		SetLimits(Double_t min, Double_t max);
	void		SetCutDir(Int_t myint);
	void		SetNext(Variable* next);
	void		SetHasMC(Int_t hasmc);
	void		SetValBkg(Double_t bval);
	void		SetValMC(Float_t mval);
	void		SetDerived();
	void		EnableBranch(TTree *tree);
	void		SetVect(TLorentzVector v);
  void    NotDerived();
  void    SetType(TString type);

	TString			GetName();
	Double_t		GetBkgVal();
	Float_t		GetMCVal();
	Double_t		GetCutVal();
	Int_t			GetCutDir();
	Variable*		GetNext();
	Double_t		GetMin();
	Double_t		GetMax();
	Int_t			HasMC();
	Bool_t			IsDerived();
	TCut			GetCut();
	TLorentzVector	GetVect();
  TString  GetType();
	void		DerivedSetup(TString name, Double_t cut, Int_t cut_dir, Int_t MC);
	void		SetCut();   //Requires that Name, Cut Value and cut_less_than have been initialised
	void		SetCut(TString name, Double_t CutValue, Int_t CutLower);
  void    PrintCut();
};

void			Variable::SetName(TString name)					{ var_name = name;						return; }
void			Variable::SetCutVal(Double_t val)				{ var_cut_val = val;					return; }
void			Variable::SetLimits(Double_t min, Double_t max) { var_min = min; var_max = max;			return; }
void			Variable::SetCutDir(Int_t myint)				{ cut_less_than = myint;				return; }
void			Variable::SetNext(Variable* next)				{ next_var = next;						return; }
void			Variable::SetHasMC(Int_t hasmc)					{ hasMonteCarlo = hasmc;				return; }
void			Variable::SetValBkg(Double_t bval)				{ bkg_value = bval;						return; }
void			Variable::SetValMC(Float_t mval)				{ mc_value = mval;						return; }
void			Variable::SetDerived()						     	{ derived = kTRUE;						return; }
void			Variable::EnableBranch(TTree *tree)				{ tree->SetBranchStatus(var_name,1);	return; }
void			Variable::SetVect(TLorentzVector v)				{ myvect = v;							return; }
void      Variable::NotDerived()  { derived = kFALSE; return; }
void      Variable::SetType(TString type) {var_type = type; return; }
TString			Variable::GetName()		{ return(var_name); }
Double_t		Variable::GetBkgVal()	{ return(bkg_value); }
Float_t		Variable::GetMCVal()	{ return(mc_value); }
Double_t		Variable::GetCutVal()	{ return(var_cut_val); }
Int_t			Variable::GetCutDir()	{ return(cut_less_than); }
Variable*		Variable::GetNext()		{ return(next_var); }
Double_t		Variable::GetMin()		{ return(var_min); }
Double_t		Variable::GetMax()		{ return(var_max); }
Int_t			Variable::HasMC()		{ return(hasMonteCarlo); }
Bool_t			Variable::IsDerived()	{ return(derived); }
TCut			Variable::GetCut()		{ return(mycut); }
TLorentzVector	Variable::GetVect()		{ return(myvect); }
TString  Variable::GetType()    { return(var_type); }

void Variable::DerivedSetup(TString name, Double_t cut, Int_t cut_dir, Int_t MC) {
	var_name = name;
	var_cut_val = cut;
	cut_less_than = cut_dir;
	hasMonteCarlo = MC;
	derived = kTRUE;
	return;
}
void Variable::SetCut() {
   TString value;
   TString equality;
	if(var_type!="bool") {
       value = patch::to_string(var_cut_val);
	     if (cut_less_than) equality = "<";
	     else			   equality = ">";
  }
  else {
       value = patch::to_string(1);
       if( cut_less_than) equality = "!=";
       else equality = "==";
  }

	mycut = var_name + equality + value;
      Variable::PrintCut();
	return;
}
void Variable::SetCut(TString name, Double_t CutValue, Int_t CutLower) {
	TString value = patch::to_string(CutValue);
	TString equality;
	if (CutLower) equality = "<";
	else		  equality = ">";

	mycut = name + equality + value;
	return;
}

void Variable::PrintCut() {
  if(!mycut) {
      std::cout << var_name;
      if(cut_less_than) std::cout << "<";
      else              std::cout << ">";
      std::cout << var_cut_val << std::endl;
  }
  else mycut.Print();
}



#endif
