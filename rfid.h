////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						rfid.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de rid.c		////
////														////
////	Créé le : 15/01/2011								////
////	Modifié le : 15/01/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef rfid_h
#define rfid_h

//// Prototypes ////
void rfid_init(void);
void rfid_scanner(unsigned char rfid_type_tag);
void rfid_taches(void);

//// Définitions ////
extern rfid rfid_buffer;					// Variable de type RFID de transition (lors de la réception)
extern unsigned char rfid_compteur_for;		// Permet de compter lors d'une boucle FOR
extern struct 
{
	unsigned attente_utilisateur:1;			// Flag qui signale que l'on attend un tag RFID d'utilisateur
	unsigned attente_vehicule:1;			// Flag qui signale que l'on attend un tag RFID de véhicule
	unsigned scan_ok:1;						// Flag qui signale que le scan est terminé
} rfid_statut;			// Structure de statut du module RFID
#endif