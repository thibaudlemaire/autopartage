////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					interrupts.h						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclarations de 			////
////				  interrutps.c							////
////														////
////	Créé le : 04/12/2010								////
////	Modifié le : 10/01/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef interrupt_h
#define interrupt_h

//// Prototypes //// 
void it_init(void);
void it_h_prio(void);
void it_b_prio(void);

#endif