#ifndef PCODE_H
#define PCODE_H

// Instructions du P-code
typedef enum {
    LDA,    // charger une adresse
    LDV,    // charger une valeur
    LDC,    // charger une constante
    AFF,    // affectation
    RD,     // read
    WRTLN,  // writeln
    JMP,    // saut inconditionnel
    JIF,    // saut si faux
    ADD,    // addition
    MIN,    // soustraction
    MULT,   // multiplication
    DIV,    // division
    NEG,    // négation
    INC,    // incrémenter
    DEC,    // décrémenter
    AND,    // et logique
    OR,     // ou logique
    NOT,    // non logique
    SUP,    // supérieur >
    SUPE,   // supérieur ou égal >=
    INF,    // inférieur 
    INFE,   // inférieur ou égal <=
    EG,     // égal =
    DIFF,   // différent <>
    STOP    // arrêt
} Instruction;

// Une case du P-code  = une instruction + son argument
typedef struct {
    Instruction inst;
    int         arg;
} Case_Pcode;

// Taille maximale du P-code
#define MAX_PCODE 10000

// Le tableau P-code et le compteur ordinal
extern Case_Pcode Pcode[MAX_PCODE];
extern int        CO;

// Émet une instruction dans le P-code
void emit(Instruction inst, int arg);

// Retourne l'adresse courante dans le P-code
int get_CO(void);

// Patche une adresse dans le P-code
void patch(int addr, int val);

// Affiche le P-code généré 
void afficher_pcode(void);

#endif