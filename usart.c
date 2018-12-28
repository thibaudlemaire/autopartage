////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de routines pour les			////
////				  communications séries					////
////				  ( 1 : Réception ; 2 : Récep/Emiss )	////
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
unsigned char usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1]; 	// Buffer de réception du port série 1
unsigned char usart_buffer_recep_2[USART_TAILLE_BUFFER_RECEP_2]; 	// Buffer de réception du port série 2
unsigned char usart_buffer_emiss_2[USART_TAILLE_BUFFER_EMISS_2]; 	// Buffer d'émission du port série 2
unsigned char *usart_ptr_buffer_recep_1_e;							// Poiteur d'écriture du buffer de reception du port série 1
unsigned char *usart_ptr_buffer_recep_1_l;							// Poiteur de lecture du buffer de reception du port série 1
unsigned char *usart_ptr_buffer_recep_2_e;							// Poiteur d'écriture du buffer de reception du port série 2
unsigned char *usart_ptr_buffer_recep_2_l;							// Poiteur de lecture du buffer de reception du port série 2
unsigned char *usart_ptr_buffer_emiss_2_e;							// Poiteur d'écriture du buffer d'émission du port série 2
unsigned char *usart_ptr_buffer_emiss_2_l;							// Poiteur de lecture du buffer d'émission du port série 2
struct
{
	unsigned nbr_octets_recus_1									: 4;		// Nombre d'octets présent dans le buffer de réception du port 1
	unsigned is_car_buffer_2									: 1;		// Paquet entier dans le buffer 2
} usart_etat_bits;	

