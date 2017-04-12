#ifndef CBPARAMS_H
#define CBPARAMS_H

#include <iostream>
#include <cstdlib>
#include <math.h>

struct CBparams{
 // Parameters
      Double_t cb_mean;
      Double_t cb_sigma;
      Double_t cb_alpha;
      Double_t cb_n;
      Double_t frac;
      Bool_t isInit = kFALSE;
 // Errors
      Double_t cb_merr;
      Double_t cb_serr;
      Double_t cb_aerr;
      Double_t cb_nerr;
      Double_t cb_ferr;
      Bool_t hasErr = kFALSE;
 // Function values
      Double_t cb_A;
      Double_t cb_B;
      Double_t cb_C;
      Double_t cb_D;
      Double_t cb_N;
      Bool_t gotABCDN = kFALSE;
 // Other Crystal Ball parameters in the fit
      CBparams* partner;
      Bool_t hasPartner = kFALSE;
 // Fraction used in the fit
      Double_t cb_frac; //frac is shared by both crystal ball functions in the fit
                     //so only vary the fraction from one of the CBs
      Bool_t hasFrac = kFALSE;
};

//Set the parameters of the CB
void SetParams(CBparams* cbp, Double_t mean, Double_t sigma, Double_t alpha, Double_t n){
      cbp->cb_mean = mean;
      cbp->cb_sigma = sigma;
      cbp->cb_alpha = alpha;
      cbp->cb_n = n;
      cbp->isInit = kTRUE;
      return;
}

//Set Errors on the parameters
void SetErrors(CBparams* cbp, Double_t mean, Double_t sigma, Double_t alpha, Double_t n){
      cbp->cb_merr = mean;
      cbp->cb_serr = sigma;
      cbp->cb_aerr = alpha;
      cbp->cb_nerr = n;
      cbp->hasErr = kTRUE;
      return;
}

//Sets the fraction to both sets of parameters for this fit.
void SetFrac(CBparams* cbp1, CBparams* cbp2, Double_t frac){
      cbp1->cb_frac = frac;
      cbp2->cb_frac = frac;
      cbp1->hasFrac = kTRUE;
      cbp2->hasFrac = kTRUE;
      return;
}

//Sets a pointer to the set of parameters for the other crystal ball in the fit
void SetPartnerLink(CBparams* cbp1, CBparams* cbp2) {
      cbp1->partner = cbp2;
      cbp2->partner = cbp1;
      if(cbp1->partner) cbp1->hasPartner = kTRUE;
      if(cbp2->partner) cbp2->hasPartner = kTRUE;
      return;
}

//Logs are taken in all calculations to avoid using std::pow() for high powers which breaks
//Calculates the value of A
void CalcA(CBparams *cbp) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return;}
      if(cbp->cb_alpha==0.0) {std::cout << "Error, alpha == 0\n"; return;}
      if(cbp->cb_alpha<0) cbp->cb_alpha = (-1)*cbp->cb_alpha;

      Double_t val = cbp->cb_n*(log(cbp->cb_n)-log(cbp->cb_alpha)) - (0.5)*cbp->cb_alpha*cbp->cb_alpha;
      cbp->cb_A = exp(val);
      return;
}

//Calculates B
void CalcB(CBparams *cbp) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return;}
      if(cbp->cb_alpha==0.0) {std::cout << "Error, alpha == 0\n"; return;}
      if(cbp->cb_alpha<0) cbp->cb_alpha = (-1)*cbp->cb_alpha;

      cbp->cb_B = (cbp->cb_n/cbp->cb_alpha)-cbp->cb_alpha ;
      return;
}

//Calculates C
void CalcC(CBparams *cbp) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return;}
      if(cbp->cb_alpha==0.0) {std::cout << "Error, alpha == 0\n"; return;}
      if(cbp->cb_alpha<0) cbp->cb_alpha = (-1)*cbp->cb_alpha;

      Double_t val = log(cbp->cb_n) - log(cbp->cb_alpha) - log(cbp->cb_n - 1) - (0.5*cbp->cb_alpha*cbp->cb_alpha);
      cbp->cb_C = exp(val);
      return;
}

//Calculates D
void CalcD(CBparams *cbp) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return;}
      if(cbp->cb_alpha==0.0) {std::cout << "Error, alpha == 0\n"; return;}
      if(cbp->cb_alpha<0) cbp->cb_alpha = (-1)*cbp->cb_alpha;

      Double_t val = 0.5*(log(3.1415926)-log(2)) + log(1 + erf(cbp->cb_alpha)/sqrt(2));
      cbp->cb_D = exp(val);
}

//Calculates C,D,N
void CalcN(CBparams *cbp) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return;}
      CalcC(cbp);
      CalcD(cbp);
      cbp->cb_N =  1/(cbp->cb_sigma*(cbp->cb_C + cbp->cb_D)) ;
      return;
}

//Calculates all of A,B,C,D,N
void CalcABN(CBparams *cbp) {
      CalcA(cbp);
      CalcB(cbp);
      CalcN(cbp);
      cbp->gotABCDN = kTRUE;
      return;
}

//Gets   (m - m_bar)sigma
Double_t getClassifier(CBparams *cbp, Double_t x) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return(0.0);}
      return((x - cbp->cb_mean)/cbp->cb_sigma);
}

//Calculates the value of the function at mass x.
//Remember this is a PDF, so needs multiplier to convert to number of events.
//Also not sure this even works properly
Double_t CalcVal(CBparams *cbp, Double_t x) {
      if(!cbp->isInit) {std::cout << "CBparams un-initialised\n"; return(0.0);}
      if(!cbp->gotABCDN) CalcABN(cbp);
      Double_t classifier = getClassifier(cbp,x);
      Double_t val;
      if(classifier <= (-1)*cbp->cb_alpha) {
            val = log(cbp->cb_N) + log(cbp->cb_A) - cbp->cb_n*log(cbp->cb_B - classifier);
            return(exp(val));
      }
      else{
            val = log(cbp->cb_N) - (classifier*classifier*0.5);
            return(exp(val));
      }
}


#endif
