////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						disque.c						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de gestion du disque � cl�s	////
////														////
////	Cr�� le : 08/01/2011								////
////	Modifi� le : 13/02/2011								////
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
char disque_encodeur_increment;		// Valeur entre 0 et 47 qui mat�rialise le nombre d'impulsions de l'encodeur rotatif pour un tour direct
unsigned char disque_increment_consigne;		// Valeur entre 0 et 47 de consigne de l'encodeur
struct
{
	unsigned consigne 		:3;					// Emplacement du disque (entre 0 et 5) voulu
	unsigned position		:3;					// Emplacement r�el du disque (entre 0 et 5)
	unsigned valeur_enc		:1;					// Valeur stable de l'encodeur
	unsigned consigne_ok	:1;					// Flag de position conforme a la consigne
} disque_emplacement_bits;
struct
{
	unsigned dernier_niveau 		:1;			// Dernier niveau lu sur l'encodeur ( 0 : bas ou 1 : haut)
	unsigned statut_moteur			:2;			// 0 Rien, 1 Acc�l�ration, 2 D�c�l�ration
	unsigned marche_moteur			:1;			// Indique si le moteur est en rotation
	unsigned init					:1; 		// Phase d'initialisation
	unsigned echantillon_enc		:3;			// Echantillons de 3 valeurs de l'encodeur
} disque_statut_bits;
struct
{
	unsigned t1						:4;
	unsigned t2						:4;
} disque_echantillons_touches;					// Echantillons de 4 valeurs des touches sensitives
struct
{
	unsigned t3						:4;
	unsigned t4						:4;
} disque_echantillons_touches1;					// Echantillons de 4 valeurs des touches sensitives
struct
{
	unsigned t1						:1;
	unsigned t2						:1;
	unsigned t3						:1;
	unsigned t4						:1;
} disque_valeurs_touches;						// Valeurs stables des touches sensitives

////////////////////////////////////////////////////////////////
////	Fonction disque_init(void)							////
////	Description : Initialise le disque					////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_init(void)
{
	// Sens des pattes :
	TRISBbits.TRISB5 = 0;		// Patte d'enable moteur : sortie
	TRISAbits.TRISA6 = 0;		// Patte de sens moteur : sortie
	TRISAbits.TRISA7 = 0;		// Patte de sens moteur : sortie
	TRISBbits.TRISB1 = 1;		// Patte de phase A de l'encodeur : entr�e
	TRISBbits.TRISB2 = 1;		// Patte de rep�rage du z�ro (microswitch) : entr�e
	TRISCbits.TRISC0 = 1;
	TRISCbits.TRISC1 = 1;
	TRISCbits.TRISC2 = 1;
	TRISCbits.TRISC3 = 1;
	ANSELCbits.ANSC2 = 0;
	ANSELCbits.ANSC3 = 0;
	// Configuration PWM du moteur :
	CCPR3L = 0b00000000;		// MSB du rapport cyclique
	CCPTMRS0 = 0b10000000;		// Selection du Timer6 pour le module CCP3
	CCP3CON = 0b00001100;		// 00 unused, 00 LSB du rapport cyclique, 11 mode PWM, 00 unused
	// Configuration du timer 6 pour le PWM et les interruptions(acc�l�ration et ralentissement)
	PR6 = 0b11111111;			// D�compte du timer, ne sera jamais modifi�, d�fini la p�riode du signal PWM
	T6CON = 0b01111010; 		// Post-diviseur 16, pr�-diviseur 16, Module activ�
	IPR5bits.TMR6IP = 1;		// Haute priorit� pour les interruptions caus�es par le Timer 6
	PIR5bits.TMR6IF = 0; 		// Flag baiss�
	PIE5bits.TMR6IE = 1;		// Interruption activ�e
	// Initialisation du disque :	
	disque_increment_consigne			= 0;				// La consigne d'incr�ment est 0
	disque_emplacement_bits.consigne 	= 0;				// Par cons�quent la position de consigne est 0
	if (DISQUE_SW == 1)										// Si on est en position z�ro, microswitch actif
	{
		disque_encodeur_increment 			= 0;			// On met � z�ro toutes les variables de positionnement
		disque_emplacement_bits.position	= 0;			
		disque_statut_bits.init = 0;						// Pas de phase d'initialisation
	}
	else													// Sinon
	{
		disque_statut_bits.init = 1;						// On passe en mode initialisation
		DISQUE_MOT_SENS1 = 1;		// Sens	direct
		DISQUE_MOT_SENS2 = 0;
		disque_statut_bits.marche_moteur = 1;	// Flag de marche moteur
		// Allumage moteur :
		CCPR3L = 100;		// Rapport cyclique : tres basse vitesse
	}
	// Initialisation de l'encodeur et de l'�chantillonage :
	if (DISQUE_ENC)											// Si l'encodeur est au niveau haut
	{
		disque_statut_bits.echantillon_enc = 3;				// On impose un niveau haut stable
		disque_emplacement_bits.valeur_enc = 1;				// Sa valeur est donc 1
	}
	else													// Encodeur niveau bas
	{
		disque_statut_bits.echantillon_enc = 0;				// On impose un niveau bas
		disque_emplacement_bits.valeur_enc = 0;					// Valeur 0
	}
	disque_statut_bits.dernier_niveau	= disque_emplacement_bits.valeur_enc;	// Le dernier niveau de l'encodeur est celui en cours
	T6CONbits.TMR6ON = 1;	// Activation du timer
}

