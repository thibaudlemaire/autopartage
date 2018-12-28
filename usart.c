////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						usart.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de routines pour les			////
////				  communications s�ries					////
////				  ( 1 : R�ception ; 2 : R�cep/Emiss )	////
////														////
////	Cr�� le : 14/11/2010								////
////	Modifi� le : 01/02/2010								////
////	Support : PIC 18F24K22  -  16 MHz  					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////
////	REM : Le module USART 1 est appel� Port S�rie 2		////
////		  par erreur de conception. Il en va de m�me	////
////		  pour le module USART 2 qui est appel� Port	////
////		  s�rie 1. Cette erreur n'alt�re pas le 		////
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

//// D�finitions ////
unsigned char usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1]; 	// Buffer de r�ception du port s�rie 1
unsigned char usart_buffer_recep_2[USART_TAILLE_BUFFER_RECEP_2]; 	// Buffer de r�ception du port s�rie 2
unsigned char usart_buffer_emiss_2[USART_TAILLE_BUFFER_EMISS_2]; 	// Buffer d'�mission du port s�rie 2
unsigned char *usart_ptr_buffer_recep_1_e;							// Poiteur d'�criture du buffer de reception du port s�rie 1
unsigned char *usart_ptr_buffer_recep_1_l;							// Poiteur de lecture du buffer de reception du port s�rie 1
unsigned char *usart_ptr_buffer_recep_2_e;							// Poiteur d'�criture du buffer de reception du port s�rie 2
unsigned char *usart_ptr_buffer_recep_2_l;							// Poiteur de lecture du buffer de reception du port s�rie 2
unsigned char *usart_ptr_buffer_emiss_2_e;							// Poiteur d'�criture du buffer d'�mission du port s�rie 2
unsigned char *usart_ptr_buffer_emiss_2_l;							// Poiteur de lecture du buffer d'�mission du port s�rie 2
struct
{
	unsigned nbr_octets_recus_1									: 4;		// Nombre d'octets pr�sent dans le buffer de r�ception du port 1
	unsigned is_car_buffer_2									: 1;		// Paquet entier dans le buffer 2
} usart_etat_bits;	

