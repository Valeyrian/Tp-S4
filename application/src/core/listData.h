
#pragma once 

//#include "core/quoridor_ai.h"
#include "core/quoridor_core.h"
#include "settings.h"

#define MAX_BACK_ANALYS 250 //nombre de coups retenus par joueurs 

typedef struct QuoridorData
{
    /// @brief Type de mur (horizontal ou vertical).
    QuoridorAction action;
    QuoridorPos destPos;
    QuoridorPos originPos;
    int score;
} QuoridorData;

typedef struct NodeData
{
    QuoridorData data;
    struct NodeData* next;
} NodeData;
typedef struct ListData
{
    NodeData* head;
} ListData;





// Fonctions

//on decale tt a droite
void ListData_insertFirstPopLast(ListData* list, QuoridorData data);

ListData* ListData_create();

void ListData_destroy(ListData* list);

void ListData_insertFirst(ListData* list, QuoridorData data);

int ListData_popLast(ListData* list, QuoridorData* outData);

QuoridorData ListData_popFirst(ListData* list);  



void ListData_debug(ListData* list, const char* name);