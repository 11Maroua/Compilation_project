#ifndef ANALYSE_GPL_H
#define ANALYSE_GPL_H

#include "tree.h"

// Résultat de l'analyse (1 = analyse juste, 0 = erreur matche pas grammaire Go)
extern int AnalyseGPL_result;

// Analyse le programme en parcourant l'arbre GPL
int AnalyseGPL(PTR pt);

// Déclenche une action sémantique GPL
void ActionGPL(int act);

#endif