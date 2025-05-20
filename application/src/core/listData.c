#include <stdlib.h>
#include <stdio.h>
#include "core/quoridor_core.h"
#include "core/quoridor_ai.h"
#include "core/listData.h" // Assurez-vous de créer ce .h avec les définitions de NodeData et ListData

// Crée une nouvelle liste vide
ListData* ListData_create()
{
   
    ListData* list = (ListData*)malloc(sizeof(ListData));
   
    if (list == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la liste.\n");
        exit(EXIT_FAILURE);
    }

    list->head = NULL;
    return list;
}

void ListData_destroy(ListData* list)
{
    if (list == NULL) return;

    NodeData* current = list->head;

    while (current != NULL) 
    {
        NodeData* next = current->next;
        free(current);
        current = next;
    }

    free(list);
}


//decale les element vers la droite
void ListData_insertFirstPopLast(ListData* list, QuoridorData data)
{
    if (list == NULL || list->head == NULL) return;

    // Étape 1 : se placer au début
    NodeData* current = list->head;
    QuoridorData tmp1 = current->data;
    current->data = data;

    // Étape 2 : propager vers la droite
    int count = 1;
    while (current->next != NULL && count < MAX_BACK_ANALYS) 
    {
        current = current->next;

        QuoridorData tmp2 = current->data;
        current->data = tmp1;
        tmp1 = tmp2;

        count++;
    }
    // Si la liste contient plus d'éléments que 'size', on ignore le reste (il sera écrasé plus tard ou jamais accédé)
}



// Insère un nouvel élément en début de liste
void ListData_insertFirst(ListData* list, QuoridorData data) 
{
    if (list == NULL) return;
    NodeData* newNode = (NodeData*)malloc(sizeof(NodeData));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour un nouveau nœud.\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
}

// Supprime et retourne le dernier élément de la liste
// Retourne 1 si un élément a été supprimé, 0 si la liste est vide
int ListData_popLast(ListData* list, QuoridorData* outData)
{
    if (list == NULL || list->head == NULL) return 0;

    NodeData* current = list->head;
    NodeData* prev = NULL;

  

    if (prev == NULL) {
        // Un seul élément dans la liste
        list->head = NULL;
    }
    else {
        prev->next = NULL;
    }

    if (outData != NULL) {
        *outData = current->data;
    }



    free(current);
    return 1;
}


