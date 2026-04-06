#ifndef SCAN_GO_H
#define SCAN_GO_H

// Codes des tokens de G0 

#define TOK_ARROW    256   // ->                              
#define TOK_IDNTER   257   // identifiant non-terminal       
#define TOK_ELTER    258   // terminal littéral 'xxx'        
#define TOK_EOF      259   // fin de fichier                 

//  Token courant 

extern int   token_courant;   // code du token lu            
extern char  token_nom[256];  // nom si IDNTER               
extern char  token_val[256];  // valeur si ELTER             

/*   Fonctions */
void ScanGO_init(const char *filename);  // ouvre le fichier et lis token initial
void ScanGO(void);                       // lit token suivant 
void ScanGO_close(void);                 // ferme le fichier  

#endif 