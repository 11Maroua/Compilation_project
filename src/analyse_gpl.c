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
            return i + 1; // adresses commencent à 1
    return -1;
}

// Ajoute une variable, retourne son adresse
static int ajouter_var(const char *nom) {
    strcpy(table_vars[nb_vars++], nom);
    return nb_vars; // adresse = position dans la table
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

        // Répéter tant que ça marche
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
                if (token_gpl_courant == pt->atom.cod) {
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

        // Déclaration d'une variable — ajouter à la table
        case 1: {
            ajouter_var(token_gpl_nom);
            break;
        }

        // Début d'une affectation — empiler l'adresse de la variable
        case 2: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n",
                        token_gpl_nom);
                exit(1);
            }
            pile[sommet++] = addr;
            emit(LDA, addr);
            break;
        }

        // Fin d'une affectation — générer AFF
        case 3:
            emit(AFF, 0);
            break;

        // Read — générer LDA + RD + AFF
        case 4: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n",
                        token_gpl_nom);
                exit(1);
            }
            emit(LDA, addr);
            emit(RD, 0);
            emit(AFF, 0);
            break;
        }

        // Write — générer LDV + WRTLN
        case 5: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n",
                        token_gpl_nom);
                exit(1);
            }
            emit(LDV, addr);
            emit(WRTLN, 0);
            break;
        }

        // Charger une variable — générer LDV
        case 6: {
            int addr = chercher_var(token_gpl_nom);
            if (addr == -1) {
                fprintf(stderr, "Erreur : variable '%s' non declaree\n",
                        token_gpl_nom);
                exit(1);
            }
            emit(LDV, addr);
            break;
        }

        // Charger une constante entière — générer LDC
        case 7:
            emit(LDC, token_gpl_val);
            break;

        // Opérateur + — générer ADD
        case 8:
            emit(ADD, 0);
            break;

        // Opérateur - — générer MIN
        case 9:
            emit(MIN, 0);
            break;

        // Opérateur * — générer MULT
        case 10:
            emit(MULT, 0);
            break;

        // Opérateur / — générer DIV
        case 11:
            emit(DIV, 0);
            break;

        // Opérateur < — générer INF
        case 12:
            emit(INF, 0);
            break;

        // Opérateur <= — générer INFE
        case 13:
            emit(INFE, 0);
            break;

        // Opérateur > — générer SUP
        case 14:
            emit(SUP, 0);
            break;

        // Opérateur >= — générer SUPE
        case 15:
            emit(SUPE, 0);
            break;

        // Opérateur = — générer EG
        case 16:
            emit(EG, 0);
            break;

        // Opérateur <> — générer DIFF
        case 17:
            emit(DIFF, 0);
            break;

        // Début du While — sauvegarder l'adresse de retour
        case 18:
            pile[sommet++] = get_CO(); // adresse courante dans Pcode
            break;

        // Condition du While — générer JIF (à patcher plus tard)
        case 19:
            emit(JIF, 0);            // 0 sera patché après
            pile[sommet++] = get_CO() - 1; // adresse du JIF à patcher
            break;

        // Fin du While — générer JMP retour + patcher le JIF
        case 20: {
            int addr_jif  = pile[--sommet]; // adresse du JIF
            int addr_debut = pile[--sommet]; // adresse de début du while
            emit(JMP, addr_debut);
            patch(addr_jif, get_CO()); // patcher le JIF avec l'adresse de fin
            break;
        }

        // Début du If — générer JIF (à patcher)
        case 21:
            emit(JIF, 0);
            pile[sommet++] = get_CO() - 1; // adresse du JIF à patcher
            break;

        // Fin du If sans Else — patcher le JIF
        case 22: {
            int addr_jif = pile[--sommet];
            patch(addr_jif, get_CO());
            break;
        }

        // Début du Else — générer JMP + patcher le JIF du If
        case 23: {
            int addr_jif = pile[--sommet];
            emit(JMP, 0);
            pile[sommet++] = get_CO() - 1; // adresse du JMP à patcher
            patch(addr_jif, get_CO());
            break;
        }

        // Fin du Else — patcher le JMP
        case 24: {
            int addr_jmp = pile[--sommet];
            patch(addr_jmp, get_CO());
            break;
        }

        // Fin du programme — générer STOP
        case 25:
            emit(STOP, 0);
            break;

        default:
            fprintf(stderr, "ActionGPL : action %d inconnue\n", act);
            exit(1);
    }
}