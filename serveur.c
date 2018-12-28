////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						serveur.c						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de commnication avec le 		////
////				  serveur.								////
////														////
////	Créé le : 15/01/2011								////
////	Modifié le : 15/01/2011								////
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

//// Définitions ////
unsigned char serveur_compteur_for;			// Compteur pour les boucles FOR
struct 
{
	unsigned validation:1;						// Flag de réception du message de validation
	unsigned identification:1;					// Flag de demande d'identification d'un Tag
	unsigned utilisateur:1;						// Flag de réception des infos sur l'utilisateur
	unsigned vehicule:1;						// Flag de réception des infos sur le véhicule
	unsigned erreur:1;							// Flag d'erreur inconnue (Tag non trouvé...)
	unsigned :1;
	unsigned :1;
	unsigned :1;
} serveur_statut;			// Structure de drapeaux à remettre à zéro par SOFT

////////////////////////////////////////////////////////////////
////	Fonction serveur_init(void)							////
////	Description : Initialise les communications avec le	////
////				  serveur.								////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_init(void)
{
	serveur_statut.validation = 0;			// Mise à zéro des flags
	serveur_statut.identification = 0; 
	serveur_statut.utilisateur = 0;
	serveur_statut.vehicule = 0;
	serveur_statut.erreur = 0;
}

////////////////////////////////////////////////////////////////
////	Fonction serveur_tag(rfid)							////
////	Description : Répond à une demande d'identification	////
////	Paramètres : serveur_tag_rfid, ptr vers le tag		////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_tag(unsigned char * serveur_tag_rfid)
{
	usart_envoyer_car(97);							// Numerot de la borne, "a"
	usart_envoyer_car(84);							// Fonction "Tag", "T"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID
	{
		usart_envoyer_car(*serveur_tag_rfid++);
	}
	usart_envoyer_car(36);							// Fin de paramètre "$"
	usart_envoyer_car(0);							// Fin de trame
}

////////////////////////////////////////////////////////////////
////	Fonction serveur_utilisateur(unsigned char *)		////
////	Description : Demande des informations sur un user	////
////	Paramètres : serveur_tag_utilisateur, tag rfid		////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_utilisateur(unsigned char * serveur_ptr_tag)
{
	usart_envoyer_car(97);							// Numerot de la borne, "a"
	usart_envoyer_car(85);							// Fonction "Utilisateur", "U"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID
	{
		usart_envoyer_car(*serveur_ptr_tag++);
	}
	usart_envoyer_car(36);							// Fin de paramètre "$"
	usart_envoyer_car(0);							// Fin de trame
}

////////////////////////////////////////////////////////////////
////	Fonction serveur_vehicule(rfid)						////
////	Description : Demande des information sur un 		////
////				  véhicule								////
////	Paramètres : serveur_tag_vehicule, tag RFID de la 	////
////				 clé du véhicule						////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_vehicule(unsigned char * serveur_tag_vehicule)
{
	usart_envoyer_car(97);							// Numerot de la borne, "a"
	usart_envoyer_car(68);							// Fonction "Demande", "D"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID
	{
		usart_envoyer_car(*serveur_tag_vehicule++);
	}
	usart_envoyer_car(36);							// Fin de paramètre "$"
	usart_envoyer_car(0);							// Fin de trame
}

////////////////////////////////////////////////////////////////
////	Fonction serveur_location(rfid)						////
////	Description : Signale la location d'un véhicule		////
////	Paramètres : serveur_tag_vehicule, tag du véhicule	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_location(unsigned char * serveur_tag_vehicule)
{
	usart_envoyer_car(97);							// Numerot de la borne, "a"
	usart_envoyer_car(76);							// Fonction "Location", "L"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID du véhicule
	{
		usart_envoyer_car(*serveur_tag_vehicule++);
	}
	usart_envoyer_car(36);							// Fin de paramètre "$"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID de l'utilisateur
	{
		usart_envoyer_car(utilisateur.tag_rfid[serveur_compteur_for]);
	}
	usart_envoyer_car(36);							// Fin de paramètre "$"
	usart_envoyer_car(0);							// Fin de trame
}

