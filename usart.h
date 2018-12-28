////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de usart.c		////
////														////
////	Créé le : 14/11/2010								////
////	Modifié le : 01/02/2010								////
////	Support : PIC 18F24K22  -  16 MHz  					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////
////	REM : Le module USART 1 est appelé Port Série 2		////
////		  par erreur de conception. Il en va de même	////
////		  pour le module USART 2 qui est appelé Port	////
////		  série 1. Cette erreur n'altère pas le 		////
////		  fonctionnement.								////
////////////////////////////////////////////////////////////////

#ifndef usart_h
#define usart_h

//// Defines ////
#define USART_TAILLE_BUFFER_RECEP_1	15 // Taille maximum des paquets en provenance du module UM-005
#define USART_TAILLE_BUFFER_RECEP_2	55 // Taille maximum des paquets du serveur
#define USART_TAILLE_BUFFER_EMISS_2 40 // Taille maximum des paquets à émettre

//// Prototypes ////
void usart_init(void);
unsigned char usart_etat_buffer_recep(unsigned char usart_portserie);
unsigned char usart_recevoir_car(unsigned char usart_portserie);
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin, unsigned char usart_portserie);
void usart_envoyer_car(unsigned char usart_caractere);
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin);
void usart_gestion_debordements_1(void);
void usart_gestion_debordements_2_emiss(void);
void usart_gestion_debordements_2_recep(void);
void usart_interrupt(void);

//// Définitions ////
extern unsigned char usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1]; 	// Buffer de réception du port série 1
extern unsigned char usart_buffer_recep_2[USART_TAILLE_BUFFER_RECEP_2]; 	// Buffer de réception du port série 2
extern unsigned char usart_buffer_emiss_2[USART_TAILLE_BUFFER_EMISS_2]; 	// Buffer d'émission du port série 2
extern unsigned char *usart_ptr_buffer_recep_1_e;							// Poiteur d'écriture du buffer de reception du port série 1
extern unsigned char *usart_ptr_buffer_recep_1_l;							// Poiteur de lecture du buffer de reception du port série 1
extern unsigned char *usart_ptr_buffer_recep_2_e;							// Poiteur d'écriture du buffer de reception du port série 2
extern unsigned char *usart_ptr_buffer_recep_2_l;							// Poiteur de lecture du buffer de reception du port série 2
extern unsigned char *usart_ptr_buffer_emiss_2_e;							// Poiteur d'écriture du buffer d'émission du port série 2
extern unsigned char *usart_ptr_buffer_emiss_2_l;							// Poiteur de lecture du buffer d'émission du port série 2
extern struct
{
	unsigned nbr_octets_recus_1									: 4;		// Nombre d'octets présent dans le buffer de réception du port 1
	unsigned is_car_buffer_2									: 1;		// Paquet entier dans le buffer 2
} usart_etat_bits;	

#endif
