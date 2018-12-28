////////////////////////////////////////////////////////////////
////														////
////			T P E   S - S I    2020-2011				////
////				Borne d'autopartage						////
////		   Lycée Auguste LOUBATIERES Agde				////
////														////
////////////////////////////////////////////////////////////////
////						main.c							////
////////////////////////////////////////////////////////////////
////														////
////	Description : Déroulement principal					////
////														////
////	Créé le : 14/11/2010								////
////	Modifié le : 04/12/2010								////
////	Support : PIC 18F24K22	-  16 MHz					////
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

//// Définitions ////
rfid tag_vehicule_scanne;	// Tag du véhicule scanné lors d'une restituion
rfid tag_temporaire;		// Tag scanné, temporaire
rfid tags_emplacements[6];	// Tags RFID aux differents emplacements, de 0 à 5, le zéro n'étant pas utilisé

// A optimiser !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
unsigned char compteur_for;	// Compteur pour boucle for
unsigned char is_vehicules;	// Variable locale de nombre de vehicule


struct
{
	unsigned char nom[17];		// Le nom de l'utilisateur, maximum 16 carractère + 1 pour le carractère de fin
	rfid tag_rfid;				// Le tag RFID de l'utilisateur en cours
	unsigned statut:1;			// Statut de l'utilisateur, 0 : aucun, 1 : en location
} utilisateur; 			// Utilisateur en cours
struct
{
	unsigned char nom[17];				// Nom du véhicule, max 16 + 1 carr. de fin
	unsigned char prix[7]; 				// Prix horraire du véhicule, max 6 + 1 carr. de fin
	unsigned char immatriculation[9]; 	// Immatriculation du véhicule, max 8 + 1 carr. de fin
} vehicule; 			// Véhicule en cours
struct
{
	unsigned erreur					:1;		// Bit d'erreur
	unsigned erreur_delais			:1;		// Erreur due à un délais trop long
	unsigned erreur_tag				:1;		// Tag non reconnu
	unsigned erreur_zero_vehicule 	:1;		// Erreur : pas de vehicule disponible
	unsigned transaction			:1;		// Bit de transaction en cours
	unsigned vehicule				:3;		// Compteur de choix de véhicule (location) de 1 à 5
} cycle;				// Structure de cycle

				unsigned char toto[2];

