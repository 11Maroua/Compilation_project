#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "scan_go.h"
#include "analyse_go.h"

// Résultat de l'analyse courante
int Analyse;

// Table des symboles — stocke les noms des non-terminaux de la GPL
static char table_symboles[MAX_RULES][256];
static int  nb_symboles = 0;

// Pile de construction — utilisée par ActionGO pour construire l'arbre
static PTR pile[MAX_RULES];
static int sommet = 0;

// Compteur de règles GPL — A[6], A[7]... sont réservés pour les règles définies dans la GPL
static int nb_regles = 6;

// Cherche un nom dans la table des symboles, retourne son index dans A
static int chercher_symbole(const char *nom) {
    for (int i = 0; i < nb_symboles; i++)
        if (strcmp(table_symboles[i], nom) == 0)
            return i + 6; // les règles GPL commencent à A[6]
    return -1;
}

// ANALYSEGO — parcourt l'arbre A et vérifie la GPL 

int AnalyseGO(PTR pt) {

    switch (pt->classe) {

        // Concaténation : les analyses des deux fils doivent marcher
        case Conc:
            Analyse = AnalyseGO(pt->conc.left);
            if (Analyse)
                Analyse = AnalyseGO(pt->conc.right);
            return Analyse;

        // Union : l'analyse du fils gauche doit marcher, sinon celle du fils droit doit marcher
        case Union:
            Analyse = AnalyseGO(pt->uni.left);
            if (!Analyse)
                Analyse = AnalyseGO(pt->uni.right);
            return Analyse;

        // Star : répéter tant que ça matche (toujours vrai car si ça marche 0 fois c'est ok pour cet opérateur)
        case Star:
            Analyse = 1;
            while (AnalyseGO(pt->star.store))
                ;
            return 1;

        // Un : optionnel, toujours vrai car 0 fois marche aussi ici
        case Un:
            AnalyseGO(pt->un.un);
            return 1;

        // Atom : feuille — terminal ou non-terminal
        case Atom:

            if (pt->atom.atype == Terminal) {
                // Vérifier que le token courant correspond
                if (token_courant == pt->atom.cod) {
                    Analyse = 1;
                    // Déclencher l'action sémantique si elle existe
                    if (pt->atom.act != 0)
                        ActionGO(pt->atom.act);
                    // Avancer au token suivant
                    ScanGO();
                } else {
                    Analyse = 0;
                }
                return Analyse;

            } else {
                // NonTerminal : analyser récursivement la règle référencée
                Analyse = AnalyseGO(A[pt->atom.ind]);
                if (Analyse && pt->atom.act != 0)
                    ActionGO(pt->atom.act);
                return Analyse;
            }
    }
    return 0;
}

// ACTIONGO — actions sémantiques (construire l'arbre GPL)


void ActionGO(int act) {
    switch (act) {

        // Action 1 : fin d'une règle — raccorder gauche et droite
        // On dépile le sous-arbre construit et on le stocke dans A
        case 1: {
            PTR droite = pile[--sommet];
            int idx = chercher_symbole(table_symboles[nb_symboles - 1]);
            if (idx == -1) idx = nb_regles++;
            A[idx] = droite;
            break;
        }

        // Action 2 : IDNTER reconnu — ajouter à la table des symboles
        case 2:
            strcpy(table_symboles[nb_symboles++], token_nom);
            break;

        // Action 3 : '+' reconnu — créer un nœud Union
        case 3: {
            PTR droite = pile[--sommet];
            PTR gauche = pile[--sommet];
            pile[sommet++] = GenUnion(gauche, droite);
            break;
        }

        // Action 4 : '.' reconnu — créer un nœud Conc
        case 4: {
            PTR droite = pile[--sommet];
            PTR gauche = pile[--sommet];
            pile[sommet++] = GenConc(gauche, droite);
            break;
        }

        // Action 5 : IDNTER ou ELTER dans un facteur — créer un Atom
        case 5: {
            PTR p;
            if (token_courant == TOK_IDNTER) {
                int idx = chercher_symbole(token_nom);
                if (idx == -1) idx = nb_regles;
                p = GenAtom(TOK_IDNTER, 0, idx, NonTerminal);
            } else {
                // ELTER : terminal littéral
                p = GenAtom(token_val[0], 0, 0, Terminal);
            }
            pile[sommet++] = p;
            break;
        }

        // Action 6 : '[' ou ']' — créer un nœud Star
        case 6: {
            PTR p = pile[--sommet];
            pile[sommet++] = GenStar(p);
            break;
        }

        // Action 7 : '(' ou ')' — créer un nœud Un
        case 7: {
            PTR p = pile[--sommet];
            pile[sommet++] = GenUn(p);
            break;
        }

        default:
            fprintf(stderr, "ActionGO : action %d inconnue\n", act);
            exit(1);
    }
}