////////////////////////////////////////////////////////////////
////	Fonction disque_tourner(unsigned char)				////
////	Description : Met le moteur en rotation	et applique	////
//// 				  applique une consigne � celui-ci		////
////	Param�tres : disque_position_voulue, consigne � 	////
////				 appliquer								////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_tourner(unsigned char disque_position_voulue)
{
	char disque_rotation; 	// Variable sign�e de rep�rage sur le disque
	
	disque_rotation = disque_position_voulue - disque_emplacement_bits.position; 	// Calcul du sens et du nombre de pas � effectuer
	disque_emplacement_bits.consigne = disque_position_voulue;						// On met en place la consigne
	disque_convert_consigne();														// On convertie la consigne en incr�ment
	disque_emplacement_bits.consigne_ok = 0;										// La consigne n'est plus respect�e
	
	if (disque_rotation > 3)			// Il se peut que ce ne soit pas le plus court chemin...
			disque_rotation -= 6;		// ...Auquel cas, on enl�ve un tour
	else if (disque_rotation < -3)
		disque_rotation += 6;			// On prend le plus court chemin
		
	if (disque_rotation > 0)			// Si la variable est positive, il faut tourner dans le sens direct
		disque_moteur_start(1);			// On lance la rotation dans le sens direct
	else if (disque_rotation < 0)		// Si la variable est n�gative, il faut tourner dans le sens indirect
		disque_moteur_start(0);			// On lance la rotation dans le sens indirect
	if (disque_rotation == 1 || disque_rotation == -1)
		disque_statut_bits.statut_moteur = 0;		// Pas d'acceleration
}

////////////////////////////////////////////////////////////////
////	Fonction disque_moteur(unsigned char)				////
////	Description : Fait tourner le moteur				////
////	Param�tres : disque_moteur_sens, sens de rotation	////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_moteur_start(unsigned char disque_moteur_sens)
{
	if (disque_moteur_sens == 0)	// Si sens indirect
	{
		DISQUE_MOT_SENS1 = 0;		// Sens	indirect
		DISQUE_MOT_SENS2 = 1;
	}
	else							// Si sens direct
	{
		DISQUE_MOT_SENS1 = 1;		// Sens	direct
		DISQUE_MOT_SENS2 = 0;
	}
	disque_statut_bits.marche_moteur = 1;	// Flag de marche moteur
	// Allumage moteur :
	CCPR3L = 135;		// Rapport cyclique environ 50% : basse vitesse
	disque_statut_bits.statut_moteur = 1;		// Moteur mode acc�l�ration
}

////////////////////////////////////////////////////////////////
////	Fonction disque_moteur_stop(void)					////
////	Description : Stoppe le moteur						////
////	Param�tres : rien									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_moteur_stop(void)
{
	disque_statut_bits.marche_moteur = 0;	// Flag de marche moteur � z�ro
	disque_statut_bits.statut_moteur = 0; 	// Aucun changement de vitesse
	CCPR3L = 0b00000000;					// Rapport cyclique 0% : Arret du moteur
	CCP3CON = 0b00001100;					// 00 unused, 00 LSB du rapport cyclique, 11 mode PWM, 00 unused
	DISQUE_MOT_SENS1 = DISQUE_MOT_SENS2;	// Mise en place du frein moteur
}