////////////////////////////////////////////////////////////////
////	Fonction main(void)									////
////	Description : Fonction principale					////
////	Paramètres : Aucun									////
////	Retour : Rien										////
////	Auteur : TL											////
////////////////////////////////////////////////////////////////
void main(void)
{
	//// Initialisation du PIC ////
	fonc_init_pic();					// Initialise le PIC
	it_init();							// Initialise les interruptions
	
	//// Tempo de réveil ////
	fonc_set_tempo(TEMPO_INIT);			// 0,5 secondes
	while (!fonc_diver.tempo);			// On attend la fin de la tempo
	fonc_diver.tempo = 0;				// RAZ du drapeau
	
	//// Initialisation de la borne : ////
	fonc_init_routines();				// Initialise les routines
	
	tags_emplacements[4] [0] = 0x01;
	tags_emplacements[4] [1] = 0x08;
	tags_emplacements[4] [2] = 0xCA;
	tags_emplacements[4] [3] = 0x37;
	tags_emplacements[4] [4] = 0x76;
	
	tags_emplacements[3] [0] = 0x01;
	tags_emplacements[3] [1] = 0x08;
	tags_emplacements[3] [2] = 0xCA;
	tags_emplacements[3] [3] = 0x3B;
	tags_emplacements[3] [4] = 0xB6;
	
	tags_emplacements[5] [0] = 0x01;
	tags_emplacements[5] [1] = 0x08;
	tags_emplacements[5] [2] = 0xCA;
	tags_emplacements[5] [3] = 0x61;
	tags_emplacements[5] [4] = 0xEB;
	
	//// Boucle sans fin ////
	while(1)
	{
		fonc_gestion_erreurs();			// Gestion et information des eventuelles erreurs
		fonc_raz_flags_var();			// RAZ de tous les drapeaux et des variables globales d'utilisateur et de véhicule en cours
		fonc_unset_tempo();				// Désactivation des eventuelles tempos
		
		////////////////////////
		//// Début du cycle ////
		////////////////////////
		cycle.transaction = 1;										// Début de la transaction
		lcd_effacer();												// On efface l'écran
		lcd_positionner(4,0); 						
		lcd_afficher_chaine_rom("Bonjour");							// Message d'accueil
		lcd_positionner(0,1); 								
		lcd_afficher_chaine_rom("Identifiez-vous");					// Demande de scan du badge
		rfid_scanner(1);											// Demande scan d'un utilisateur
		while (cycle.transaction && !cycle.erreur)					// Tant que l'on est dans un cycle normal, sans erreurs
		{
			fonc_taches();											// On effectue les taches récurentes
			///////////////////////////////
			//// Utilisateur identifié ////
			///////////////////////////////
			if (rfid_statut.scan_ok)								// Si on a identifié un utilisateur
			{
				rfid_statut.scan_ok = 0;							// RAZ du flag
				serveur_utilisateur(utilisateur.tag_rfid); 			// Envoi du Tag RFID au serveur
				fonc_set_tempo(TEMPO_SERVEUR); 
				while (cycle.transaction && !cycle.erreur)			// Tant que l'on est dans le cycle normal, sans erreurs
				{
					fonc_taches();									// On effectue les taches
					///////////////////////////////////
					//// Infos utilisateurs reçues ////
					///////////////////////////////////
					if (serveur_statut.utilisateur)									// Si on a reçu les infos sur l'utilisateur
					{
						serveur_statut.utilisateur = 0;								// RAZ du flag
						lcd_effacer();												// On efface l'écran
						lcd_positionner(3,0); 										// Colone 4, ligne 1
						lcd_afficher_chaine_rom("Bienvenue");
						lcd_positionner(0,1); 										// Colone 0, ligne 2
						lcd_afficher_chaine_ram(utilisateur.nom);					// Affichage du nom de l'utilisateur
						fonc_set_tempo(TEMPO_MESSAGE);
						while (!fonc_diver.tempo)									// Tant que l'on est dans a tempo
							fonc_taches();											// On effectue les taches
						/////////////////////
						//// Restitution ////
						/////////////////////
						if (utilisateur.statut)														// Si l'utilisateur est déja en location
						{
							lcd_effacer();															// On efface l'écran
							lcd_positionner(3,0); 													// Colone 4, ligne 1
							lcd_afficher_chaine_rom("Restitution");
							lcd_positionner(1,1); 													// Colone 0, ligne 2
							lcd_afficher_chaine_rom("Scannez la cle");
							rfid_scanner(2);														// Demande de scan de la clé
							fonc_set_tempo(TEMPO_SCAN);														// 10 secondes pour scanner la clé
							while (cycle.transaction && !cycle.erreur)								// Tant que l'on est dans le cycle normal, sans erreurs
							{
								fonc_taches();														// On effectue les taches
								//// Clé scannée ////
								if (rfid_statut.scan_ok)											// Si on a identifié un badge
								{
									rfid_statut.scan_ok = 0;										// RAZ du flag
									serveur_restitution(tag_vehicule_scanne);						// Signalement de la restitution
									fonc_set_tempo(4);
									while (cycle.transaction && !cycle.erreur)						// Tant que l'on est dans le cycle normal, sans erreurs
									{
										fonc_taches();												// On effectue les taches
										if (serveur_statut.validation)								// Si le serveur autorise la restitution
										{
											serveur_statut.validation = 0;									// RAZ du drapeau
											for(compteur_for = 1; compteur_for < 6; compteur_for++)			// Pour chaque emplacement, recherche de l'emplacement vide
											{
												if (fonc_is_tag_empty(tags_emplacements[compteur_for]))					// Si l'emplacement est vide
												{
													disque_tourner(compteur_for);							// On place le disque en position voulue
													lcd_effacer();											// On efface l'écran
													lcd_positionner(1,0); 									// Colone 1, ligne 1
													lcd_afficher_chaine_rom("Positionnement");
													lcd_positionner(1,1); 									// Colone 1, ligne 2
													lcd_afficher_chaine_rom("Patientez SVP");
													while (cycle.transaction && !cycle.erreur)				// Tant que l'on est dans le cycle normal, sans erreurs
													{
														fonc_taches();										// Taches
														if (disque_emplacement_bits.consigne_ok)			// Si on est à l'emplacement prévu
														{
															lcd_effacer();									// On efface l'écran
															lcd_positionner(0,0); 							// Colone 0, ligne 1
															lcd_afficher_chaine_rom("Deposez cle puis");
															lcd_positionner(4,1); 							// Colone 4, ligne 2
															lcd_afficher_chaine_rom("validez");	
															while (cycle.transaction && !cycle.erreur)		// Tant que l'on est dans le cycle normal, sans erreurs
															{
																fonc_taches();								// Taches
																if (disque_valeurs_touches.t2)				// Bouton OK
																{
																	disque_tourner(0);											// Retour à l'emplacement 0
																	fonc_copie_tag(tag_vehicule_scanne, tags_emplacements[compteur_for]);		// Enregistrement du tag
																	lcd_effacer();												// On efface l'écran
																	lcd_positionner(5,0); 										// Colone 6, ligne 1
																	lcd_afficher_chaine_rom("Merci");
																	lcd_positionner(3,1); 										// Colone 4, ligne 2
																	lcd_afficher_chaine_rom("A bientot");	
																	fonc_set_tempo(TEMPO_MESSAGE);											// Definition de la tempo de 3 secondes
																	while (!fonc_diver.tempo)									// Tant que l'on est dans la tempo
																	{
																		fonc_taches();											// Taches récurentes
																	}
																	fonc_diver.tempo = 0;
																	cycle.transaction = 0;										// Fin du cycle
																	compteur_for = 7;
																}
															}
														}
													}
												}
											}
/// !!!!!!!!!!!!!!!!!!! Si tous les emplacements sont occupés... 
										}
										else if (serveur_statut.erreur)		// Si le tag n'est pas reconnu
										{
											serveur_statut.erreur = 0;		// RAZ du drapeau
											cycle.erreur = 1;				// Erreur
											cycle.erreur_tag = 1;
										}
										else if (fonc_diver.tempo)			// Temps dépassé
										{
											cycle.erreur = 1;				// Erreur
											cycle.erreur_delais = 1;		// Erreur de délais
										}
									}
								}
								else if (disque_valeurs_touches.t1)
{
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x01;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x08;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0xCA;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x3B;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0xB6;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'c';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'c';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e = 'c';
	usart_etat_bits.nbr_octets_recus_1 = 11;	
}
								//// Erreur de temps dépassé ////
								else if (fonc_diver.tempo)
								{
									fonc_diver.tempo = 0;
									cycle.erreur = 1;				// Erreur
									cycle.erreur_delais = 1;		// Erreur de délais
								}
							}
						}
						//////////////////
						//// Location ////
						//////////////////
						else
						{
							is_vehicules = 0;													// Mise à 0 du compteur
							for (compteur_for = 1 ; compteur_for < 6 ; compteur_for++)			// Boucle de 1 à 5
							{
								if (!fonc_is_tag_empty(tags_emplacements[compteur_for]))		// Si il y a une clé dans l'emplacement
									is_vehicules++;												// On incremente ce compteur
							}	
							if (is_vehicules)																// Si il y a au moins un vehicule dans la borne
							{
								lcd_effacer();																// On efface l'écran
								lcd_positionner(1,0); 													
								lcd_afficher_chaine_rom("Faites defiler");
								lcd_positionner(1,1); 														
								lcd_afficher_chaine_rom("les vehicules");					
								fonc_set_tempo(TEMPO_MESSAGE);								// Definition de la tempo de 3 secondes
								while (!fonc_diver.tempo)									// Tant que l'on est dans la tempo
									fonc_taches();											// Taches récurentes
								fonc_diver.tempo = 0;
								cycle.vehicule = 0;											// Mise à zéro du compteur de vehicule
								do
								{
									cycle.vehicule++;		
								}
								while (fonc_is_tag_empty(tags_emplacements[cycle.vehicule]));				// On incrémente jusqu'à trouver un emplacement occupé
								fonc_loc_info_vehicule();													// On demande les infos de ce véhicule et on les affiches
								fonc_set_tempo(TEMPO_TOUCHE);															// Mise en place de la tempo de 10 secondes
								while (cycle.transaction && !cycle.erreur)									// Tant que l'on est dans le cycle normal, sans erreurs
								{
									fonc_taches();															// Taches
									if (disque_valeurs_touches.t1)											// Défilement gauche
									{
										do
										{
											cycle.vehicule++;														// On incremente le compteur
											fonc_loc_vehicule_debordements();										// Gestion des débordements du compteur
										} while (fonc_is_tag_empty(tags_emplacements[cycle.vehicule]));				// On incrémente jusqu'à trouver un emplacement occupé
										fonc_loc_info_vehicule();													// On demande les infos de ce véhicule et on les affiches
										while(disque_valeurs_touches.t1)
											fonc_taches();
										fonc_set_tempo(TEMPO_TOUCHE);															// Mise en place de la tempo de 10 secondes
									}
									else if (disque_valeurs_touches.t3)												// Défilement droit
									{
										do
										{
											cycle.vehicule--;														// On incremente le compteur
											fonc_loc_vehicule_debordements();										// Gestion des débordements du compteur
										} while (fonc_is_tag_empty(tags_emplacements[cycle.vehicule]));				// On incrémente jusqu'à trouver un emplacement occupé
										fonc_loc_info_vehicule();													// On demande les infos de ce véhicule et on les affiches
										while(disque_valeurs_touches.t3)
											fonc_taches();
										fonc_set_tempo(TEMPO_TOUCHE);															// Mise en place de la tempo de 10 secondes
									}
									else if (fonc_diver.tempo)														// Erreur de temps dépassé
									{
										fonc_diver.tempo = 0;
										cycle.erreur = 1;
										cycle.erreur_delais = 1;
									}
									else if (disque_valeurs_touches.t2)												// Touche OK	
									{
										serveur_statut.validation = 0;												// RAZ du drapeau
										serveur_location(tags_emplacements[cycle.vehicule]);						// Location du vehicule
										fonc_set_tempo(TEMPO_SERVEUR);															// Tempo de reponse serveur de 1 seconde
										while (cycle.transaction && !cycle.erreur)									// Tant que l'on est dans le cycle normal, sans erreurs
										{
											fonc_taches();															// Taches
											if (serveur_statut.validation)											// Si le serveur valide la location
											{
												serveur_statut.validation = 0;							// RAZ du flag
												fonc_effacer_tag(tags_emplacements[cycle.vehicule]);	// Remise à zéro de l'emplacement (vide)
												disque_tourner(cycle.vehicule);							// Rotation du disque
												lcd_effacer();											// On efface l'écran
												lcd_positionner(1,0); 									
												lcd_afficher_chaine_rom("Positionnement");
												lcd_positionner(1,1); 									
												lcd_afficher_chaine_rom("Patientez SVP");
												while (cycle.transaction && !cycle.erreur)				// Tant que l'on est dans le cycle normal, sans erreurs
												{
													fonc_taches();										// Taches
													if (disque_emplacement_bits.consigne_ok)			// Si on est à l'emplacement prévu
													{
														lcd_effacer();									// On efface l'écran
														lcd_positionner(1,0); 							
														lcd_afficher_chaine_rom("Recuperez cle");
														lcd_positionner(2,1); 							
														lcd_afficher_chaine_rom("puis validez");	
														fonc_set_tempo(TEMPO_CLE);								// Tempo de 10 secondes
														while (cycle.transaction && !cycle.erreur)		// Tant que l'on est dans le cycle normal, sans erreurs
														{
															fonc_taches();								// Taches
															if (disque_valeurs_touches.t2 || fonc_diver.tempo)				// Bouton OK ou fin de tempo
															{
																disque_tourner(0);											// Retour à l'emplacement 0
																lcd_effacer();												// On efface l'écran
																lcd_positionner(5,0); 										// Colone 6, ligne 1
																lcd_afficher_chaine_rom("Merci");
																lcd_positionner(3,1); 										// Colone 4, ligne 2
																lcd_afficher_chaine_rom("Bon voyage");	
																fonc_set_tempo(TEMPO_MESSAGE);											// Definition de la tempo de 3 secondes
																while (!fonc_diver.tempo)									// Tant que l'on est dans la tempo
																{
																	fonc_taches();											// Taches récurentes
																}
																fonc_diver.tempo = 0;
																cycle.transaction = 0;										// Fin du cycle
															}
														}
													}
												}
											}
											else if (fonc_diver.tempo)
											{
												fonc_diver.tempo = 0;
												cycle.erreur = 1;
												cycle.erreur_delais = 1;
											}
										}
									}
								}	
							}
							else	// Pas de véhicle disponible
							{
								cycle.erreur = 1;
								cycle.erreur_zero_vehicule = 1;
							}
						}
					}
					//// Erreur d'identification ////
					else if (serveur_statut.erreur)
					{
						serveur_statut.erreur = 0;				// RAZ du flag
						cycle.erreur = 1;
						cycle.erreur_tag = 1;
					}
					//// Erreur de temps dépassé ////
					else if (fonc_diver.tempo)
					{
						fonc_diver.tempo = 0;
						cycle.erreur = 1;
						cycle.erreur_delais = 1;
					}
				}
			}
			//////////////////////////////////
			//// Demande d'identification ////
			//////////////////////////////////
			else if (serveur_statut.identification)									// Si on a une demande d'identification
			{
				serveur_statut.identification = 0;									// RAZ du drapeau
				lcd_effacer();														// On efface l'écran
				lcd_positionner(0,0); 												// Colone 0, ligne 1
				lcd_afficher_chaine_rom("Demande d'ident.");
				lcd_positionner(0,1); 												// Colone 0, ligne 2
				lcd_afficher_chaine_rom("Passez un badge");							// Demande de scan du badge
				fonc_set_tempo(TEMPO_SCAN);													// Mise en place de la tempo de 10 secondes
				rfid_scanner(0);													// Demande de scann
				while (cycle.transaction && !cycle.erreur)							// Tant que l'on est dans le cycle normal, sans erreurs
				{
					fonc_taches();													// On effectue les taches
					if (rfid_statut.scan_ok)										// Si on a identifié un badge
					{
						rfid_statut.scan_ok = 0;
						serveur_tag(tag_temporaire);								// On envoi le tag
						cycle.transaction = 0;										// Puis on termine le cycle
					}
					else if (fonc_diver.tempo)										// Délais expiré
					{
						cycle.erreur = 1;											// Erreur
						cycle.erreur_delais = 1;
					}
				}
			}
else if (disque_valeurs_touches.t1)
{
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'a';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x01;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x08;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x4D;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0xD0;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 0x04;
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'c';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e++ = 'c';
	usart_gestion_debordements_1();
	*usart_ptr_buffer_recep_1_e = 'c';
	usart_etat_bits.nbr_octets_recus_1 = 11;	
}
			//////////////////////
			//// Touche appel ////
			//////////////////////
			else if (disque_valeurs_touches.t4)
			{
				serveur_appel();													// Appel du serveur
				lcd_effacer();														// On efface l'écran
				lcd_positionner(0,0); 												// Colone 0, ligne 1
				lcd_afficher_chaine_rom("Serveur alerte");
				lcd_positionner(0,1); 												// Colone 0, ligne 2
				lcd_afficher_chaine_rom("0800 77 24 24");
				fonc_set_tempo(TEMPO_MESSAGE);													// Mise en place de la tempo de 10 secondes
				while (!fonc_diver.tempo)											// Tant que la tempo n'est pas finie
					fonc_taches();													// On effectue les taches
				cycle.transaction = 0;												// Fin du cycle
			}
		}
	}
}
