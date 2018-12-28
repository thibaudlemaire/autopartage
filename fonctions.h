////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					fonctions.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclarations de			////
////				  fonctions.c							////
////														////
////	Créé le : 04/12/2010								////
////	Modifié le : 31/01/2010								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef fonctions_h
#define fonctions_h

//// Definitons ////
extern struct
{
	unsigned tempo			:1;				// Flag de la tempo
	unsigned boucle_info_v	:1;				// Boucle info véhicule, bit de sortie
} fonc_diver;								// Structure foure tout	
extern unsigned char compteur_tempo;		// Compteur pour la tempo


//// Prototypes ////
void fonc_init_pic(void);
void fonc_init_routines(void);
void fonc_taches(void);
void fonc_gestion_erreurs(void);
void fonc_set_tempo(unsigned char fonc_tempo);
void fonc_unset_tempo();
void fonc_interrupt_tempo(void);
void fonc_loc_info_vehicule(void);
void fonc_loc_vehicule_debordements(void);
void fonc_raz_flags_var(void);
void fonc_copie_tag(unsigned char *fonc_from, unsigned char *fonc_to);
void fonc_effacer_tag(unsigned char *fonc_tag);
unsigned char fonc_is_tag_empty(unsigned char *fonc_tag);

#endif