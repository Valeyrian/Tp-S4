#include <stdlib.h>
#include <stdio.h>
#include "core/quoridor_core.h"
#include "core/quoridor_ai.h"
#include "core/listData.h" // Assurez-vous de cr�er ce .h avec les d�finitions de NodeData et ListData

// Cr�e une nouvelle liste vide
ListData* ListData_create()
{
   
    ListData* list = (ListData*)malloc(sizeof(ListData));
   
    if (list == NULL) {
        fprintf(stderr, "Erreur d'allocation m�moire pour la liste.\n");
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

    // �tape 1 : se placer au d�but
    NodeData* current = list->head;
    QuoridorData tmp1 = current->data;
    current->data = data;

    // �tape 2 : propager vers la droite
    int count = 1;
    while (current->next != NULL && count < MAX_BACK_ANALYS) 
    {
        current = current->next;

        QuoridorData tmp2 = current->data;
        current->data = tmp1;
        tmp1 = tmp2;

        count++;
    }
    // Si la liste contient plus d'�l�ments que 'size', on ignore le reste (il sera �cras� plus tard ou jamais acc�d�)
}



// Ins�re un nouvel �l�ment en d�but de liste
void ListData_insertFirst(ListData* list, QuoridorData data) 
{
    if (list == NULL) return;
    NodeData* newNode = (NodeData*)malloc(sizeof(NodeData));
    if (newNode == NULL) {
        fprintf(stderr, "Erreur d'allocation m�moire pour un nouveau n�ud.\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
}

// Supprime et retourne le dernier �l�ment de la liste
// Retourne 1 si un �l�ment a �t� supprim�, 0 si la liste est vide
int ListData_popLast(ListData* list, QuoridorData* outData)
{
    if (list == NULL || list->head == NULL) return 0;

    NodeData* current = list->head;
    NodeData* prev = NULL;

  

    if (prev == NULL) {
        // Un seul �l�ment dans la liste
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


