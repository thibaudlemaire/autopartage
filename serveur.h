////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						serveur.h						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de d�claration de serveur.c	////
////														////
////	Cr�� le : 15/01/2011								////
////	Modifi� le : 15/01/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef serveur_h
#define serveur_h

//// Defines ////
#define SERVEUR_EMP_AUCUN 0 			// Sera utilis� lors de la demande d'infos sur un emplacement
#define SERVEUR_EMP_SUIVANT 1			// Signifie que l'on veut l'emplacement libre suivant
#define SERVEUR_EMP_PRESEDENT 2			// Signifie que l'on veut l'emplacement libre pr�c�dent

//// Prototypes ////
void serveur_init(void);
void serveur_tag(unsigned char * serveur_tag_rfid);
void serveur_utilisateur(unsigned char * serveur_ptr_tag);
void serveur_vehicule(unsigned char * serveur_tag_vehicule);
void serveur_location(unsigned char *serveur_tag_vehicule);
void serveur_restitution(unsigned char * serveur_tag_vehicule);
void serveur_appel(void);
void serveur_taches(void);

//// D�finitions ////
extern unsigned char serveur_compteur_for;		// Compteur pour les boucles FOR
extern struct 
{
	unsigned validation:1;						// Flag de r�ception du message de validation, RAZ par software
	unsigned identification:1;					// Flag de demande d'identification d'un Tag
	unsigned utilisateur:1;						// Flag de r�ception des infos sur l'utilisateur
	unsigned vehicule:1;						// Flag de r�ception des infos sur le v�hicule
	unsigned erreur:1;							// Flag d'erreur inconnue (Tag non trouv�...)
	unsigned :1;
	unsigned :1;
	unsigned :1;
} serveur_statut;				// Structure de drapeaux � remettre � z�ro par SOFT

#endif
