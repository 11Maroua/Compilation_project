#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "scan_gpl.h"
#include "pcode.h"
#include "analyse_gpl.h"

// Résultat de l'analyse courante
int AnalyseGPL_result;

// Table des variables du programme
static char table_vars[256][256];
static int  nb_vars = 0;

// Pile de travail pour les adresses et valeurs
static int pile[1000];
static int sommet = 0;

// Cherche une variable, retourne son adresse (index)
static int chercher_var(const char *nom) {
    for (int i = 0; i < nb_vars; i++)
        if (strcmp(table_vars[i], nom) == 0)
            return i + 1;
    return -1;
}

// Ajoute une variable, retourne son adresse
static int ajouter_var(const char *nom) {
    strcpy(table_vars[nb_vars++], nom);
    return nb_vars;
}

// AnalyseGPL
int AnalyseGPL(PTR pt) {
    switch (pt->classe) {

        // Les deux fils doivent matcher
        case Conc:
            AnalyseGPL_result = AnalyseGPL(pt->conc.left);
            if (AnalyseGPL_result)
                AnalyseGPL_result = AnalyseGPL(pt->conc.right);
            return AnalyseGPL_result;

        // Essayer gauche, sinon droite
        case Union:
            AnalyseGPL_result = AnalyseGPL(pt->uni.left);
            if (!AnalyseGPL_result)
                AnalyseGPL_result = AnalyseGPL(pt->uni.right);
            return AnalyseGPL_result;

        // Répéter tant que ca marche
        case Star:
            while (AnalyseGPL(pt->star.store))
                ;
            return 1;

        // Optionnel, toujours vrai
        case Un:
            AnalyseGPL(pt->un.un);
            return 1;

        // Feuille
        case Atom:
            if (pt->atom.atype == Terminal) {
                int match = 0;
                if (pt->atom.val[0] != '\0') {
                    // tokens speciaux
                    if (strcmp(pt->atom.val, "IDNTER") == 0) {
                        match = (token_gpl_courant == TOK_GPL_IDNTER);
                    } else if (strcmp(pt->atom.val, "ELTER") == 0) {
                        match = (token_gpl_courant == TOK_GPL_ENTIER);
                    } else {
                        match = (strcmp(pt->atom.val, token_gpl_nom) == 0);
                    }
                } else {
                    // Terminal simple — comparer par code ASCII
                    match = (token_gpl_courant == pt->atom.cod);
                }
                if (match) {
                    AnalyseGPL_result = 1;
                    if (pt->atom.act != 0)
                        ActionGPL(pt->atom.act);
                    ScanGPL();
                } else {
                    AnalyseGPL_result = 0;
                }
                return AnalyseGPL_result;
            } else {
                AnalyseGPL_result = AnalyseGPL(A[pt->atom.ind]);
                if (AnalyseGPL_result && pt->atom.act != 0)
                    ActionGPL(pt->atom.act);
                return AnalyseGPL_result;
            }
    }
    return 0;
}

// ActionGPL
void ActionGPL(int act) {
    switch (act) {

        // Déclaration d'une variable
        case 1:
            ajouter_var(token_gpl_nom);
            break;

        // Debut d'une affectation
        case 2: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n", token_gpl_nom);
                exit(1);
            }
            pile[sommet++] = addr;
            emit(LDA, addr);
            break;
        }

        // Fin d'une affectation
        case 3:
            emit(AFF, 0);
            break;

        // Read
        case 4: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n", token_gpl_nom);
                exit(1);
            }
            emit(LDA, addr);
            emit(RD, 0);
            emit(AFF, 0);
            break;
        }

        // Write
        case 5: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n", token_gpl_nom);
                exit(1);
            }
            emit(LDV, addr);
            emit(WRTLN, 0);
            break;
        }

        // Charger une variable
        case 6: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n", token_gpl_nom);
                exit(1);
            }
            emit(LDV, addr);
            break;
        }

        // Charger une constante entiere
        case 7:
            emit(LDC, token_gpl_val);
            break;

        // Operateur +
        case 8:
            emit(ADD, 0);
            break;

        // Operateur -
        case 9:
            emit(MIN, 0);
            break;

        // Operateur *
        case 10:
            emit(MULT, 0);
            break;

        // Operateur /
        case 11:
            emit(DIV, 0);
            break;

        // Operateur <
        case 12:
            emit(INF, 0);
            break;

        // Operateur <=
        case 13:
            emit(INFE, 0);
            break;

        // Operateur >
        case 14:
            emit(SUP, 0);
            break;

        // Operateur >=
        case 15:
            emit(SUPE, 0);
            break;

        // Operateur =
        case 16:
            emit(EG, 0);
            break;

        // Operateur <>
        case 17:
            emit(DIFF, 0);
            break;

        // Debut du While
        case 18:
            pile[sommet++] = get_CO();
            break;

        // Condition du While — JIF a patcher
        case 19:
            emit(JIF, 0);
            pile[sommet++] = get_CO() - 1;
            break;

        // Fin du While
        case 20: {
            int addr_jif   = pile[--sommet];
            int addr_debut = pile[--sommet];
            emit(JMP, addr_debut);
            patch(addr_jif, get_CO());
            break;
        }

        // Debut du If
        case 21:
            emit(JIF, 0);
            pile[sommet++] = get_CO() - 1;
            break;

        // Fin du If sans Else
        case 22: {
            int addr_jif = pile[--sommet];
            patch(addr_jif, get_CO());
            break;
        }

        // Debut du Else
        case 23: {
            int addr_jif = pile[--sommet];
            emit(JMP, 0);
            pile[sommet++] = get_CO() - 1;
            patch(addr_jif, get_CO());
            break;
        }

        // Fin du Else
        case 24: {
            int addr_jmp = pile[--sommet];
            patch(addr_jmp, get_CO());
            break;
        }

        // Fin du programme
        case 25:
            emit(STOP, 0);
            break;

        default:
            fprintf(stderr, "ActionGPL : action %d inconnue\n", act);
            exit(1);
    }
}