
#pragma once 

#include "core/quoridor_ai.h" // pour QuoridorPos

typedef struct QuoridorData
{
    /// @brief Type de mur (horizontal ou vertical).
    QuoridorAction action;
    QuoridorPos pos;
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





