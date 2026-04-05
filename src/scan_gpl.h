#ifndef SCAN_GPL_H
#define SCAN_GPL_H

// Codes des tokens du programme GPL
#define TOK_GPL_IDNTER  300  // identifiant (variable, nom de programme)
#define TOK_GPL_ENTIER  301  // constante entière
#define TOK_GPL_EOF     302  // fin de fichier

// Token courant
extern int  token_gpl_courant;  // code du token lu
extern char token_gpl_nom[256]; // nom si IDNTER
extern int  token_gpl_val;      // valeur si ENTIER

// Fonctions
void ScanGPL_init(const char *filename);
void ScanGPL(void);
void ScanGPL_close(void);

#endif