////////////////////////////////////////////////////////////////
////	Fonction disque_taches(void)						////
////	Description : S'occupe de g�rer l'encodeur rotatif	////
////				  et d'arreter la rotation du moteur	//// 
////				  quand la consigne est atteinte		////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_taches(void)
{
	if (disque_statut_bits.marche_moteur)		// Si le moteur est en marche
	{
		if (DISQUE_SW)			// Si on est en position 0, microswitch ferm�
		{
			disque_encodeur_increment 	= 0;			// On met � z�ro toutes les variables de positionnement
			disque_emplacement_bits.position	= 0;
			disque_statut_bits.dernier_niveau	= disque_emplacement_bits.valeur_enc;	// Le dernier niveau de l'encodeur est celui en cours
			if (disque_emplacement_bits.consigne == 0)	// Si la consigne est 0
			{
				disque_moteur_stop();				// On arrete le moteur
				disque_statut_bits.init = 0;		// On repasse en mode stadart
			}
		}
		
		if (disque_emplacement_bits.valeur_enc != disque_statut_bits.dernier_niveau && disque_statut_bits.init != 1)	// Si la valeur de l'encodeur � chang�e, et que l'on n'est pas en initialisation
		{
			if (DISQUE_MOT_SENS1)					// Et si on est dans le sent direct
			{
				disque_encodeur_increment += 1;			// On incr�mente de 1
				disque_gestion_debordements();			// Puis on corrige les �vntuels d�bordements
			}
			else										// Si on est dans le sens indirect
			{
				disque_encodeur_increment -= 1;			// On d�cr�mente
				disque_gestion_debordements();			// On corrige les �ventuels d�bordements
			}
			
			//// Ralentissement ou arret ////
			if (disque_encodeur_increment == disque_increment_consigne && disque_increment_consigne != 0)		// Et enfin, on v�rifie que l'on est pas � la position voulue
			{
				disque_moteur_stop();									// Si c'est le cas, on stope le moteur
				disque_emplacement_bits.consigne_ok = 1;				// La consigne est respect�e
				disque_emplacement_bits.position = disque_emplacement_bits.consigne;
			}
			else if (disque_increment_consigne != 0)						// Si la consigne n'est pas z�ro 
			{
				if (disque_encodeur_increment == (disque_increment_consigne - 3 ) ||		// Limite de ralentissement : 30� <=> 3 impulsions
					(disque_encodeur_increment == (disque_increment_consigne + 3 )))		// On v�rifie si on est pas au limites de ralentissement (sens direct)
					disque_statut_bits.statut_moteur = 2;									// Moteur mode d�c�l�ration
			}	
			else																			// Si la consigne est l'incr�met z�ro
			{
				if (disque_encodeur_increment == 30 || disque_encodeur_increment == 3)		// On v�rifie les limites de ralentissement
					disque_statut_bits.statut_moteur = 2;									// Moteur mode d�c�l�ration
			}	
			disque_statut_bits.dernier_niveau = disque_emplacement_bits.valeur_enc; 		// On affecte la valeur de l'encodeur
		}
	}
}

////////////////////////////////////////////////////////////////
////	Fonction disque_convert_consigne(void)				////
////	Description : Convertie la position de consigne en 	////
////				  incr�ment de consigne					////
////	Param�tres : Aucun									////
////	Retour : rien 										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_convert_consigne(void)
{
	switch(disque_emplacement_bits.consigne)						// Simple conversion
	{
		case 0:	
			disque_increment_consigne = 0;		// On aurait put faire disque_consigne * 6
		break;
		case 1:
			disque_increment_consigne = 4;		// mais ce proc�d� est plus gourmand en ressource
		break;
		case 2:
			disque_increment_consigne = 11;		// et un tableau de valeur r�pond parfaitement au cahier des charges...
		break;
		case 3:
			disque_increment_consigne = 17;		// REM : Voir peut �tre un d�calage de bit moins gourmand
		break;
		case 4:
			disque_increment_consigne = 24;
		break;
		case 5:
			disque_increment_consigne = 29;
		break;
	}
}

////////////////////////////////////////////////////////////////
////	Fonction disque_gestion_debordements(void)			////
////	Description : V�rifie est corrige les d�passements	////
////				  de valeur de l'incr�ment				////
////	Param�tres : Aucun									////
////	Retour : rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_gestion_debordements(void)
{
	if (disque_encodeur_increment > 33)		// Si on a fait un tour
		disque_encodeur_increment = 0;		// On est donc � z�ro
	else if (disque_encodeur_increment < 0)	// Si on part dans le sens indirect
		disque_encodeur_increment = 33;		// On est donc en position 33
}

