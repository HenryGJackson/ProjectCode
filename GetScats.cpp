#include "/home/epp/phunch/Documents/Useful/Variable.h"

void GetScats () {
  Variable *seed = GetVariables();
  Variable *list = seed;
  Variable *list2 = seed;
  
  while(list) {
  //  list->AnalyseCuts_vals(0.2,10);
    while(list2) {
      if(list->m_name != list2->m_name){
	list->CreateScats (list2);
	FillScats(list,list2,kFALSE);
	list->c = new TCanvas((((list->m_name)+(list2->m_name)).c_str()),(((list->m_name)+(list2->m_name)).c_str()),800,600);
	list->m_scat_bkg->SetMarkerColor(kRed);
	list->m_scat_bkg->DrawNormalized();
	list->m_scat_mc->DrawNormalized("same");
      }
    list2 = list2->m_next;
    }
    list2 = list->m_next;
    list = list->m_next;
   }
}
    
  