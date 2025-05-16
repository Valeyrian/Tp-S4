#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

Graph* Graph_create(int size) {
    Graph* graph = malloc(sizeof(Graph));
    if (!graph) return NULL;

    graph->size = size;
    graph->nodes = calloc(size, sizeof(GraphNode));
    if (!graph->nodes) {
        free(graph);
        return NULL;
    }
    return graph;
}

void Graph_destroy(Graph* graph) {
    if (!graph) return;

    for (int i = 0; i < graph->size; i++) {
        ArcList* arc = graph->nodes[i].arcList;
        while (arc) {
            ArcList* temp = arc;
            arc = arc->next;
            free(temp);
        }
    }
    free(graph->nodes);
    free(graph);
}

void Graph_setArc(Graph* graph, int u, int v, float weight) {
    assert(graph);
    assert(0 <= u && u < graph->size);
    assert(0 <= v && v < graph->size);

    ArcList** arcPtr = &graph->nodes[u].arcList;
    while (*arcPtr && (*arcPtr)->target < v) {
        arcPtr = &((*arcPtr)->next);
    }
    if (*arcPtr && (*arcPtr)->target == v) {
        (*arcPtr)->weight = weight;
    }
    else {
        ArcList* newArc = malloc(sizeof(ArcList));
        newArc->target = v;
        newArc->weight = weight;
        newArc->next = *arcPtr;
        *arcPtr = newArc;
        graph->nodes[u].arcCount++;
    }
}

void Graph_removeArc(Graph* graph, int u, int v) {
    assert(graph);
    assert(0 <= u && u < graph->size);
    assert(0 <= v && v < graph->size);

    ArcList** arcPtr = &graph->nodes[u].arcList;
    while (*arcPtr && (*arcPtr)->target < v) {
        arcPtr = &((*arcPtr)->next);
    }
    if (*arcPtr && (*arcPtr)->target == v) {
        ArcList* temp = *arcPtr;
        *arcPtr = temp->next;
        free(temp);
        graph->nodes[u].arcCount--;
    }
}

float* Graph_getArc(Graph* graph, int u, int v) {
    assert(graph);
    assert(0 <= u && u < graph->size);
    assert(0 <= v && v < graph->size);

    ArcList* arc = graph->nodes[u].arcList;
    while (arc) {
        if (arc->target == v) {
            return &(arc->weight);
        }
        arc = arc->next;
    }
    return NULL;
}

Graph* Graph_load(char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        assert(false && "File opening failed");
        return NULL;
    }

    int n, m;
    fscanf(file, "%d %d", &n, &m);
    Graph* graph = Graph_create(n);
    if (!graph) {
        fclose(file);
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        int u, v;
        float weight;
        fscanf(file, "%d %d %f", &u, &v, &weight);
        Graph_setArc(graph, u, v, weight);
    }

    fclose(file);
    return graph;
}

void Graph_print(Graph* graph)
{
    assert(graph && "The Graph must be created");

    for (int currID = 0; currID < graph->size; currID++)
    {
        printf("Node %d:\n", currID);

        for (ArcList* arc = Graph_getArcList(graph, currID); arc != NULL; arc = arc->next)
        {
            int target = arc->target;
            float weight = arc->weight;
            printf("  -> Node %d (Weight: %.2f)\n", target, weight);
        }
    }
}
void Graph_Write(Graph* graph, char* filename) {

    if (!graph) {
        return NULL;
    }

    FILE* file = fopen(filename, "w");

    int size = 0;
    for (int i = 0;i < graph->size;i++)
        size += graph->nodes[i].arcCount;
    fprintf(file, "%d %d\n", graph->size, size);


    for (int i = 0; i < graph->size; i++) {

        ArcList* arc = graph->nodes[i].arcList;
        while (arc)
        {
            fprintf(file, "%d %d %f\n", i, arc->target, arc->weight);
            arc = arc->next;
        }
    }

    fclose(file);
}