////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						main.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de d�clarations de main.c		////
////														////
////	Cr�� le : 14/11/2010								////
////	Modifi� le : 04/12/2010								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef main_h
#define main_h

//// Defines ////
#define TEMPO_ERREUR 6
#define TEMPO_INIT 2
#define TEMPO_SERVEUR 1
#define TEMPO_MESSAGE 4
#define TEMPO_SCAN 20
#define TEMPO_TOUCHE 20
#define TEMPO_CLE 20

//// D�finitions ////
typedef unsigned char rfid[5];
extern rfid tag_vehicule_scanne;	// Tag du v�hicule scann� lors d'une restituion
extern rfid tag_temporaire;		// Tag scann�, temporaire
extern rfid tags_emplacements[6];	// Tags RFID aux differents emplacements, de 0 � 5, le z�ro n'�tant pas utilis�

// A optimiser !
extern unsigned char compteur_for;	// Compteur pour boucle for
extern unsigned char is_vehicules;	// Variable locale de nombre de vehicule

extern struct
{
	unsigned char nom[17];		// Le nom de l'utilisateur, maximum 16 carract�re + 1 pour le carract�re de fin
	rfid tag_rfid;				// Le tag RFID de l'utilisateur en cours
	unsigned statut:1;			// Statut de l'utilisateur, 0 : aucun, 1 : en location
} utilisateur; 			// Utilisateur en cours
extern struct
{
	unsigned char nom[17];				// Nom du v�hicule, max 16 + 1 carr. de fin
	unsigned char prix[7]; 				// Prix horraire du v�hicule, max 6 + 1 carr. de fin
	unsigned char immatriculation[9]; 	// Immatriculation du v�hicule, max 8 + 1 carr. de fin
	unsigned char emplacement[17]; 		// Emplacement, 16 + 1
} vehicule; 			// V�hicule en cours
extern struct
{
	unsigned erreur					:1;		// Bit d'erreur
	unsigned erreur_delais			:1;		// Erreur due � un d�lais trop long
	unsigned erreur_tag				:1;		// Tag non reconnu
	unsigned erreur_zero_vehicule 	:1;		// Erreur : pas de vehicule disponible
	unsigned transaction			:1;		// Bit de transaction en cours
	unsigned vehicule				:3;		// Compteur de choix de v�hicule (location) de 1 � 5
} cycle;				// Structure de cycle

#endif