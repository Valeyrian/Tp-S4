

#include "core/quoridor_core.h" // pour QuoridorPos

typedef struct NodeQuor {
    QuoridorPos pos;
    struct NodeQuor* parent;   // <-- ici, parent devient un pointeur
    struct NodeQuor* next;
} NodeQuor;

typedef struct ListQuor {
    NodeQuor* head;
} ListQuor;

// Fonctions
ListQuor* ListQuor_create();
void ListQuor_insert_last(ListQuor* list, QuoridorPos pos, NodeQuor* parent);
NodeQuor* ListQuor_pop_first(ListQuor* list);
NodeQuor* ListQuor_pop_last(ListQuor* list);
void ListQuor_free(ListQuor* list);
int ListQuor_size(ListQuor* list);