////////////////////////////////////////////////////////////////
////	Fonction disque_interrupt(void)						////
////	Description : G�re les interrutpions				////
////	Param�tres : Aucun									////
////	Retour : rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void disque_interrupt(void)
{
	if (disque_statut_bits.statut_moteur == 1)			// Si on est dans la phase d'acc�l�ration
	{
		CCPR3L += 4; 						// On augmente la vitesse du moteur
		if (CCPR3L == 255)					// On v�rifie que l'on est pas a la vitesse maxi
			disque_statut_bits.statut_moteur = 0;		// Moteur mode normal
	}
	else if (disque_statut_bits.statut_moteur == 2)		// Si on est dans la phase de d�c�l�ration
	{
		CCPR3L -= 8; 						// On diminue la vitesse du moteur
		if (CCPR3L <= 103)					// On v�rifie que l'on est pas a la vitesse mini
			disque_statut_bits.statut_moteur = 0;			// Moteur mode normal
	}
	//// Echantillonage de l'encodeur ////
	disque_statut_bits.echantillon_enc <<= 1; // On d�cale d'un cran vers la gauche
	disque_statut_bits.echantillon_enc |= DISQUE_ENC;	// Puis on affecte la valeur de l'encodeur au bit de poids faible
	if (disque_statut_bits.echantillon_enc == 7)	// Si les trois derniers �chantillons sont "1"
		disque_emplacement_bits.valeur_enc = 1; 		// L'encodeur est stable, sa valeur est "1"
	else if (disque_statut_bits.echantillon_enc == 0)	
		disque_emplacement_bits.valeur_enc = 0;		// La valeur de l'encodeur est "0"
	//// Echantillonage des touches ////
	disque_echantillons_touches.t1 <<= 1; // On d�cale d'un cran vers la gauche
	disque_echantillons_touches.t1 |= PORTCbits.RC0;	// Puis on affecte la valeur de l'encodeur au bit de poids faible
	if (disque_echantillons_touches.t1 == 15)		// Si les trois derniers �chantillons sont "1"
		disque_valeurs_touches.t1 = 0; 	// L'encodeur est stable, sa valeur est "0"
	else if (disque_echantillons_touches.t1 == 0)	
		disque_valeurs_touches.t1 = 1;		// La valeur de la touches est "1"
		
	disque_echantillons_touches.t2 <<= 1; // On d�cale d'un cran vers la gauche
	disque_echantillons_touches.t2 |= PORTCbits.RC1;	// Puis on affecte la valeur de l'encodeur au bit de poids faible
	if (disque_echantillons_touches.t2 == 15)		// Si les trois derniers �chantillons sont "1"
		disque_valeurs_touches.t2 = 0; 	// L'encodeur est stable, sa valeur est "0"
	else if (disque_echantillons_touches.t2 == 0)	
		disque_valeurs_touches.t2 = 1;		// La valeur de la touches est "1"
		
	disque_echantillons_touches1.t3 <<= 1; // On d�cale d'un cran vers la gauche
	disque_echantillons_touches1.t3 |= PORTCbits.RC2;	// Puis on affecte la valeur de l'encodeur au bit de poids faible
	if (disque_echantillons_touches1.t3 == 15)		// Si les 8 derniers �chantillons sont "1"
		disque_valeurs_touches.t3 = 0; 	// L'encodeur est stable, sa valeur est "0"
	else if (disque_echantillons_touches1.t3 == 0)	
		disque_valeurs_touches.t3 = 1;		// La valeur de la touches est "1"
		
	disque_echantillons_touches1.t4 <<= 1; // On d�cale d'un cran vers la gauche
	disque_echantillons_touches1.t4 |= PORTCbits.RC3;	// Puis on affecte la valeur de l'encodeur au bit de poids faible
	if (disque_echantillons_touches1.t4 == 15)		// Si les trois derniers �chantillons sont "1"
		disque_valeurs_touches.t4 = 0; 	// L'encodeur est stable, sa valeur est "0"
	else if (disque_echantillons_touches1.t4 == 0)	
		disque_valeurs_touches.t4 = 1;		// La valeur de la touches est "1"
}