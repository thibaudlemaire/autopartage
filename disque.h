////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						disque.h						////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de disque.c	////
////														////
////	Créé le : 08/01/2011								////
////	Modifié le : 13/02/2011								////
////	Support : PIC 18F24K22  -  16 MHz					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef disque_h
#define disque_h

//// Defines ////	
#define DISQUE_MOT_SENS1 PORTAbits.RA6	// Patte de sens moteur
#define DISQUE_MOT_SENS2 PORTAbits.RA7	// Patte de sens moteur
#define DISQUE_MOT_EN PORTBbits.RB5		// Patte d'activation moteur
#define DISQUE_ENC PORTBbits.RB1		// Patte de phase A de l'encodeur rotatif
#define DISQUE_SW PORTBbits.RB2			// Patte de mise à zéro de l'angle (microswitch)

//// Prototypes ////
void disque_init(void);
void disque_tourner(unsigned char disque_position_voulue);
void disque_moteur_start(unsigned char disque_moteur_sens);
void disque_moteur_stop(void);
void disque_taches(void);
void disque_convert_consigne(void);
void disque_gestion_debordements(void);
void disque_interrupt(void);

//// Définitions ////
extern char disque_encodeur_increment;							// Valeur entre 0 et 35 qui matérialise le nombre d'impulsions de l'encodeur rotatif pour un tour direct
extern unsigned char disque_increment_consigne;					// Valeur entre 0 et 35 de consigne de l'encodeur
extern struct
{
	unsigned consigne 		:3;					// Emplacement du disque (entre 0 et 5) voulu
	unsigned position		:3;					// Emplacement réel du disque (entre 0 et 5)
	unsigned valeur_enc		:1;					// Valeur stable de l'encodeur
	unsigned consigne_ok	:1;					// Flag de position conforme a la consigne
} disque_emplacement_bits;
extern struct
{
	unsigned dernier_niveau 		:1;			// Dernier niveau lu sur l'encodeur ( 0 : bas ou 1 : haut)
	unsigned statut_moteur			:2;			// 0 Rien, 1 Accélération, 2 Décélération
	unsigned marche_moteur			:1;			// Indique si le moteur est en rotation
	unsigned init					:1; 		// Phase d'initialisation
	unsigned echantillon_enc		:3;			// Echantillons de 3 valeurs de l'encodeur
} disque_statut_bits;
extern struct
{
	unsigned t1						:4;
	unsigned t2						:4;
} disque_echantillons_touches;					// Echantillons de 4 valeurs des touches sensitives
extern struct
{
	unsigned t3						:4;
	unsigned t4						:4;
} disque_echantillons_touches1;					// Echantillons de 4 valeurs des touches sensitives
extern struct
{
	unsigned t1						:1;
	unsigned t2						:1;
	unsigned t3						:1;
	unsigned t4						:1;
} disque_valeurs_touches;						// Valeurs stables des touches sensitives
#endif