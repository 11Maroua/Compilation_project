#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "scan_gpl.h"

// Variables globales
int  token_gpl_courant;
char token_gpl_nom[256];
int  token_gpl_val;

static FILE *fichier_gpl;
static int   car_courant;

// Lire un caractère
static void lire_car(void) {
    car_courant = fgetc(fichier_gpl);
}

// Ignorer espaces et tabulations
static void ignorer_blancs(void) {
    while (car_courant != EOF && isspace(car_courant))
        lire_car();
}

// ScanGPL_init — ouvre le fichier programme
void ScanGPL_init(const char *filename) {
    fichier_gpl = fopen(filename, "r");
    if (!fichier_gpl) {
        fprintf(stderr, "Erreur : impossible d'ouvrir %s\n", filename);
        exit(1);
    }
    lire_car();
}

// ScanGPL — lit le prochain token du programme GPL
void ScanGPL(void) {

    // Ignorer les espaces
    ignorer_blancs();

    // Fin de fichier
    if (car_courant == EOF) {
        token_gpl_courant = TOK_GPL_EOF;
        return;
    }

    // Lettre minuscule — token simple, retourner son code ASCII
    if (islower(car_courant)) {
        token_gpl_nom[0] = car_courant;
        token_gpl_nom[1] = '\0';
        token_gpl_courant = (unsigned char)car_courant;
        lire_car();
        return;
    }

    // Lettre majuscule — identifiant long (Sp, IDNTER, While...)
    if (isupper(car_courant)) {
        int i = 0;
        while (car_courant != EOF &&
               (isalnum(car_courant) || car_courant == '_')) {
            token_gpl_nom[i++] = car_courant;
            lire_car();
        }
        token_gpl_nom[i] = '\0';
        token_gpl_courant = TOK_GPL_IDNTER;
        return;
    }

    // Constante entière — commence par un chiffre
    if (isdigit(car_courant)) {
        token_gpl_val = 0;
        while (car_courant != EOF && isdigit(car_courant)) {
            token_gpl_val = token_gpl_val * 10 + (car_courant - '0');
            lire_car();
        }
        token_gpl_courant = TOK_GPL_ENTIER;
        return;
    }

    // -> (fleche)
    if (car_courant == '-') {
        lire_car();
        if (car_courant == '>') {
            lire_car();
            strcpy(token_gpl_nom, "->");
            token_gpl_courant = TOK_GPL_IDNTER;
            return;
        }
        token_gpl_nom[0] = '-';
        token_gpl_nom[1] = '\0';
        token_gpl_courant = '-';
        return;
    }

    // := (affectation)
    if (car_courant == ':') {
        lire_car();
        if (car_courant == '=') {
            lire_car();
            token_gpl_courant = ':' * 256 + '=';
            return;
        }
        token_gpl_nom[0] = ':';
        token_gpl_nom[1] = '\0';
        token_gpl_courant = ':';
        return;
    }

    // <= ou <>
    if (car_courant == '<') {
        lire_car();
        if (car_courant == '=') {
            lire_car();
            token_gpl_courant = '<' * 256 + '=';
            return;
        }
        if (car_courant == '>') {
            lire_car();
            token_gpl_courant = '<' * 256 + '>';
            return;
        }
        token_gpl_nom[0] = '<';
        token_gpl_nom[1] = '\0';
        token_gpl_courant = '<';
        return;
    }

    // >=
    if (car_courant == '>') {
        lire_car();
        if (car_courant == '=') {
            lire_car();
            token_gpl_courant = '>' * 256 + '=';
            return;
        }
        token_gpl_nom[0] = '>';
        token_gpl_nom[1] = '\0';
        token_gpl_courant = '>';
        return;
    }

    // Apostrophe — terminal litteral entre apostrophes dans une GPL
    if (car_courant == '\'') {
        lire_car();
        int i = 0;
        while (car_courant != EOF && car_courant != '\'') {
            token_gpl_nom[i++] = car_courant;
            lire_car();
        }
        token_gpl_nom[i] = '\0';
        if (car_courant == '\'')
            lire_car();
        token_gpl_courant = TOK_GPL_IDNTER;
        return;
    }

    // Symboles simples : + - * / = ; , ( ) [ ] .
    token_gpl_nom[0] = car_courant;
    token_gpl_nom[1] = '\0';
    token_gpl_courant = car_courant;
    lire_car();
}

// ScanGPL_close — ferme le fichier
void ScanGPL_close(void) {
    if (fichier_gpl) {
        fclose(fichier_gpl);
        fichier_gpl = NULL;
    }
}