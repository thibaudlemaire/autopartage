////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lyc�e Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////					fonctions.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de fonctions diverses			////
////														////
////	Cr�� le : 04/12/2010								////
////	Modifi� le : 31/01/2010								////
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

//// Definitons ////
struct
{
	unsigned tempo			:1;				// Flag de la tempo
	unsigned boucle_info_v	:1;				// Boucle info v�hicule, bit de sortie
} fonc_diver;								// Structure foure tout
unsigned char compteur_tempo;				// Compteur pour la tempo

////////////////////////////////////////////////////////////////
////	Fonction fonc_init_pic(void)						////
////	Description : Initialise le microcontroleur			////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_init_pic(void)
{
	//// Initialisation de l'oscillateur ////
	OSCTUNE = 0;
	OSCCON = 0b01110010;
	OSCCON2bits.MFIOSEL = 1;
	// Initialisation du timer 1
	T1CON = 0b00110101;					// Timer activ�, FOSC/4, Prescaler 8
	IPR1bits.TMR1IP = 0;				// Basse priorit�
	TMR1L = 0b10110000;					// G�n�ration d'une interruption toutes les 100ms
	TMR1H = 0b00111100;
	PIE1bits.TMR1IE = 0;				// On d�sactive les interruptions pour le moment
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_init_routines(void)					////
////	Description : Initialise les routines				////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_init_routines(void)
{
	it_init();				// Initialise les interruptions
	lcd_init();				// Initialise l'afficheur
	usart_init();			// Initialise les ports s�rie
	disque_init();			// Initialise la gestion du disque
	serveur_init();			// Initialise les communications avec le serveur
	rfid_init();			// Initialise les communications avec le module RFID
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_taches(void)							////
////	Description : Effectue les taches r�curentes		////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_taches(void)
{
	lcd_taches();		// Taches de l'ecran LCD
	disque_taches();	// Taches de la gestion du disque
	rfid_taches();		// Taches du module RFID
	serveur_taches(); 	// Taches de communication avec le serveur
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_set_tempo(unsigned char)				////
////	Description : Configure la temporisation			////
////	Param�tres : Dur�e de la tempo en 1/2 sec			////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_set_tempo(unsigned char fonc_tempo)
{
	fonc_diver.tempo = 0;							// RAZ du drapeau
	PIE1bits.TMR1IE = 1;							// Activation des interruptions
	compteur_tempo = fonc_tempo * 5;				// Nombre d'interuptions
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_unset_tempo()							////
////	Description : D�sactive la tempo					////
////	Param�tres : 										////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_unset_tempo()
{
	PIE1bits.TMR1IE = 0;							// D�sactivation des interruptions
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_interrupt_tempo(void)					////
////	Description : Gestion des interrutptions de tempo	////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_interrupt_tempo(void)
{
	compteur_tempo--;					// On d�cr�mente le compteur
	if (compteur_tempo == 0)			// Si la tempo est finie
	{
		fonc_diver.tempo = 1;			// On leve le drapeau
		PIE1bits.TMR1IE = 0;			// On d�sactive les interruptions
	}
}

////////////////////////////////////////////////////////////////
////	Fonction fonc_gestion_erreurs(void)					////
////	Description : G�re et informe des erreurs			////
////	Param�tres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void fonc_gestion_erreurs(void)
{
	if (cycle.erreur)	// Si une erreur est survenue
		{
			lcd_effacer();														// On efface l'�cran
			lcd_positionner(4,0); 												
			lcd_afficher_chaine_rom("Erreur !");
			lcd_positionner(0,1); 
			if (cycle.erreur_delais)
				lcd_afficher_chaine_rom(" Delais expire");
			else if (cycle.erreur_tag)
				lcd_afficher_chaine_rom("Tag non reconnu");
			else if (cycle.erreur_zero_vehicule)
				lcd_afficher_chaine_rom(" Aucun vehicule");
			else
				lcd_afficher_chaine_rom("Erreur inconnue");
			fonc_set_tempo(TEMPO_ERREUR);										// Mise en place de la tempo de 3 secondes
			while (!fonc_diver.tempo)											// Tant que la tempo n'est pas finie
				fonc_taches();													// On effectue les taches
			cycle.erreur = 0;													// RAZ des drapeaux
			cycle.erreur_tag = 0;
			cycle.erreur_delais = 0;
			cycle.erreur_zero_vehicule = 0;	
		}
}

////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_loc_info_vehicule(void)							////
////	Description : Demande et affiche les infos sur un v�hicule		////
////	Param�tres : Aucun												////
////	Retour : Rien													////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
void fonc_loc_info_vehicule(void)
{
	serveur_statut.erreur = 0;											// RAZ du flag d'erreur
	serveur_statut.vehicule = 0;										// RAZ du flag de reception des infos v�hicule
	serveur_vehicule(tags_emplacements[cycle.vehicule]);				// Demande des infos sur le vehicule
	fonc_set_tempo(20);													// Tempo d'une seconde pour la reponse du serveur
	fonc_diver.boucle_info_v = 1;										// Mise � 1 du bit de sortie
	while (fonc_diver.boucle_info_v)									
	{
		fonc_taches();													// Taches
		if (serveur_statut.vehicule)									// Si on a re�u les infos sur le vehicule
		{
			fonc_diver.boucle_info_v = 0;								// On sort de la boucle
			lcd_effacer();												// On efface l'�cran
			lcd_positionner(0,0); 												
			lcd_afficher_chaine_ram(vehicule.nom);						// Affichage du nom du vehicule
			lcd_positionner(0,1); 											
			lcd_afficher_chaine_ram(vehicule.immatriculation);			// Affichage le l'immatriculation
			lcd_positionner(12,1); 											
			lcd_afficher_chaine_ram(vehicule.prix);						// Affichage du prix
			lcd_afficher_chaine_rom("E/h");
		}
		else if (serveur_statut.erreur)									// Si le serveur signale une erreur
		{
			serveur_statut.erreur = 0;									// RAZ du drapeau
			lcd_effacer();												// On efface l'�cran
			lcd_positionner(0,0); 												
			lcd_afficher_chaine_rom("Vehicule inconnu");
			lcd_positionner(1,1); 											
			lcd_afficher_chaine_rom("Faites defiler");			
			fonc_diver.boucle_info_v = 0;								// On sort de la boucle
		}
		else if (fonc_diver.tempo)										// Si le serveur met trop de temps � r�pondre
		{
			fonc_diver.tempo = 0;										// RAZ du drapeau
			cycle.erreur = 1;
			cycle.erreur_delais = 1;
			fonc_diver.boucle_info_v = 0;								// On sort de la boucle
		} 
	}		
}

////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_loc_vehicule_debordements(void)					////
////	Description : G�re les d�bordements des v�hicules en location	////
////	Param�tres : Aucun												////
////	Retour : Rien													////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
void fonc_loc_vehicule_debordements(void)
{
	if (cycle.vehicule >= 6)			// Si on d�borde
		cycle.vehicule = 0;				// On remet � l'emplacement 1
	else if (cycle.vehicule <= 0)
		cycle.vehicule = 5;
}

////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_raz_flags_var(void)								////
////	Description : Remet � z�ro les drapeaux	et les variables		////
////	Param�tres : Aucun												////
////	Retour : Rien													////
////	Auteur : TL														////
////////////////////////////////////////////////////////////////////////////
void fonc_raz_flags_var(void)
{
	//// RAZ des flags ////
	rfid_statut.attente_vehicule = 0;							// RAZ des demandes de scan
	rfid_statut.attente_utilisateur = 0;	
	rfid_statut.scan_ok = 0;									// RAZ du drapeau de scan
	serveur_statut.validation = 0;								// RAZ des flags serveurs
	serveur_statut.identification = 0;
	serveur_statut.utilisateur = 0;
	serveur_statut.vehicule = 0;
	serveur_statut.erreur = 0;	
	disque_emplacement_bits.consigne_ok = 0;					// RAZ du drapeau de consigne du disque
	//// RAZ des variables ////
	
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! A faire !!!

}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_copie_tag(unsigned char *, unsigned char *)				////
////	Description : R�alise une transfert (copie) de tag rfid					////
////	Param�tres : Deux pointeurs, un vers la source, l'autre vers la cible	////
////	Retour : Rien															////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
void fonc_copie_tag(unsigned char *fonc_from, unsigned char *fonc_to)
{
	*fonc_to++ = *fonc_from++;			// Transfere de la source a la destination puis incremente les pointeurs
	*fonc_to++ = *fonc_from++;
	*fonc_to++ = *fonc_from++;
	*fonc_to++ = *fonc_from++;
	*fonc_to = *fonc_from;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_effacer_tag(unsigned char *)								////
////	Description : Efface un tag rfid										////
////	Param�tres : Un pointeur vers le tag a effacer							////
////	Retour : Rien															////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
void fonc_effacer_tag(unsigned char *fonc_tag)
{
	*fonc_tag++ = 0;					// Assigne 0 au point� puis incr�mente le pointeur
	*fonc_tag++ = 0;
	*fonc_tag++ = 0;
	*fonc_tag++ = 0;
	*fonc_tag = 0;
}

////////////////////////////////////////////////////////////////////////////////////
////	Fonction fonc_is_tag_empty(unsigned char *)								////
////	Description : Retourne 1 si le tag est vide ou 0 s'il ne l'est pas		////
////	Param�tres : Un pointeur vers le tag � tester							////
////	Retour : 0 ou 1															////
////	Auteur : TL																////
////////////////////////////////////////////////////////////////////////////////////
unsigned char fonc_is_tag_empty(unsigned char *fonc_tag)
{
	if (*fonc_tag++ != 0)
		return 0;
	else if (*fonc_tag++ != 0)
		return 0;
	else if (*fonc_tag++ != 0)
		return 0;
	else if (*fonc_tag++ != 0)
		return 0;
	else if (*fonc_tag != 0)
		return 0;
	else 
		return 1;
}