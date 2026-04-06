#ifndef TREE_H
#define TREE_H

/* Types de base */

typedef enum {
    Terminal,
    NonTerminal
} AtomType;

typedef enum {
    Conc,    /* concaténation  '.'  */
    Union,   /* union          '+'  */
    Star,    /* étoile         '[X]'  zéro ou plusieurs fois */
    Un,      /* optionnel      '(|X|)' zéro ou une fois     */
    Atom     /* feuille : terminal ou non-terminal           */
} Operation;

/* Nœud de l'arbre */

typedef struct Node* PTR;

struct Node {
    Operation classe;
    union {
        struct { PTR left;  PTR right; } conc;
        struct { PTR left;  PTR right; } uni;
        struct { PTR store;            } star;
        struct { PTR un;               } un;
        struct {
            int      cod;    //code ASCII si Terminal, index dans A si NonTerminal 
            int      act;    //numéro d'action sémantique à décelncher (0 = aucune)
            int      ind;    //index dans A[] si NonTerminal   
            char     val[256]; //valeur de l'atom si c'est un terminal (ex: "IDNTER", "ELTER", ou le caractère lui même)                    
            AtomType atype; //terminal ou non-terminal
        } atom;
    };
};

/*  Tableau principal */

#define MAX_RULES 200

extern PTR A[MAX_RULES];  /* A[1..5] = régles de grammaires G0,  A[6..n] = GPL */

/* Indices des 5 règles de G0 */
#define IDX_S  1
#define IDX_N  2
#define IDX_E  3
#define IDX_T  4
#define IDX_F  5

/* Fonctions de construction */

PTR GenConc (PTR p1, PTR p2);
PTR GenUnion(PTR p1, PTR p2);
PTR GenStar (PTR p);
PTR GenUn   (PTR p);
PTR GenAtom (int cod, int act, int ind, AtomType atype);

/* Génération de la forêt G0 */

void GenForetGO(void);

/* Impression de l'arbre*/

void ImprimArbre(PTR p, int profondeur);

#endif /* TREE_H */
