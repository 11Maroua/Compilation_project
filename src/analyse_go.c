#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "scan_go.h"
#include "analyse_go.h"

// Résultat de l'analyse courante
int Analyse;

// Table des symboles : nom + index dans A[]
static char table_noms[MAX_RULES][256];
static int  table_idx[MAX_RULES];
static int  nb_symboles = 0;

// Pile de construction des noeuds
static PTR pile[MAX_RULES];
static int sommet = 0;

// Prochain index libre dans A[]
static int prochain_idx = 6;

// Nom de la regle courante (gauche de la fleche)
static char regle_courante[256];

// Cherche un nom, retourne son index dans A[] ou -1
static int chercher_symbole(const char *nom) {
    for (int i = 0; i < nb_symboles; i++)
        if (strcmp(table_noms[i], nom) == 0)
            return table_idx[i];
    return -1;
}

// Réserve un index pour un symbole, retourne l'index
static int reserver_symbole(const char *nom) {
    int idx = chercher_symbole(nom);
    if (idx != -1) return idx;
    strcpy(table_noms[nb_symboles], nom);
    table_idx[nb_symboles] = prochain_idx;
    idx = prochain_idx;
    nb_symboles++;
    prochain_idx++;
    return idx;
}

// Empile un noeud
static void empiler(PTR p) {
    pile[sommet++] = p;
}

// Depile un noeud
static PTR depiler(void) {
    if (sommet <= 0) {
        fprintf(stderr, "Erreur : pile vide\n");
        exit(1);
    }
    return pile[--sommet];
}

// Construit le sous-arbre depuis la pile jusqu'a un marqueur NULL ou la fin
static PTR construire_depuis_pile(void) {
    // construire le dernier terme (serie de concats)
    PTR terme = depiler();
    while (sommet > 0 && pile[sommet-1] == (PTR)2) {
        depiler();
        PTR gauche = depiler();
        terme = GenConc(gauche, terme);
    }
    PTR resultat = terme;
    
    // combiner les termes par union
    while (sommet > 0 && pile[sommet-1] == (PTR)1) {
        depiler(); // retirer marqueur union
        // construire le terme gauche
        PTR terme_gauche = depiler();
        while (sommet > 0 && pile[sommet-1] == (PTR)2) {
            depiler();
            PTR g = depiler();
            terme_gauche = GenConc(g, terme_gauche);
        }
        resultat = GenUnion(terme_gauche, resultat);
    }
    return resultat;
}

// AnalyseGO
int AnalyseGO(PTR pt) {
    switch (pt->classe) {

        case Conc:
            Analyse = AnalyseGO(pt->conc.left);
            if (Analyse)
                Analyse = AnalyseGO(pt->conc.right);
            return Analyse;

        case Union: {
                int token_save = token_courant;
                char nom_save[256];
                char val_save[256];
                strcpy(nom_save, token_nom);
                strcpy(val_save, token_val);
                int sommet_save = sommet;
                
                Analyse = AnalyseGO(pt->uni.left);
                if (!Analyse) {
                    token_courant = token_save;
                    strcpy(token_nom, nom_save);
                    strcpy(token_val, val_save);
                    sommet = sommet_save;
                    Analyse = AnalyseGO(pt->uni.right);
                }
                return Analyse;
            }

        case Star:
            while (AnalyseGO(pt->star.store))
                ;
            return 1;

        case Un:
            AnalyseGO(pt->un.un);
            return 1;

        case Atom:
            if (pt->atom.atype == Terminal) {
                if (token_courant == pt->atom.cod) {
                    Analyse = 1;
                    if (pt->atom.act != 0)
                        ActionGO(pt->atom.act);
                    ScanGO();
                } else {
                    Analyse = 0;
                }
                return Analyse;
            } else {
                Analyse = AnalyseGO(A[pt->atom.ind]);
                if (Analyse && pt->atom.act != 0)
                    ActionGO(pt->atom.act);
                return Analyse;
            }
    }
    return 0;
}

// ActionGO
void ActionGO(int act) {
    switch (act) {

        // Fin d'une regle — construire l'arbre et stocker dans A[]
        case 1: {
            PTR resultat = construire_depuis_pile();
            int idx = chercher_symbole(regle_courante);
            if (idx == -1) idx = reserver_symbole(regle_courante);
            //printf("DEBUG action1: regle=%s idx=%d\n", regle_courante, idx);
            A[idx] = resultat;
            sommet = 0;
            break;
        }

        // IDNTER reconnu — c'est le nom de la regle courante
        case 2:
            strcpy(regle_courante, token_nom);
            reserver_symbole(token_nom);
            break;

        // '+' vu — empiler marqueur union
        case 3:
            empiler((PTR)1);
            break;

        // '.' vu — empiler marqueur concat
        case 4:
            empiler((PTR)2);
            break;

        // IDNTER ou ELTER dans un facteur — creer un Atom
        case 5: {
            PTR p;
            if (token_courant == TOK_IDNTER) {
                int idx = reserver_symbole(token_nom);
                p = GenAtom(TOK_IDNTER, 0, idx, NonTerminal);
            } else {
                // stocker la chaîne complète au lieu du seul premier caractère
                if (strlen(token_val) == 1) {
                    // un seul caractère — garder le code ASCII
                    p = GenAtom((unsigned char)token_val[0], 0, 0, Terminal);
                    p->atom.val[0] = '\0';
                } else {
                    // plusieurs caractères — stocker la chaîne
                    p = GenAtom(0, 0, 0, Terminal);
                    strcpy(p->atom.val, token_val);
                }
            }
            empiler(p);
            break;
        }

        // '[' — empiler marqueur debut Star
        case 6:
            empiler(NULL);
            break;

        // ']' — construire sous-arbre et creer Star
        case 8: {
            PTR contenu = construire_depuis_pile();
            depiler();
            empiler(GenStar(contenu));
            break;
        }

        // '(' — empiler marqueur debut Un
        case 7:
            empiler(NULL);
            break;

        // ')' — construire sous-arbre et creer Un
        case 9: {
            PTR contenu = construire_depuis_pile();
            depiler();
            empiler(GenUn(contenu));
            break;
        }

        default:
            fprintf(stderr, "ActionGO : action %d inconnue\n", act);
            exit(1);
    }
}