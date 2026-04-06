#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scan_go.h"

// Variables globales
int  token_courant;
char token_nom[256];
char token_val[256];

static FILE *fichier;     // fichier GPL en cours de lecture
static int   car_courant; // caractère courant lu

// Lecture d'un caractère
static void lire_car(void) {
    car_courant = fgetc(fichier);
}

// Ignorer espaces et tabulations
static void ignorer_blancs(void) {
    while (car_courant != EOF && isspace(car_courant))
        lire_car();
}

void ScanGO_init(const char *filename) {
    fichier = fopen(filename, "r");
    if (!fichier) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        exit(1);
    }

    lire_car(); // lis start symbol

}



// ScanGO — lit le prochain token depuis le fichier GPL
void ScanGO(void) {

    // Ignorer les espaces
    ignorer_blancs();

    // Fin de fichier
    if (car_courant == EOF) {
        token_courant = TOK_EOF;
        return;
    }

    // Identifiant non-terminal (IDNTER) — commence par une majuscule
    if (isupper(car_courant)) {
        int i = 0;
        while (car_courant != EOF &&
               (isalnum(car_courant) || car_courant == '_')) {
            token_nom[i++] = car_courant;
            lire_car();
        }
        token_nom[i] = '\0';
        token_courant = TOK_IDNTER;
        return;
    }

    // Terminal littéral (ELTER) — entre apostrophes : 'xxx'
    if (car_courant == '\'') {
        lire_car(); // sauter l'apostrophe ouvrante
        int i = 0;
        while (car_courant != EOF && car_courant != '\'') {
            token_val[i++] = car_courant;
            lire_car();
        }
        token_val[i] = '\0';
        if (car_courant == '\'')
            lire_car(); // sauter l'apostrophe fermante
        token_courant = TOK_ELTER;
        return;
    }

    // Flèche -> (représente → dans le fichier texte)
    if (car_courant == '-') {
        lire_car();
        if (car_courant == '>') {
            lire_car();
            token_courant = TOK_ARROW;
            return;
        }
        fprintf(stderr, "Erreur lexicale : '-' inattendu\n");
        exit(1);
    }

    // Symboles simples : . + ; , [ ] ( ) /
    token_courant = car_courant;
    lire_car();
}

// ScanGO_close — ferme le fichier
void ScanGO_close(void) {
    if (fichier) {
        fclose(fichier);
        fichier = NULL;
    }
}