////////////////////////////////////////////////////////////////
////	Fonction usart_init(void)							////
////	Description : Initialise les ports s�ries			////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_init(void)
{
	//// Pointeurs de buffers ////
	usart_ptr_buffer_recep_1_e = usart_buffer_recep_1;	// Mise � z�ro des pointeurs...
	usart_ptr_buffer_recep_1_l = usart_buffer_recep_1;
	usart_ptr_buffer_recep_2_e = usart_buffer_recep_2;
	usart_ptr_buffer_recep_2_l = usart_buffer_recep_2;
	usart_ptr_buffer_emiss_2_e = usart_buffer_emiss_2;
	usart_ptr_buffer_emiss_2_l = usart_buffer_emiss_2;
	
	//// Pattes de communication ////
	TRISCbits.TRISC7 = 1;	// RX - En entr�e
	TRISCbits.TRISC6 = 1;	// TX - En entr�e bien que se soit une sortie
	TRISBbits.TRISB7 = 1; 	// RX - En entr�e
	TRISBbits.TRISB6 = 1;	// TX - En entr�e bien que se soit une sortie
	ANSELCbits.ANSC7 = 0;	// Patte non annalogique
	
	//// Baudrate g�n�rator ////
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
	TXSTA2 = 0b10000000;			// Module emission n�2 d�sactiv�
   	
    //// Interruptions ////
	// Emission
    PIR1bits.TX1IF = 0;				// Mise � z�ro des drapeaux
	PIR3bits.TX2IF = 0;
	IPR1bits.TX1IP = 0;				// Selectionne basse priorit� pour TX (vecteur en 0x18), inutile
	IPR3bits.TX2IP = 0;
	PIE1bits.TX1IE = 0;				// IT en emission d�sactiv�e, sera activ� par la fonction "usart_envoyer_car()"
	PIE3bits.TX2IE = 0;				// IT toujours d�sactiv�e
	// R�ception
	PIR1bits.RC1IF = 0;				// Drapeaux
	PIR3bits.RC2IF = 0;
	IPR1bits.RC1IP = 0;				// S�lectionne basse priorit� pour RX (vecteur en 0x18)
	IPR3bits.RC2IP = 0;
	PIE1bits.RC1IE = 1;				// IT en r�ception activ�es
	PIE3bits.RC2IE = 1;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_etat_buffer_recep(unsigned char usart_portserie)			////
////	Description : Indique si un carract�re est disponible dans un buffer	////
////	Param�tres : usart_portserie : Buffer � v�rifier (1 ou 2)				////
////	Retour : 0, aucun carract�re disponible									////
////			 1ou+, un ou plusieurs carract�re sont disponibles				////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
unsigned char usart_etat_buffer_recep(unsigned char usart_portserie)
{
	if(usart_portserie == 1) // Port s�rie 1
	{
		return (usart_ptr_buffer_recep_1_e - usart_ptr_buffer_recep_1_l); // Pointeur d'ecriture moins pointeur lecture
	}
	else	// Port s�rie 2
	{
		return (usart_ptr_buffer_recep_2_e - usart_ptr_buffer_recep_2_l); // Pointeur d'ecriture moins pointeur lecture
	}
}

////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_car(unsigned char usart_portserie)		////
////	Description : Renvoi le carract�re le plus ancien disponible	////
////				  dans un buffer									////
////	Param�tres : usart_portserie, buffer � utiliser (1 ou 2)		///////////////////////////////////////////////////// Voir cas du z�ro !!!!!!!!!!!!!!!!
////	Retour : le carract�re le plus ancien du buffer selectionn�		////
////			 ou 0 si aucun caract�re n'est disponible				////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
unsigned char usart_recevoir_car(unsigned char usart_portserie)
{
	unsigned char usart_car_retour;	// Carract�re � renvoyer
	if(usart_portserie == 1) // Port s�rie 1
	{
		usart_car_retour = *usart_ptr_buffer_recep_1_l++;	// On lit le caract�re puis on incr�mente		
		usart_gestion_debordements_1();						// On corrige les �ventuels d�bordements
	}
	else // Port s�rie 2
	{
		usart_car_retour = *usart_ptr_buffer_recep_2_l++;			
		usart_gestion_debordements_2_recep();
	}
	return (usart_car_retour);								// On retourne le caract�re lu.
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin, unsigned char usart_portserie)	////
////	Description : R�ceptionne une chaine dans le buffer s�lectionn� et la termine par NULL (0)								////
////	Param�tres : *usart_cible, pointeur sur l'emplacement m�moire de r�ception												////
////				 usart_car_fin, carract�re de fin de chaine																	////
////				 usart_portserie, choix du buffer (1 ou 2)																	////
////	Retour : un pointeur vers la chaine																						////
////	Auteur : TL																												////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char *usart_recevoir_chaine(unsigned char *usart_cible, unsigned char usart_car_fin, unsigned char usart_portserie)
{
	unsigned char *usart_ptr_retour = usart_cible;	// D�finition du pointeur de retour
	unsigned char usart_caractere_temporaire;		// Octet dans lequel on stocke le caract�re temporaire

	while ((usart_caractere_temporaire = usart_recevoir_car(usart_portserie)) != usart_car_fin)	// Tant qu'on n'a pas obtenu le caract�re de fin
		*usart_cible++ = usart_caractere_temporaire; // On le place � l'emplacement cible puis on incr�mente
	*usart_cible = 0;			// Enfin, on termine par le caract�re de fin
	return (usart_ptr_retour);	// Et on retourne le pointeur de d�but de chaine
}

////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_car(unsigned char usart_car)	////
////	Description : Envoi un caract�re sur le port s�rie	////
////				  num�ro 2								////
////	Param�tres : usart_caractere, caract�re � envoyer	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_envoyer_car(unsigned char usart_caractere)
{
	*usart_ptr_buffer_emiss_2_e++ = usart_caractere; // On place le caract�re � envoyer dans le buffer de sortie, qu'on incr�mente
	usart_gestion_debordements_2_emiss();	// Puis on corrige les �ventuels d�bordements
	PIE1bits.TX1IE = 1;		// Activation des interruptions, si le module est libre, cela aura pour effet de g�n�rer une interruption
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////	Fonction usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)			////
////	Description : Envoie une chaine de caract�res ( !!!  PRESENTE EN RAM  !!! )						////
////	Param�tres : *usart_chaine, poiteur vers le premier caract�re									////
////				 usart_car_fin, caract�re de fin de chaine, non �mit sur le port					////
////	Retour : Rien																					////
////	Auteur : TL																						////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void usart_envoyer_chaine(unsigned char *usart_chaine, unsigned char usart_car_fin)
{
	while (*usart_chaine != usart_car_fin) // Tant qu'on n'a pas le caract�re de fin de chaine
	{
		usart_envoyer_car(*usart_chaine);	// On �met le caract�re
		usart_chaine++;	// Et on incr�mente le pointeur
	}
}

////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_1(void)			////
////	Description : D�tecte et corrige les d�bordements 	////
////				  du buffer de r�ception du port 1		////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_gestion_debordements_1(void)
{
	if (usart_ptr_buffer_recep_1_l >= &usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1]) 	// Si le buffer d�borde
		usart_ptr_buffer_recep_1_l = usart_buffer_recep_1;									// On remet � z�ro
	if (usart_ptr_buffer_recep_1_e >= &usart_buffer_recep_1[USART_TAILLE_BUFFER_RECEP_1])	
		usart_ptr_buffer_recep_1_e = usart_buffer_recep_1;
}

////////////////////////////////////////////////////////////////////////
////	Fonction usart_gestion_debordements_2_recep(void)			////
////	Description : D�tecte et corrige les d�bordements du buffer ////
////				  de r�ception du port s�rie 2					////
////	Param�tres : Aucun											////
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
////	Description : D�tecte et corrige les d�bordements	////
////				  du buffer d'�mission du port 2		////
////	Param�tres : Aucun									////
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
////	Description : G�re les interruption dues aux ports	////
////				  s�ries								////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void usart_interrupt(void)
{
	if(PIR1bits.TX1IF && PIE1bits.TX1IE)
	{
		if(usart_ptr_buffer_emiss_2_l != usart_ptr_buffer_emiss_2_e)		// Si un carract�re est pr�sent dans le buffer
		{	TXREG1 = *usart_ptr_buffer_emiss_2_l++;			// On envoi le carract�re le plus ancien du buffer
			usart_gestion_debordements_2_emiss();			// Gestion des d�bordements
		}
		else 
			PIE1bits.TX1IE = 0;		// On d�sactive cette source d'interruption
	}
	if(PIR3bits.RC2IF)		// Interruption de reception du port s�rie 1 (Module 2 cf. REM)
	{
		*usart_ptr_buffer_recep_1_e = RCREG2;		// Copie du registre de r�ception
		usart_etat_bits.nbr_octets_recus_1++;		// On incr�mente le nombre d'octet re�us
		usart_ptr_buffer_recep_1_e++;				// On incr�mente le pointeur
		usart_gestion_debordements_1();				// On v�rifie les d�bordements
	}	
	if(PIR1bits.RC1IF)		// Interruption de r�ception du port s�rie 2 (Module 1 cf. REM)
	{
		*usart_ptr_buffer_recep_2_e = RCREG1;		// Copie du registre de r�ception
		if (*usart_ptr_buffer_recep_2_e == 0) 		// Si il s'agit du carract�re de fin de trame (0)
			usart_etat_bits.is_car_buffer_2 = 1;	// On met le bits de fin de trame � 1
		usart_ptr_buffer_recep_2_e++;				// On incr�mente le pointeur
		usart_gestion_debordements_2_recep();		// On v�rifie les d�bordements
	}	
}