#ifndef _PATHFINDING_H_
#define _PATHFINDING_H_

#include "gf2d_vector.h"
#include "priority_queue.h"
#include "map.h"

typedef struct PF_Node_S PF_Node;
typedef struct PF_Edge_S PF_Edge;
typedef struct PF_Path_S PF_Path;

typedef struct PF_Node_S
{
	Vector2D pos;
	char tile;
	PF_Edge *connections[4];
	int num_con;
} PF_Node;

typedef struct PF_Edge_S
{
	PF_Node *a;
	Uint32 weight;
	PF_Node *b;
} PF_Edge;

typedef struct PF_Graph_S
{
	PF_Node **nodes;
	int max;
} PF_Graph;

typedef struct PF_Path_S
{
	PF_Path *parent;
	PF_Node *current;
	PF_Edge *edgeTaken;
} PF_Path;

typedef struct PF_PathArray_S
{
	int count;
	Vector2D *path;
} PF_PathArray;

PF_Graph *pathfinding_generate_graph_from_tilemap(TileMap *t);

PF_Path *pathfinding_get_path(PF_Graph *graph, Vector2D start, Vector2D end);

PF_PathArray *convert_path_to_vector2d_array(PF_Path *path);

PF_Graph *get_current_graph();

void path_free_all_parents(PF_Path *path);

void patharray_free(PF_PathArray *pa);

void path_free(PF_Path *path);
#endif