////////////////////////////////////////////////////////////////
////	Fonction serveur_restitution(rfid, unsigend char)	////
////	Description : Signale la restitution d'une clé		////
////	Paramètres : serveur_tag_vehicule, tag rfid du 		////
////				 véhicule.								////
////				 serveur_emplacement_vehicule			////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void serveur_restitution(unsigned char * serveur_tag_vehicule)
{
	usart_envoyer_car(97);								// Numerot de la borne, "a"
	usart_envoyer_car(67);								// Fonction "Clé", "C"
	for (serveur_compteur_for=0; serveur_compteur_for<=4; serveur_compteur_for++)	// Boucle qui envoie le tag RFID
	{
		usart_envoyer_car(*serveur_tag_vehicule++);
	}
	usart_envoyer_car(36);								// Fin de paramètre "$"
	usart_envoyer_car(0);								// Fin de trame
}

/////////////////////////////////////////////////////////////////
////	Fonction serveur_appel(void)						 ////
////	Description : Signale un problème sur la borne. Ce 	 ////
////				  problème est signalé par l'utilisateur ////
////	Paramètres : Aucun									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void serveur_appel(void)
{
	usart_envoyer_car(97);								// Numerot de la borne, "a"
	usart_envoyer_car(65);								// Fonction "Appel", "A"
	usart_envoyer_car(0);								// Fin de trame
}

/////////////////////////////////////////////////////////////////
////	Fonction serveur_taches(void)						 ////
////	Description : Effectue les communications C/Serveur	 ////
////	Paramètres : Aucun									 ////
////	Retour : Rien										 ////
////	Auteur : TL											 ////
/////////////////////////////////////////////////////////////////
void serveur_taches(void)
{
	if (usart_etat_bits.is_car_buffer_2)		// Si un paquet entier est dans le buffer 2
	{
		//// Déclaration des variables locales ////
		unsigned char fonction;					// Variable dans laquelle sera stoquée le code de la fonction
		//// Traitement ////
		usart_recevoir_car(2);					// Réception du destinataire (buffer 2)
		fonction = usart_recevoir_car(2);		// Réception du code de la fonction
		switch(fonction)						// Switch sur le code de la fonction
		{
			case 73:							// Fonction "I" : Identification du tag RFID	
				serveur_statut.identification = 1;		// Set du drapeau d'identification
			break;
			case 82:							// Fonction "R" : Réception de informations sur l'utilisateur	
				usart_recevoir_chaine(&(utilisateur.nom[0]), 36, 2); 	// Réception et placement du nom de l'utilisateur dans la structure
				utilisateur.statut = usart_recevoir_car(2); 	// On réceptionne l'état de l'utilisateur
				usart_recevoir_car(2);			// On enlève le séparateur ($)
				serveur_statut.utilisateur = 1; 		// On lève le drapeau d'identification de l'utilisateur
			break;
			case 74:							// Fonction "J" : Réception des informations sur un véhicule
				usart_recevoir_chaine(&(vehicule.nom[0]), 36, 2); 	// Réception et placement du nom du véhicule dans la structure
				usart_recevoir_chaine(&(vehicule.prix[0]), 36, 2);	// Réception du prix
				usart_recevoir_chaine(&(vehicule.immatriculation[0]), 36, 2);	// Réception de l'immatriculation
				serveur_statut.vehicule = 1;			// On lève le drapeau du véhicule
			break;
			case 86:							// Fonction "V" : Validation ou confirmation d'un message ou d'une demande
				serveur_statut.validation = 1;			// Set du drapeau de validation
			break;
			case 69:							// Fonction "E" : Erreur
				serveur_statut.erreur = 1;		// Set du drapeau d'erreur
			break;
		}
		usart_recevoir_car(2);					// Réception du carractère de fin de trame
		usart_etat_bits.is_car_buffer_2 = 0;	// On abaisse le drapeau
	}
}