////////////////////////////////////////////////////////////////
////	Fonction usart_init(void)							////
////	Description : Initialise les ports séries			////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_init(void)
{
	//// Pointeurs de buffers ////
	usart_ptr_buffer_recep_1_e = usart_buffer_recep_1;	// Mise à zéro des pointeurs...
	usart_ptr_buffer_recep_1_l = usart_buffer_recep_1;
	usart_ptr_buffer_recep_2_e = usart_buffer_recep_2;
	usart_ptr_buffer_recep_2_l = usart_buffer_recep_2;
	usart_ptr_buffer_emiss_2_e = usart_buffer_emiss_2;
	usart_ptr_buffer_emiss_2_l = usart_buffer_emiss_2;
	
	//// Pattes de communication ////
	TRISCbits.TRISC7 = 1;	// RX - En entrée
	TRISCbits.TRISC6 = 1;	// TX - En entrée bien que se soit une sortie
	TRISBbits.TRISB7 = 1; 	// RX - En entrée
	TRISBbits.TRISB6 = 1;	// TX - En entrée bien que se soit une sortie
	ANSELCbits.ANSC7 = 0;	// Patte non annalogique
	
	//// Baudrate générator ////
	BAUDCON1 = 0;			// Baudrate en mode 8 bits, aucune options
	BAUDCON2 = 0;			// Baudrate en mode 8 bits, aucune options
	SPBRG1 = 25; 	                // Configure la vitesse (BAUD) 9600
	SPBRG2 = 25; 	                // Configure la vitesse (BAUD) 9600
	
	//// Modules USART ////
	// PMD
	PMD0bits.UART1MD = 0;			// On laisse le UART1 sous tension
	PMD0bits.UART2MD = 0;			// On laisse le UART2 sous tension
	// UART
	RCSTA1 = 0b10010000;			// Active l'USART  CREN=1 et SPEN=1
	RCSTA2 = 0b10010000;			// Active l'USART  CREN=1 et SPEN=1
    TXSTA1 = 0b10100000;			// Config du module USART 1, en emmission 	  
	TXSTA2 = 0b10000000;			// Module emission n°2 désactivé
   	
    //// Interruptions ////
	// Emission
    PIR1bits.TX1IF = 0;				// Mise à zéro des drapeaux
	PIR3bits.TX2IF = 0;
	IPR1bits.TX1IP = 0;				// Selectionne basse priorité pour TX (vecteur en 0x18), inutile
	IPR3bits.TX2IP = 0;
	PIE1bits.TX1IE = 0;				// IT en emission désactivée, sera activé par la fonction "usart_envoyer_car()"
	PIE3bits.TX2IE = 0;				// IT toujours désactivée
	// Réception
	PIR1bits.RC1IF = 0;				// Drapeaux
	PIR3bits.RC2IF = 0;
	IPR1bits.RC1IP = 0;				// Sélectionne basse priorité pour RX (vecteur en 0x18)
	IPR3bits.RC2IP = 0;
	PIE1bits.RC1IE = 1;				// IT en réception activées
	PIE3bits.RC2IE = 1;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_etat_buffer_recep(unsigned char usart_portserie)			////
////	Description : Indique si un carractère est disponible dans un buffer	////
////	Paramètres : usart_portserie : Buffer à vérifier (1 ou 2)				////
////	Retour : 0, aucun carractère disponible									////
////			 1ou+, un ou plusieurs carractère sont disponibles				////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
unsigned char usart_etat_buffer_recep(unsigned char usart_portserie)
{
	if(usart_portserie == 1) // Port série 1
	{
		return (usart_ptr_buffer_recep_1_e - usart_ptr_buffer_recep_1_l); // Pointeur d'ecriture moins pointeur lecture
	}
	else	// Port série 2
	{
		return (usart_ptr_buffer_recep_2_e - usart_ptr_buffer_recep_2_l); // Pointeur d'ecriture moins pointeur lecture
	}
}

////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_car(unsigned char usart_portserie)		////
////	Description : Renvoi le carractère le plus ancien disponible	////
////				  dans un buffer									////
////	Paramètres : usart_portserie, buffer à utiliser (1 ou 2)		///////////////////////////////////////////////////// Voir cas du zéro !!!!!!!!!!!!!!!!
////	Retour : le carractère le plus ancien du buffer selectionné		////
////			 ou 0 si aucun caractère n'est disponible				////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
unsigned char usart_recevoir_car(unsigned char usart_portserie)
{
	unsigned char usart_car_retour;	// Carractère à renvoyer
	if(usart_portserie == 1) // Port série 1
	{
		usart_car_retour = *usart_ptr_buffer_recep_1_l++;	// On lit le caractère puis on incrémente		
		usart_gestion_debordements_1();						// On corrige les éventuels débordements
	}
	else // Port série 2
	{
		usart_car_retour = *usart_ptr_buffer_recep_2_l++;			
		usart_gestion_debordements_2_recep();
	}
	return (usart_car_retour);								// On retourne le caractère lu.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin, unsigned char usart_portserie)	////
////	Description : Réceptionne une chaine dans le buffer sélectionné et la termine par NULL (0)								////
////	Paramètres : *usart_cible, pointeur sur l'emplacement mémoire de réception												////
////				 usart_car_fin, carractère de fin de chaine																	////
////				 usart_portserie, choix du buffer (1 ou 2)																	////
////	Retour : un pointeur vers la chaine																						////
////	Auteur : TL																												////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin, unsigned char usart_portserie)
{
	unsigned char *usart_ptr_retour = usart_cible;	// Définition du pointeur de retour
	unsigned char usart_caractere_temporaire;		// Octet dans lequel on stocke le caractère temporaire

	while ((usart_caractere_temporaire = usart_recevoir_car(usart_portserie)) != usart_car_fin)	// Tant qu'on n'a pas obtenu le caractère de fin
		*usart_cible++ = usart_caractere_temporaire; // On le place à l'emplacement cible puis on incrémente
	*usart_cible = 0;			// Enfin, on termine par le caractère de fin
	return (usart_ptr_retour);	// Et on retourne le pointeur de début de chaine
}

////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_car(unsigned char usart_car)	////
////	Description : Envoi un caractère sur le port série	////
////				  numéro 2								////
////	Paramètres : usart_caractere, caractère à envoyer	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_envoyer_car(unsigned char usart_caractere)
{
	*usart_ptr_buffer_emiss_2_e++ = usart_caractere; // On place le caractère à envoyer dans le buffer de sortie, qu'on incrémente
	usart_gestion_debordements_2_emiss();	// Puis on corrige les éventuels débordements
	PIE1bits.TX1IE = 1;		// Activation des interruptions, si le module est libre, cela aura pour effet de générer une interruption
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)			////
////	Description : Envoie une chaine de caractères ( !!!  PRESENTE EN RAM  !!! )						////
////	Paramètres : *usart_chaine, poiteur vers le premier caractère									////
////				 usart_car_fin, caractère de fin de chaine, non émit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)
{
	while (*usart_chaine != usart_car_fin) // Tant qu'on n'a pas le caractère de fin de chaine
	{
		usart_envoyer_car(*usart_chaine);	// On émet le caractère
		usart_chaine++;	// Et on incrémente le pointeur
	}
}

////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_1(void)			////
////	Description : Détecte et corrige les débordements 	////
////				  du buffer de réception du port 1		////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_gestion_debordements_1(void)
{
	if (usart_ptr_buffer_recep_1_l >= &usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1]) 	// Si le buffer déborde
		usart_ptr_buffer_recep_1_l = usart_buffer_recep_1;									// On remet à zéro
	if (usart_ptr_buffer_recep_1_e >= &usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1])	
		usart_ptr_buffer_recep_1_e = usart_buffer_recep_1;
}

