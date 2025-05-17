#include <stdlib.h>
#include "listquor.h"

void init_list(ListQuor* list) {
    list->head = NULL;
}

void insert(ListQuor* list, QuoridorWall wall) {
    NodeQuor* new_node = (NodeQuor*)malloc(sizeof(NodeQuor));
    if (!new_node) return;

    new_node->wall = wall;
    new_node->next = NULL;

    if (!list->head || wall.score < list->head->wall.score) {
        new_node->next = list->head;
        list->head = new_node;
        return;
    }

    NodeQuor* current = list->head;
    while (current->next && current->next->wall.score < wall.score) {
        current = current->next;
    }

    new_node->next = current->next;
    current->next = new_node;
}

QuoridorWall pop_first(ListQuor* list) {
    NodeQuor* temp = list->head;
    QuoridorWall wall = temp->wall;
    list->head = temp->next;
    free(temp);
    return wall;
}

void free_list(ListQuor* list) {
    NodeQuor* current = list->head;
    while (current) {
        NodeQuor* next = current->next;
        free(current);
        current = next;
    }
    list->head = NULL;
}
QuoridorWall pop_last(ListQuor* list) {

    NodeQuor* current = list->head;
    while (current->next->next) {
        current = current->next;
    }

    QuoridorWall wall = current->next->wall;
    free(current->next);
    current->next = NULL;
    return wall;
}
