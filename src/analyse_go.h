#ifndef ANALYSE_GO_H
#define ANALYSE_GO_H

#include "tree.h"

// Résultat de l'analyse (1 = analyse juste, 0 = erreur matche pas grammaire Go)
extern int Analyse;

// AnalyseGO — parcourt l'arbre A et vérifie la GPL
int AnalyseGO(PTR pt);

// ActionGO — déclenche une action sémantique
void ActionGO(int act);

#endif