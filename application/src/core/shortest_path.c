/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#include "shortest_path.h"

Path *Graph_shortestPath(Graph *graph, int start, int end)
{
    // TODO
    int* predecessors = calloc(graph->size, sizeof(int));
    int* distances = calloc(graph->size, sizeof(int));
    Graph_dijkstra(graph, start, end, predecessors, distances);
    //for (int i = 0;i < graph->size;i++)
      //  printf("\n%d ", predecessors[i]);
    Path* path = Graph_dijkstraGetPath(predecessors, distances, end);
    if(path);
        return path;
    return NULL;
}

void Graph_dijkstra(Graph *graph, int start, int end, int *predecessors, float *distances)
{
    assert(graph && "The Graph must be created");
    assert(0 <= start && start < graph->size && "The start is invalid");
    assert(end < graph->size && "The end is invalid");
    assert(predecessors && distances);

    const int size = Graph_size(graph);

    assert(0 <= start && start < size);
    assert(end < size);
    bool* explo = calloc(size, sizeof(bool));
    for (int i = 0;i < size;i++)
    {
        predecessors[i] = -1;
        distances[i] = INFINITY;
    }
    int stop = 0;
    distances[start] = 0;
    int min = INFINITY;
    int Id = 0;
    while (stop == 0)
    {
        Id = 0;
        min = INT16_MAX;
        for (int i = 0;i < size;i++)
        {
           // printf("id = %d, distance = %f, explo = %d min = %d\n", i, distances[i], explo[i],min);

            if (distances[i] < min && explo[i] == 0)
            {   
                min = distances[i];
                Id = i;
            }
        }
        //printf("id = %d, distance = %d, explo = %d\n", Id, distances[Id], explo[Id]);
        if (distances[Id] == INFINITY)
            break;
        if (explo[Id] == 1)
            break;
        explo[Id] = 1;
        for (ArcList* arc = graph->nodes[Id].arcList; arc; arc = arc->next)
        {
            if (distances[Id] + arc->weight < distances[arc->target])
            {
                predecessors[arc->target] = Id;
                distances[arc->target] = distances[Id] + arc->weight;
               // printf("\ndistance de %d = %f\n", arc->target, distances[arc->target]);
            }
        }
        stop = 1;
        for (int i = 0;i < size;i++)
        {
            
            if (explo[i] == 0)
                stop = 0;
        }

    }
    
    // TODO
}

Path *Graph_dijkstraGetPath(int *predecessors, float *distances, int end)
{
    assert(predecessors && distances);
    assert(end >= 0);
    if (predecessors[end] == -1)
        return NULL;
    
    int tmp = end;
    while (true)
    {
        //printf("bb");

        tmp = predecessors[tmp];
        if (predecessors[tmp] == -1)
            break;  
    }
    Path* path = Path_create(tmp);
    path->distance = distances[end];
    path->list = ListInt_create();

    tmp = end;

    while (true)
    {
        //printf("bb");
        ListInt_insertLast(path->list, tmp);
        tmp = predecessors[tmp];
        if (predecessors[tmp] == -1)
        {
            ListInt_insertLast(path->list, tmp);
                break;

        }
    }


    // TODO
    
    return path;
}

Path *Path_create(int start)
{
    Path *path = (Path *)calloc(1, sizeof(Path));
    AssertNew(path);

    path->distance = 0.0f;
    path->list = ListInt_create();
    ListInt_insertLast(path->list, start);

    return path;
}

void Path_destroy(Path *path)
{
    if (path == NULL) return;

    ListInt_destroy(path->list);
    free(path);
}

void Path_print(Path *path)
{
    if (path == NULL)
    {
        printf("path : NULL\n");
        return;
    }

    printf("path (distance = %f) : ", path->distance);
    ListInt_print(path->list);
}
