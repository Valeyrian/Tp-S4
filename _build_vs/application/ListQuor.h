

// Types nécessaires
#include "core/quoridor_ai.h"


// Nœud de la liste
typedef struct Node {
    QuoridorWall wall;
    struct Node* next;
} Node;

// Liste
typedef struct {
    Node* head;
} ListQuor;

// Fonctions
void init_list(ListQuor* list);
void insert(ListQuor* list, QuoridorWall wall);
QuoridorWall pop_first(ListQuor* list);
void free_list(ListQuor* list);
QuoridorWall pop_last(ListQuor* list);
