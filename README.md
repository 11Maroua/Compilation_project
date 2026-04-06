# Projet Compilo

Implémentation en C d'un méta-compilateur et d'un compilateur basés sur la grammaire G0.

---

## Architecture globale

```
Grammaire G0 (écrite à la main)
        │
        ▼
┌─────────────────────┐        ┌─────────────────────┐
│   MÉTA-COMPILATEUR  │        │     COMPILATEUR      │
│                     │        │                      │
│  ScanGO             │        │  ScanGPL             │
│  AnalyseGO   ───────┼──────► │  AnalyseGPL          │
│  ActionGO           │        │  ActionGPL           │
│                     │        │        │             │
│  → construit        │        │        ▼             │
│    l'arbre A[6..n]  │        │   P-code généré      │
└─────────────────────┘        │        │             │
                               │        ▼             │
                               │   Exec + Interpret   │
                               └─────────────────────┘
```

**Le méta-compilateur** lit une grammaire GPL et vérifie qu'elle respecte G0.
S'il la valide, il construit sa représentation interne dans le tableau A[6..n].

**Le compilateur** lit un programme écrit en GPL, l'analyse grâce à l'arbre
construit par le méta-compilateur, génère du P-code et l'exécute.

---

## La Grammaire G0 — le cœur du projet

G0 est une **méta-grammaire** : c'est la grammaire qui décrit ce qu'est une
grammaire valide. Elle se compose de 5 règles fixes, écrites à la main dans
le code. Toute grammaire GPL doit respecter ces 5 règles pour être acceptée.

### Les 5 règles de G0

```
(1) S → [N. '->' .E. ';']. ','
(2) N → 'IDNTER'
(3) E → T. ['+'.T]
(4) T → F. ['.'.F]
(5) F → 'IDNTER' | 'ELTER' | '('.E.')' | '['.E.']' | '/'.E.'/'
```

### Notation utilisée dans G0

| Notation | Signification                              | Exemple                           |
|----------|--------------------------------------------|-----------------------------------|
| `[X]`    | X répété 0 ou plusieurs fois (étoile `*`)  | `['+'.T]` = zéro ou plusieurs +T  |
| `(X)`    | X optionnel (0 ou 1 fois)                  | `('Else'.Inst)` = Else optionnel  |
| `.`      | Concaténation — "suivi de"                 | `N.'->'` = N puis →               |
| `+`      | Union — "ou"                               | `'a'+'b'` = a ou b                |
| `'x'`    | Terminal littéral — le caractère x lui-même| `';'` = point-virgule             |
| `IDNTER` | Identifiant non-terminal (S, N, E, T, F…)  | N, E, Inst…                       |
| `ELTER`  | Terminal littéral entre apostrophes        | `'if'`, `'then'`                  |

### Ce que chaque règle signifie en français

**Règle 1 — S** : une grammaire valide est une liste de règles séparées par des
virgules. Chaque règle a la forme `NomNonTerminal -> Expression ;`

**Règle 2 — N** : un non-terminal est un identifiant (un nom comme S, Inst, Expr…)

**Règle 3 — E** : une expression est un ou plusieurs Termes reliés par `+` (union = "ou")

**Règle 4 — T** : un terme est un ou plusieurs Facteurs reliés par `.` (concaténation = "suivi de")

**Règle 5 — F** : un facteur est soit :
- un non-terminal (IDNTER)
- un terminal littéral (ELTER)
- une expression entre parenthèses `(E)` → optionnel
- une expression entre crochets `[E]` → répétition
- une expression entre slashs `/E/` → optionnel aussi

---

## Structure de données — l'Arbre A

### Le tableau A[]

```c
PTR A[MAX_RULES];
```

C'est le tableau central de tout le projet.

| Cases    | Contenu                      | Rempli par                   |
|----------|------------------------------|------------------------------|
| A[1]     | Arbre de la règle S de G0    | GenForetGO (à la main)       |
| A[2]     | Arbre de la règle N de G0    | GenForetGO (à la main)       |
| A[3]     | Arbre de la règle E de G0    | GenForetGO (à la main)       |
| A[4]     | Arbre de la règle T de G0    | GenForetGO (à la main)       |
| A[5]     | Arbre de la règle F de G0    | GenForetGO (à la main)       |
| A[6..n]  | Arbres des règles de la GPL  | ActionGO automatiquement     |

### Les types de nœuds

Chaque nœud de l'arbre a un type parmi 5 :

