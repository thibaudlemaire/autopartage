////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						serveur.h						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de serveur.c	////
////														////
////	Créé le : 15/01/2011								////
////	Modifié le : 15/01/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef serveur_h
#define serveur_h

//// Defines ////
#define SERVEUR_EMP_AUCUN 0 			// Sera utilisé lors de la demande d'infos sur un emplacement
#define SERVEUR_EMP_SUIVANT 1			// Signifie que l'on veut l'emplacement libre suivant
#define SERVEUR_EMP_PRESEDENT 2			// Signifie que l'on veut l'emplacement libre précédent

//// Prototypes ////
void serveur_init(void);
void serveur_tag(unsigned char * serveur_tag_rfid);
void serveur_utilisateur(unsigned char * serveur_ptr_tag);
void serveur_vehicule(unsigned char * serveur_tag_vehicule);
void serveur_location(unsigned char *serveur_tag_vehicule);
void serveur_restitution(unsigned char * serveur_tag_vehicule);
void serveur_appel(void);
void serveur_taches(void);

//// Définitions ////
extern unsigned char serveur_compteur_for;		// Compteur pour les boucles FOR
extern struct 
{
	unsigned validation:1;						// Flag de réception du message de validation, RAZ par software
	unsigned identification:1;					// Flag de demande d'identification d'un Tag
	unsigned utilisateur:1;						// Flag de réception des infos sur l'utilisateur
	unsigned vehicule:1;						// Flag de réception des infos sur le véhicule
	unsigned erreur:1;							// Flag d'erreur inconnue (Tag non trouvé...)
	unsigned :1;
	unsigned :1;
	unsigned :1;
} serveur_statut;				// Structure de drapeaux à remettre à zéro par SOFT

#endif