////////////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_2_recep(void)			////
////	Description : Détecte et corrige les débordements du buffer ////
////				  de réception du port série 2					////
////	Paramètres : Aucun											////
////	Retour : Rien												////
////	Auteur : TL													////
////////////////////////////////////////////////////////////////////////
void usart_gestion_debordements_2_recep(void)
{
	if (usart_ptr_buffer_recep_2_l >= &usart_buffer_recep_2[USART_TAILLE_BUFFER_RECEP_2])	
		usart_ptr_buffer_recep_2_l = usart_buffer_recep_2;
	if (usart_ptr_buffer_recep_2_e >= &usart_buffer_recep_2[USART_TAILLE_BUFFER_RECEP_2])	
		usart_ptr_buffer_recep_2_e = usart_buffer_recep_2;
}

////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_2_emiss(void)	////
////	Description : Détecte et corrige les débordements	////
////				  du buffer d'émission du port 2		////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_gestion_debordements_2_emiss(void)
{
	if (usart_ptr_buffer_emiss_2_l >= &usart_buffer_emiss_2[USART_TAILLE_BUFFER_EMISS_2])	
		usart_ptr_buffer_emiss_2_l = usart_buffer_emiss_2;
	if (usart_ptr_buffer_emiss_2_e >= &usart_buffer_emiss_2[USART_TAILLE_BUFFER_EMISS_2])	
		usart_ptr_buffer_emiss_2_e = usart_buffer_emiss_2;
}

////////////////////////////////////////////////////////////////
////	Fonction usart_interrupt(void)						////
////	Description : Gère les interruption dues aux ports	////
////				  séries								////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_interrupt(void)
{
	if(PIR1bits.TX1IF && PIE1bits.TX1IE)
	{
		if(usart_ptr_buffer_emiss_2_l != usart_ptr_buffer_emiss_2_e)		// Si un carractère est présent dans le buffer
		{	TXREG1 = *usart_ptr_buffer_emiss_2_l++;			// On envoi le carractère le plus ancien du buffer
			usart_gestion_debordements_2_emiss();			// Gestion des débordements
		}
		else 
			PIE1bits.TX1IE = 0;		// On désactive cette source d'interruption
	}
	if(PIR3bits.RC2IF)		// Interruption de reception du port série 1 (Module 2 cf. REM)
	{
		*usart_ptr_buffer_recep_1_e = RCREG2;		// Copie du registre de réception
		usart_etat_bits.nbr_octets_recus_1++;		// On incrémente le nombre d'octet reçus
		usart_ptr_buffer_recep_1_e++;				// On incrémente le pointeur
		usart_gestion_debordements_1();				// On vérifie les débordements
	}	
	if(PIR1bits.RC1IF)		// Interruption de réception du port série 2 (Module 1 cf. REM)
	{
		*usart_ptr_buffer_recep_2_e = RCREG1;		// Copie du registre de réception
		if (*usart_ptr_buffer_recep_2_e == 0) 		// Si il s'agit du carractère de fin de trame (0)
			usart_etat_bits.is_car_buffer_2 = 1;	// On met le bits de fin de trame à 1
		usart_ptr_buffer_recep_2_e++;				// On incrémente le pointeur
		usart_gestion_debordements_2_recep();		// On vérifie les débordements
	}	
}