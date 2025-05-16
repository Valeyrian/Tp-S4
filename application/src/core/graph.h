/*
    Copyright (c) Arnaud BANNIER, Nicolas BODIN and Matthieu LE BERRE.
    Licensed under the MIT License.
    See LICENSE.md in the project root for license information.
*/

#pragma once

#include "settings.h"

typedef struct Graph Graph;
typedef struct GraphNode GraphNode;
typedef struct ArcList ArcList;

/// @brief Structure représentant un graphe orienté.
struct Graph
{
    /// @brief Tableau contenant les noeuds du graphe.
    GraphNode *nodes;

    /// @brief Nombre de noeuds du graphe.
    int size;
};

/// @brief Structure représentant un noeud d'un graphe.
struct GraphNode
{
    /// @brief Degré sortant du noeud.
    int arcCount;

    /// @brief Liste simplement chaînée des arcs sortants du noeud.
    /// Cette liste est triée selon les identifiants des noeuds de destination
    /// (dans l'ordre croissant).
    /// Vaut NULL si arcCount = 0.
    ArcList *arcList;
};

/// @brief Structure représentant une liste simplement chaînée des arcs sortants d'un noeud.
struct ArcList
{
    /// @brief Pointeur vers l'élément suivant de la liste.
    /// Vaut NULL s'il s'agit du dernier élément.
    ArcList *next;

    /// @brief Identifiant du noeud de destination de l'arc.
    int target;

    /// @brief Poids de l'arc.
    float weight;
};

/// @brief Crée un nouveau graphe.
/// Le nombre de noeuds doit être défini au moment de la création et ne peut
/// plus être modifié par la suite.
/// @param size le nombre de noeuds du graphe.
/// @return Le graphe créé.
Graph *Graph_create(int size);

/// @brief Détruit un graphe créé avec Graph_create().
/// @param graph le graphe.
void Graph_destroy(Graph *graph);

/// @brief Crée un nouveau graphe à partir du graphe stocké dans le fichier filename.
/// @param filenamne chemin vers le fichier texte contenant le graphe.
/// @return Le graphe créé.
Graph *Graph_load(char *filename);

/// @brief Affiche le graphe.
/// @param graph le graphe.
void Graph_print(Graph *graph);

void Graph_Write(Graph* graph, char* filename);


/// @brief Renvoie le nombre de noeuds d'un graphe.
/// @param graph le graphe.
/// @return Le nombre de noeuds du graphe.
INLINE int Graph_size(Graph *graph)
{
    assert(graph && "The Graph must be created");
    return graph->size;
}

/// @brief Retourne le nombre de voisins du noeud u.
/// @param graph le graphe.
/// @param u l'identifiant du noeud.
/// @return Le degré sortant de u.
INLINE int Graph_getArcCount(Graph *graph, int u)
{
    assert(graph && "The Graph must be created");
    assert(0 <= u && u < graph->size && "The node ID is invalid");
    return graph->nodes[u].arcCount;
}

/// @brief Retourne la liste d'arcs reliant u à ses voisins.
/// @param graph le graphe.
/// @param u l'identifiant du noeud.
/// @return La liste d'arcs reliant u à ses voisins.
INLINE ArcList *Graph_getArcList(Graph *graph, int u)
{
    assert(graph && "The Graph must be created");
    assert(0 <= u && u < graph->size && "The node ID is invalid");
    return graph->nodes[u].arcList;
}

/// @brief Retourne un pointeur sur le poids de l'arc entre les sommets u et v
/// ou NULL s'il n'existe pas.
/// @param graph le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
/// @return Un pointeur sur la donnée de l'arc.
float *Graph_getArc(Graph *graph, int u, int v);

/// @brief Modifie le poids de l'arc entre les sommets u et v
/// ou crée cet arc s'il n'existe pas.
/// @param graph le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
/// @param weight le poids de l'arc.
void Graph_setArc(Graph *graph, int u, int v, float weight);

/// @brief Supprime l'arc entre les sommets u et v s'il existe.
/// @param graph le graphe.
/// @param u l'identifiant du noeud source.
/// @param v l'identifiant du noeud de destination.
void Graph_removeArc(Graph *graph, int u, int v);
