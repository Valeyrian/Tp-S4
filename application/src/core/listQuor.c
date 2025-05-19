#include <stdlib.h>
#include "listquor.h"

ListQuor* ListQuor_create() {
    ListQuor* list = (ListQuor*)malloc(sizeof(ListQuor));
    if (!list) return NULL;
    list->head = NULL;
    return list;
}

NodeQuor* ListQuor_pop_first(ListQuor* list) {
    if (!list || !list->head) return NULL;

    NodeQuor* temp = list->head;
    list->head = temp->next;
    temp->next = NULL; // couper le lien
    return temp;
}


NodeQuor* ListQuor_pop_last(ListQuor* list) {
    NodeQuor* current = list->head;

    // Cas spécial : un seul élément
    if (!current->next) {

        return current;
    }

    // Parcours jusqu'à l'avant-dernier élément
    while (current->next) {
        current = current->next;
    }

    return current;
}

void ListQuor_insert_last(ListQuor* list, QuoridorPos pos, NodeQuor* parent) {
    NodeQuor* new_node = (NodeQuor*)malloc(sizeof(NodeQuor));
    if (!new_node) return;

    new_node->pos = pos;
    new_node->parent = parent;
    new_node->next = NULL;

    if (!list->head) {
        list->head = new_node;
        return;
    }

    NodeQuor* current = list->head;
    while (current->next) {
        current = current->next;
    }

    current->next = new_node;
}



void ListQuor_free(ListQuor* list) {
    NodeQuor* current = list->head;
    while (current) {
        NodeQuor* next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
}

int ListQuor_size(ListQuor* list)
{
    if (!list->head)
        return 0;
    int tmp = 0;
    NodeQuor* current = ListQuor_pop_last(list);
    while (current)
    {
        tmp++;
        current = current->parent;
    }
    return tmp;
}