#include "/home/epp/phunch/Documents/Messing/Variable.h"

void GetCuts () {
  std::cout << "Get Variables" << std::endl;
  Variable *seed = GetVariables();
  Variable *list = &seed;
  Variable *list2 = &seed;
  
  TFile myFile("BMESON.root","RECREATE");
  if(!myFile) std::cerr << "Output file error" << std::endl;

  while(list) {
    while(list2) {
      if(list->m_name != list2->m_name){
	list->CreateScats (&list2);
	FillScats(&list,&list2,kFALSE);
	list->m_scat_mc->Write();
      }
    list2 = list2->m_next;
    }
    list2 = seed;
    list = list->m_next;
   }
   myFile.Close();
}
    
  