////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						lcd.h							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Fichier de déclaration de lcd.c		////
////														////
////	Créé le : 14/11/2010								////
////	Modifié le : 28/01/2010								////
////	Support : PIC 18F24K22  -  16 MHz  					////
////	Par : 	Jean CLAISSE								////
////			Thibaud LEMAIRE								////
////														////
////////////////////////////////////////////////////////////////

#ifndef lcd_h
#define lcd_h

//// Defines ////
// Ports :
#define LCD_D7 LATAbits.LATA0 									// 4 bits de données
#define LCD_D6 LATAbits.LATA1 									// ...
#define LCD_D5 LATAbits.LATA2 									// ...
#define LCD_D4 LATAbits.LATA3 									// ...
#define LCD_E  LATAbits.LATA4									// Validation
#define LCD_RS LATAbits.LATA5									// Bit de choix entre commande et affichage
#define LCD_TRIS_D7 TRISAbits.TRISA0							// TRIS du PORTB
#define LCD_TRIS_D6 TRISAbits.TRISA1							// TRIS du PORTB
#define LCD_TRIS_D5 TRISAbits.TRISA2							// TRIS du PORTB
#define LCD_TRIS_D4 TRISAbits.TRISA3							// TRIS du PORTB
#define LCD_TRIS_E TRISAbits.TRISA4								// TRIS du PORTB
#define LCD_TRIS_RS TRISAbits.TRISA5							// TRIS du PORTB
// Buffer
#define LCD_TAILLE_BUFFER 100									// Taille du buffer de commandes
// Paramètres
#define LCD_DELAY_600NS Nop();Nop();Nop();					 	// Attendre 600ns, 3 NOP à 20MHz
#define LCD_PARAMETRES_AFFICHAGE_MSB 0x80						// Affecte 1 à RS
#define LCD_PARAMETRES_AFFICHAGE_LSB 0xA0						// Correspond à une tempo de 60us et RS à 1
#define LCD_PARAMETRES_EFFACEMENT 0x40							// Correspond à une tempo de 2ms
#define LCD_PARAMETRES_FONCTIONS 0x10							// Correspond à une tempo de 40us

//// Prototypes ////
void lcd_init(void);
void lcd_taches(void);
void lcd_positionner(unsigned char lcd_colone, unsigned char lcd_ligne);
void lcd_afficher(unsigned char lcd_carractere);
void lcd_afficher_chaine_ram(char *lcd_chaine);
void lcd_afficher_chaine_rom(const rom char *lcd_chaine);
void lcd_effacer(void);
void lcd_verif_debordement_e(void);
void lcd_verif_debordement_l(void);
void lcd_interrupt(void);

//// Définitions ///
extern struct 	// Structure du buffer
{
	unsigned commande				: 4; 	// Les 4 bits de commande
	// Les types de tempos :
	unsigned tempo_40us				: 1;	// Utilisé pour les commandes
	unsigned tempo_60us				: 1;	// Utilisé pour l'affichage
	unsigned tempo_2ms				: 1;	// Utilisé pour l'effacement
	unsigned com_ou_car				: 1;	// Définit si la patte RS doit être à 1 ou 0
}lcd_buffer[LCD_TAILLE_BUFFER];				// Déclaration du buffer
extern unsigned char *lcd_buffer_ptr_l; 		// Pointeur du buffer en lecture
extern unsigned char *lcd_buffer_ptr_e;			// Pointeur du buffer en écriture
extern unsigned char lcd_flag_tempo;			// Drapeau qui indique si la temporisation entre deux commande est terminée			

#endif
