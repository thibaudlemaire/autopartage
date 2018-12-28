////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					interrupts.c						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de gestion des interruptions	////
////														////
////	Cr�� le : 04/12/2010								////
////	Modifi� le : 10/01/2011								////
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

////////////////////////////////////////////////////////////////
////	Fonction it_init(void)								////
////	Description : Initialise les interrutpions			////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void it_init(void)
{
	//// Initialisations des interruptions ////
	RCONbits.IPEN = 1;			// Activaion des priorit�es d'interuption
	INTCONbits.GIEL = 1;		// Activation des interruptions de basse priorit�
	INTCONbits.GIEH = 1;		// Activation des interruptions de haute priorit�
}

////////////////////////////////////////////////
////		Gestion des interruptions 		////
////////////////////////////////////////////////

//Sous programme de traitement de l�interruption de basse priorit�
#pragma code low_vector=0x18
void interrupt_at_low_vector(void)
{
_asm GOTO it_b_prio _endasm
}
#pragma code 
#pragma interruptlow it_b_prio
void it_b_prio(void)
{
	usart_interrupt();
	if(PIR1bits.TMR2IF)		// Si l'interruption provient du timer 2 --> LCD
	{
		lcd_interrupt();		// On g�re l'interruption
		PIR1bits.TMR2IF = 0;	// On remet le drapeau � z�ro
	}
	if(PIR1bits.TMR1IF)		// Si l'interruption provient du timer 1 --> TEMPO
	{
		fonc_interrupt_tempo();	// Appel de la fonction d'interruption
		PIR1bits.TMR1IF = 0;		// RAZ du drapeau
	}
}

//Sous programme de traitement de l�interruption de haute priorit�
#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
_asm GOTO it_h_prio _endasm
}
#pragma code 
#pragma interrupt it_h_prio
void it_h_prio(void)
{
	if(PIR5bits.TMR6IF)		// Si l'interruption provient du timer 6 --> DISQUE
	{
		disque_interrupt();		// On g�re l'interruption
		PIR5bits.TMR6IF = 0;	// On remet le drapeau � z�ro
	}
}