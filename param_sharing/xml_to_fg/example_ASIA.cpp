#include <dai/factorgraph.h>
#include <iostream>
#include <fstream>
using namespace std;
using namespace dai;
int main() {
Var VisitAsia(0,0);
Var Tuberculosis(1,0);
Var Smoking(2,0);
Var Cancer(3,0);
Var TbOrCa(4,0);
Var XRay(5,0);
Var Bronchitis(6,0);
Var Dyspnea(7,0);
Factor P_VisitAsia(  VisitAsia  );
P_VisitAsia.set(0, 0.01);
P_VisitAsia.set(1, 0.99);
std::vector<Var> a1;
a1.push_back( VisitAsia );
Factor P_Tuberculosis_given_VisitAsia( VarSet( a1.begin(), a1.end() )  | Tuberculosis );
P_Tuberculosis_given_VisitAsia.set(0, 0.05);
P_Tuberculosis_given_VisitAsia.set(1, 0.01);
P_Tuberculosis_given_VisitAsia.set(2, 0.95);
P_Tuberculosis_given_VisitAsia.set(3, 0.99);
Factor P_Smoking(  Smoking  );
P_Smoking.set(0, 0.5);
P_Smoking.set(1, 0.5);
std::vector<Var> a3;
a3.push_back( Smoking );
Factor P_Cancer_given_Smoking( VarSet( a3.begin(), a3.end() )  | Cancer );
P_Cancer_given_Smoking.set(0, 0.1);
P_Cancer_given_Smoking.set(1, 0.01);
P_Cancer_given_Smoking.set(2, 0.9);
P_Cancer_given_Smoking.set(3, 0.99);
std::vector<Var> a4;
a4.push_back( Tuberculosis );
a4.push_back( Cancer );
Factor P_TbOrCa_given_Tuberculosis_Cancer( VarSet( a4.begin(), a4.end() )  | TbOrCa );
P_TbOrCa_given_Tuberculosis_Cancer.set(0, 1.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(1, 1.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(2, 1.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(3, 0.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(4, 0.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(5, 0.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(6, 0.0);
P_TbOrCa_given_Tuberculosis_Cancer.set(7, 1.0);
std::vector<Var> a5;
a5.push_back( TbOrCa );
Factor P_XRay_given_TbOrCa( VarSet( a5.begin(), a5.end() )  | XRay );
P_XRay_given_TbOrCa.set(0, 0.98);
P_XRay_given_TbOrCa.set(1, 0.05);
P_XRay_given_TbOrCa.set(2, 0.02);
P_XRay_given_TbOrCa.set(3, 0.95);
std::vector<Var> a6;
a6.push_back( Smoking );
Factor P_Bronchitis_given_Smoking( VarSet( a6.begin(), a6.end() )  | Bronchitis );
P_Bronchitis_given_Smoking.set(0, 0.6);
P_Bronchitis_given_Smoking.set(1, 0.3);
P_Bronchitis_given_Smoking.set(2, 0.4);
P_Bronchitis_given_Smoking.set(3, 0.7);
std::vector<Var> a7;
a7.push_back( TbOrCa );
a7.push_back( Bronchitis );
Factor P_Dyspnea_given_TbOrCa_Bronchitis( VarSet( a7.begin(), a7.end() )  | Dyspnea );
P_Dyspnea_given_TbOrCa_Bronchitis.set(0, 0.9);
P_Dyspnea_given_TbOrCa_Bronchitis.set(1, 0.7);
P_Dyspnea_given_TbOrCa_Bronchitis.set(2, 0.8);
P_Dyspnea_given_TbOrCa_Bronchitis.set(3, 0.1);
P_Dyspnea_given_TbOrCa_Bronchitis.set(4, 0.1);
P_Dyspnea_given_TbOrCa_Bronchitis.set(5, 0.3);
P_Dyspnea_given_TbOrCa_Bronchitis.set(6, 0.2);
P_Dyspnea_given_TbOrCa_Bronchitis.set(7, 0.9);
 vector<Factor> ASIAFactors;FactorGraph ASIANetwork( ASIAFactors );
ASIANetwork.WriteToFile( "ASIA.fg");
return 0; }
