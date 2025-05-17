#ifndef LISTQUOR_H
#define LISTQUOR_H
// types ici



// Types nécessaires
#include "core/quoridor_ai.h"


typedef struct NodeQuor NodeQuor;
struct NodeQuor
{
    QuoridorWall wall;
    NodeQuor* next;
};

// Liste
typedef struct ListQuor 
{
    NodeQuor* head;
} ListQuor;

// Fonctions
ListQuor* ListQuor_create();
void ListQuor_insert(ListQuor* list, QuoridorWall wall);
QuoridorWall ListQuor_pop_first(ListQuor* list);
void ListQuor_free(ListQuor* list);
QuoridorWall ListQuor_pop_last(ListQuor* list);
#endif