| Type    | Symbole G0 | Fils           | Rôle                     |
|---------|-----------|----------------|--------------------------|
| `Conc`  | `.`       | gauche + droit | "A suivi de B"           |
| `Union` | `+`       | gauche + droit | "A ou B"                 |
| `Star`  | `[X]`     | store          | "répéter 0 ou n fois"    |
| `Un`    | `(X)`     | un             | "optionnel, 0 ou 1 fois" |
| `Atom`  | feuille   | aucun          | terminal ou non-terminal |

### Le nœud Atom en détail

Un nœud `Atom` est une feuille de l'arbre. Il peut être de deux sortes :

**Terminal** : représente un symbole littéral à lire dans l'entrée.
```
cod   = code ASCII du caractère (ex: ';' = 59)
        ou code spécial (TOK_ARROW=256, TOK_IDNTER=257, TOK_ELTER=258)
val   = chaîne complète si ELTER multi-caractères (ex: "->", "IDNTER")
act   = numéro d'action sémantique à déclencher (0 = aucune)
atype = Terminal
```

**NonTerminal** : représente une référence vers une autre règle.
```
ind   = index dans A[] vers la règle référencée
        (ex: ind=3 → A[IDX_E] → règle E)
act   = numéro d'action sémantique à déclencher (0 = aucune)
atype = NonTerminal
```

---

## Les Actions Sémantiques

Quand `AnalyseGO` reconnaît un token qui a un numéro d'action non nul
(`act != 0`), il appelle `ActionGO(numéro)`. Ces actions construisent
l'arbre de la GPL dans A[6..n].

Il y a 9 actions au total :

| #  | Déclenché quand                  | Ce que ça fait                                           |
|----|----------------------------------|----------------------------------------------------------|
| 1  | Fin d'une règle (token `;`)      | Construit l'arbre complet et le stocke dans A[index]     |
| 2  | IDNTER (nom de règle)            | Enregistre le nom dans la table des symboles             |
| 3  | `+` (union)                      | Empile un marqueur d'union                               |
| 4  | `.` (concat)                     | Empile un marqueur de concaténation                      |
| 5  | IDNTER ou ELTER dans un facteur  | Crée une feuille Atom et l'empile                        |
| 6  | `[` (ouverture Star)             | Empile un marqueur NULL de début Star                    |
| 7  | `(` ou `/` (ouverture Un)        | Empile un marqueur NULL de début Un                      |
| 8  | `]` (fermeture Star)             | Dépile le contenu, crée GenStar et l'empile              |
| 9  | `)` ou `/` (fermeture Un)        | Dépile le contenu, crée GenUn et l'empile                |

**Exemple concret** : quand le méta-compilateur lit `Sp -> ['a'].'b';,`

- `Sp`  → action 2 — enregistre "Sp" dans la table des symboles
- `[`   → action 6 — empile marqueur NULL (début du Star)
- `'a'` → action 5 — crée Atom('a') et l'empile
- `]`   → action 8 — dépile Atom('a'), crée Star(Atom('a')), empile
- `.`   → action 4 — empile marqueur concat (PTR)2
- `'b'` → action 5 — crée Atom('b') et l'empile
- `;`   → action 1 — construit Conc(Star('a'), 'b'), stocke dans A[6]

---

## Les fonctions de construction (Gen)

Ces fonctions créent les nœuds de l'arbre un par un.

```c
PTR GenConc(PTR p1, PTR p2)                          // nœud '.'
PTR GenUnion(PTR p1, PTR p2)                         // nœud '+'
PTR GenStar(PTR p)                                   // nœud '[X]'
PTR GenUn(PTR p)                                     // nœud '(X)'
PTR GenAtom(int cod, int act, int ind, AtomType t)   // feuille
```

Chacune alloue un nœud en mémoire avec `malloc`, remplit ses champs,
et retourne le pointeur.

---

## Le P-code

Le compilateur génère du P-code — un langage intermédiaire simple
que l'interpréteur exécute ensuite.

### Instructions disponibles

| Catégorie     | Instructions                                               |
|---------------|------------------------------------------------------------|
| Chargement    | `LDA @` (adresse), `LDV val` (valeur), `LDC c` (constante)|
| Affectation   | `AFF`                                                      |
| Entrée/Sortie | `RD` (read), `WRTLN` (writeln)                             |
| Saut          | `JMP @` (inconditionnel), `JIF @` (si faux), `JSR`, `RSR` |
| Arithmétique  | `ADD`, `MIN`, `MULT`, `DIV`, `NEG`, `INC`, `DEC`          |
| Logique       | `AND`, `OR`, `NOT`                                         |
| Comparaison   | `SUP`, `SUPE`, `INF`, `INFE`, `EG`, `DIFF`                |
| Arrêt         | `STOP`                                                     |

