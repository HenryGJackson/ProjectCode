#include "/home/epp/phunch/Documents/Useful/Variable.h"

void GetCutCurve() {
  SetupCut(); 
  Variable *seed = GetVariables();
  Variable *list = seed;
  Variable *list2 = seed;
  SetupCut();
  while(list) {
    list->AnalyseCuts_vals(0.2,20);
    list->c = new TCanvas((list->m_name).c_str(),(list->m_name).c_str(),800,400);
    list->c->Divide(2);
    list->c->cd(1);
    list->effScatDo->Draw();
    list->c->cd(2);
    list->effScatUp->SetLineColor(kRed);
    list->effScatUp->Draw();
    list = list->m_next;
  }
}