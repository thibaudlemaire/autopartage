////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						rfid.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de gestion du module RFID		////
////														////
////	Cr�� le : 15/01/2011								////
////	Modifi� le : 15/01/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

//// Includes ////
#include <p18F24K22.h>
#include "main.h"
#include "lcd.h"
#include "usart.h"
#include "fonctions.h"
#include "interrupts.h"
#include "rfid.h"
#include "serveur.h"
#include "disque.h"

//// D�finitions ////
rfid rfid_buffer;							// Variable de type RFID de transition (lors de la r�ception)
unsigned char rfid_compteur_for;			// Permet de compter lors d'une boucle FOR
struct 
{
	unsigned attente_utilisateur:1;			// Flag qui signale que l'on attend un tag RFID d'utilisateur
	unsigned attente_vehicule:1;			// Flag qui signale que l'on attend un tag RFID de v�hicule
	unsigned scan_ok:1;						// Flag qui signale que le scan est termin�
} rfid_statut;			// Structure de statut du module RFID

////////////////////////////////////////////////////////////////
////	Fonction rfid_init(void)							////
////	Description : Initialise le module RFID				////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void rfid_init(void)
{
	rfid_statut.attente_utilisateur = 0;			// On met les flags � z�ro
	rfid_statut.attente_vehicule = 0;
	rfid_statut.scan_ok = 0;
}

////////////////////////////////////////////////////////////////
////	Fonction rfid_scanner(unsigned char rfid_type_tag)	////
////	Description : Configure le module pour scanner		////
////	Param�tres : rfid_type_tag : 1, utilisateur			////
////								 2, v�hicule			////
////								 0, indef�rent			////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void rfid_scanner(unsigned char rfid_type_tag)
{
	rfid_statut.attente_utilisateur = 0;			// On baisse les drapeaux
	rfid_statut.attente_vehicule = 0;	
	if (rfid_type_tag == 1)							// Si on demande un tag d'utilisateur
		rfid_statut.attente_utilisateur = 1;		// On leve son drapeau
	else if (rfid_type_tag == 2)					// Si on attend un tag de v�hicule
		rfid_statut.attente_vehicule = 1;			//On leve le drapeau
		
	usart_ptr_buffer_recep_1_l = usart_ptr_buffer_recep_1_e;	// On efface le buffer 
	usart_etat_bits.nbr_octets_recus_1 = 0;						// Remise � z�ro du compteur
	rfid_statut.scan_ok = 0;									// Remise a z�ro du drapeau
	
	
	//// Activer la reception !!!!
	
	
}

////////////////////////////////////////////////////////////////
////	Fonction rfid_taches(void)							////
////	Description : G�re les taches des routines RFID		////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void rfid_taches(void)
{
	if(usart_etat_bits.nbr_octets_recus_1 >= 11)				// Si on a re�u une trame complete provenant du module UM-005
	{
		usart_etat_bits.nbr_octets_recus_1 = 0;					// RAZ du compteur
		usart_recevoir_car(1);									// On recoit l'adresse du module
		usart_recevoir_car(1);									// On lit la longueur de la trame
		usart_recevoir_car(1);									// Puis l'identifiant r�ponse
		for(rfid_compteur_for = 0; rfid_compteur_for < 5; rfid_compteur_for++) 	// Execute 5 fois pour les 5 octets
			rfid_buffer[rfid_compteur_for] = usart_recevoir_car(1);				// On int�gre le tag RFID dans la variable de transition
		usart_recevoir_car(1);									// Code d'op�ration : 0xFF
		usart_recevoir_car(1);									// Code de v�rification (inutile)
		usart_recevoir_car(1);									// Suite...
		//// Traitement du tag re�u ////
		if(rfid_statut.attente_utilisateur)
			fonc_copie_tag(rfid_buffer, utilisateur.tag_rfid);					// On transf�re le contenu du buffer dans la structure "Utilisateur"
		else if (rfid_statut.attente_vehicule)
			fonc_copie_tag(rfid_buffer, tag_vehicule_scanne);					// On transf�re le contenu vers la variable restitution de vehicule
		else 
			fonc_copie_tag(rfid_buffer, tag_temporaire);
		rfid_statut.scan_ok = 1;								// Flag de fin de scan
		rfid_statut.attente_vehicule = 0;						// Remise a z�ro des drapeaux
		rfid_statut.attente_utilisateur = 0;
		usart_ptr_buffer_recep_1_l = usart_ptr_buffer_recep_1_e;// On efface le buffer
		
		//// D�sactiver la reception !!!!
		
		
	}
}