### Fonctionnement de l'interpréteur

L'interpréteur utilise deux structures :

- `Pcode[]` : le tableau contenant toutes les instructions générées
- `Pilex[]` : la pile d'exécution (joue le rôle des registres)
- `CO` : le compteur ordinal, pointe vers l'instruction courante

La procédure `Exec` boucle tant que `Pcode[CO] != STOP` et appelle
`Interpret(Pcode[CO])` sur chaque instruction. `Interpret` modifie
`Pilex` et `CO` selon l'instruction reçue.

---

## Structure du projet

```
projet-compilo/
├── README.md
├── rapport/
│   └── rapport.pdf
├── src/
│   ├── main.c             ← point d'entrée
│   ├── tree.h / tree.c    ← types, GenXxx, GenForetGO, ImprimArbre
│   ├── scan_go.h / .c     ← ScanGO (analyse lexicale de la GPL)
│   ├── analyse_go.h / .c  ← AnalyseGO + ActionGO
│   ├── scan_gpl.h / .c    ← ScanGPL (analyse lexicale du programme)
│   ├── analyse_gpl.h / .c ← AnalyseGPL + ActionGPL
│   ├── pcode.h / .c       ← génération du P-code
│   ├── exec.h / exec.c    ← Exec + Interpret
│   └── Makefile
└── tests/
    ├── gpl1.txt           ← Sp -> ['a'].'b';,
    ├── gpl2.txt           ← Sp -> 'a'+'b';,
    ├── gpl3.txt           ← Sp -> ['a'.'b'];,
    ├── gpl4.txt           ← Sp -> N.'b';, N -> 'a';,
    ├── gpl5.txt           ← Sp -> ('a').'b';,
    ├── gpl6.txt           ← Sp -> N+'b';, N -> 'a'+'c';,
    ├── gpl7.txt           ← Sp -> [['a'].'b'];,
    ├── gpl8.txt           ← G0 écrite en GPL (preuve G0 ∈ G0)
    ├── prog1.txt          ← aab
    ├── prog2.txt          ← a
    ├── prog3.txt          ← ababab
    ├── prog4.txt          ← ab
    ├── prog5.txt          ← b
    ├── prog6.txt          ← a
    └── prog7.txt          ← abb
```

---

## Compilation et exécution

```bash
# Compiler tout le projet
make

# Lancer sur une grammaire GPL et un programme
./compilo tests/gpl1.txt tests/prog1.txt

# Lancer tous les tests
make clean && make && \
./compilo tests/gpl1.txt tests/prog1.txt && \
./compilo tests/gpl2.txt tests/prog2.txt && \
./compilo tests/gpl3.txt tests/prog3.txt && \
./compilo tests/gpl4.txt tests/prog4.txt && \
./compilo tests/gpl5.txt tests/prog5.txt && \
./compilo tests/gpl6.txt tests/prog6.txt && \
./compilo tests/gpl7.txt tests/prog7.txt && \
./compilo tests/gpl8.txt tests/gpl1.txt

# Nettoyer les fichiers compilés
make clean
```

---

## Format des fichiers GPL

Une grammaire GPL doit respecter le format suivant :

- Chaque règle se termine par `;,`
- La dernière règle se termine aussi par `;,`
- Les non-terminaux commencent par une majuscule
- Les terminaux sont entre apostrophes : `'a'`, `'->'`, `'if'`
- La flèche s'écrit `->`

Exemple de GPL valide :
```
Sp -> N.'b';,
N -> 'a';,
```

---

## Preuve que G0 ∈ G0

G0 peut s'écrire elle-même en GPL :

```
S -> [N.'->' .E. ';'].',';,
N -> 'IDNTER';,
E -> T.['+'.T];,
T -> F.['.'.F];,
F -> 'IDNTER'+'ELTER'+'('.E.')'+'['.E.']'+'/'.E.'/';,
```

En lançant `./compilo tests/gpl8.txt tests/gpl1.txt`, le programme
confirme que `gpl1.txt` est un programme valide selon cette grammaire —
prouvant ainsi que G0 ∈ G0.