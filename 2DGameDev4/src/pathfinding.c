#include "pathfinding.h"
#include "simple_logger.h"
#include <stdint.h>

PF_Graph *currentGraph;

PF_Graph *get_current_graph()
{
	return currentGraph;
}

void set_current_graph(PF_Graph *graph)
{
	currentGraph = graph;
}

PF_Path *path_new()
{
	PF_Path *path;

	path = (PF_Path *)malloc(sizeof(PF_Path));
	if (!path)
	{
		slog("unable to allocate path");
		return NULL;
	}
	memset(path, 0, sizeof(PF_Path));
	return path;
}

void patharray_free(PF_PathArray *pa)
{
	if (!pa)
		return;
	if (pa->path != NULL)
		free(pa->path);
	free(pa);
}

void path_free_all_parents(PF_Path *path)
{
	PF_Path *parent, *cursor;
	int i = 0;
	if (!path)
		return;
	cursor = path;
	while (cursor != NULL)
	{
		parent = cursor->parent;
		path_free(cursor);
		cursor = parent;
	}
}

void path_free(PF_Path *path)
{
	if (!path)
		return;
	free(path);
}

PF_Graph *pathfinding_new_graph(int max)
{
	PF_Graph *graph;
	if (max == 0)
	{
		slog("cannot create a pf_graph with 0 elements");
		return NULL;
	}
	graph = (PF_Graph *)malloc(sizeof(PF_Graph));
	memset(graph, NULL, sizeof(PF_Graph));

	graph->nodes = (PF_Node *)malloc(sizeof(PF_Node)*max);
	memset(graph->nodes, NULL, sizeof(PF_Node)*max);

	graph->max = max;
	return graph;
}

PF_Edge *pathfinding_new_edge()
{
	PF_Edge *edge;
	edge = (PF_Edge *)malloc(sizeof(PF_Edge));
	if (!edge)
	{
		slog("unable to allocate new linklist node");
		return NULL;
	}
	memset(edge, 0, sizeof(PF_Edge));
	return edge;
}

PF_Node *pathfinding_new_node()
{
	PF_Node *node;
	node = (PF_Node *)malloc(sizeof(PF_Node));
	if (!node)
	{
		slog("unable to allocate new linklist node");
		return NULL;
	}
	memset(node, 0, sizeof(PF_Node));
	return node;
}

void pathfinding_edge_set(PF_Edge *edge, PF_Node *a, PF_Node *b)
{
	edge->a = a;
	edge->b = b;
	if ((a->tile != '0') || (b->tile != '0'))
		edge->weight = UINT32_MAX;
	else
		edge->weight = 1;
}

PF_Graph *pathfinding_generate_graph_from_tilemap(TileMap *t)
{
	PF_Graph *graph;
	PF_Node *node;
	PF_Edge *edge;
	int counter;
	Uint32 i;
	Uint32 j;
	graph = pathfinding_new_graph(t->width * t->height);
	counter = 0;
	for (i = 0; i < t->height; i++)
	{
		for (j = 0; j < t->width; j++)
		{
			node = pathfinding_new_node();
			node->tile = t->map[(t->width * i) + j];
			node->pos = vector2d((double)j, (double)i);
			graph->nodes[counter] = node;
			if (j > 0)
			{
				edge = pathfinding_new_edge();
				pathfinding_edge_set(edge, node, graph->nodes[(t->width * i) + j - 1]);
				node->connections[node->num_con] = edge;
				node->num_con += 1;
				graph->nodes[(t->width * i) + j - 1]->connections[graph->nodes[(t->width * i) + j - 1]->num_con] = edge;
				graph->nodes[(t->width * i) + j - 1]->num_con += 1;
			}
			if (i > 0)
			{
				edge = pathfinding_new_edge();
				pathfinding_edge_set(edge, node, graph->nodes[((t->width * (i - 1)) + j)]);
				node->connections[node->num_con] = edge;
				node->num_con += 1;
				graph->nodes[((t->width * (i - 1)) + j)]->connections[graph->nodes[((t->width * (i - 1)) + j)]->num_con] = edge;
				graph->nodes[((t->width * (i - 1)) + j)]->num_con += 1;
			}
			counter += 1;
		}
	}
	set_current_graph(graph);
	return graph;
}

PF_Node *pathfinding_get_node_from_vector2D(PF_Graph *graph, Vector2D pos)
{
	int i;
	for (i = 0; i < graph->max; i++)
	{
		if ((pos.x == graph->nodes[i]->pos.x) && (pos.y == graph->nodes[i]->pos.y))
		{
			return graph->nodes[i];
		}
	}
	return NULL;
}

PF_Node *pathfinding_get_other_from_edge(PF_Node *self, PF_Edge *edge)
{
	if (edge->a == self)
	{
		return edge->b;
	}
	else if (edge->b == self)
	{
		return edge->a;
	}
	else
	{
		slog("Node was not on the edge");
		return NULL;
	}
}

int is_edge_in_list(PriorityQueueList *pq, PF_Edge *edge)
{
	PriorityNode *pn;
	PF_Path *tmp;
	if (!pq)
	{
		slog("no pqlist to check");
		return -1;
	}
	if (!edge)
	{
		slog("no edge to find");
		return -1;
	}
	if (pq->head == NULL)
	{
		//slog("pqlist is empty");
		return 0;
	}
	pn = pq->head;
	while (pn != NULL)
	{
		if ((PF_Edge *)pn->data == edge)
			return 1;
		pn = pn->next;
	}
	return 0;
}

int pathfinding_get_heuristic(PF_Node *a, PF_Node *b)
{
	int xdif;
	int ydif;
	if ((!a) || (!b))
		return -1;
	if (a->pos.x > b->pos.x)
		xdif = a->pos.x - b->pos.x;
	else
		xdif = b->pos.x - a->pos.x;
	if (a->pos.y > b->pos.y)
		ydif = a->pos.y - b->pos.y;
	else
		ydif = b->pos.y - a->pos.y;
	return xdif + ydif;
}

PF_PathArray *patharray_new()
{
	PF_PathArray *pa;
	pa = (PF_PathArray *)malloc(sizeof(PF_PathArray));
	if (!pa)
	{
		slog("unable to allocate a new patharray");
		return NULL;
	}
	memset(pa, 0, sizeof(PF_PathArray));
	return pa;
}

PF_PathArray *convert_path_to_vector2d_array(PF_Path *path)
{
	int i;
	PF_Path *cursor;
	PF_PathArray *rtn;
	Vector2D *v;
	if (!path)
	{
		slog("cannot convert a null path into an array..");
		return NULL;
	}
	i = 0;
	cursor = path;
	while (cursor != NULL)
	{
		cursor = cursor->parent;
		i++;
	}
	rtn = patharray_new();
	if (!rtn)
	{
		slog("null patharray allocation");
		return NULL;
	}
	v = (Vector2D *)malloc(sizeof(Vector2D) * i);
	if (!v)
	{
		slog("unable to allocate vector2d array");
		// well.. get rid of this then
		free(rtn);
		return NULL;
	}
	memset(v, 0, sizeof(Vector2D) * i);
	rtn->count = i;
	rtn->path = v;
	cursor = path;
	i = 0;
	while (i < rtn->count)
	{
		rtn->path[i].x = cursor->current->pos.x;
		rtn->path[i].y = cursor->current->pos.y;
		cursor = cursor->parent;
		i++;
	}
	return rtn;
}

int is_sub_path_on_path(PF_Path *sub, PF_Path *path)
{
	PF_Path *cursor = path;
	if ((!path) || (!sub))
		return -1;
	while (cursor != NULL)
	{
		if (cursor == sub)
			return 1;
		cursor = cursor->parent;
	}
	return 0;
}

PF_Path *pathfinding_get_path(PF_Graph *graph, Vector2D start, Vector2D end)
{
	PriorityQueueList *pq;
	PriorityQueueList *pqEdges;
	PriorityQueueList *pqPaths;
	PF_Path *path;
	PF_Path *tmpPath;
	PF_Node *startNode;
	PF_Node *endNode;
	PF_Path *curPath;
	int i;
	int priority;

	startNode = pathfinding_get_node_from_vector2D(graph, start);
	endNode = pathfinding_get_node_from_vector2D(graph, end);
	if ((!startNode) || (!endNode))
	{
		slog("cannot find a path, cannot locate start or end node");
	}
	pq = pqlist_new();
	pqEdges = pqlist_new();
	pqPaths = pqlist_new();
	if ((!pq)||(!pqEdges)||(!pqPaths))
	{
		slog("Unable to find a path without a pqlist");
		return NULL;
	}
	path = path_new();
	path->current = startNode;
	curPath = path;
	priority = pathfinding_get_heuristic(path->current, endNode);
	pqlist_insert(pq, path, priority * -1);
	while (curPath->current != endNode)
	{
		// insert all edges into the pqlist that are not already in the pqlist
		for (i = 0; i < curPath->current->num_con; i++)
		{
			// ignore edges that are not passable
			if (curPath->current->connections[i]->weight == UINT32_MAX)
				continue;
			// ignore edges that have already been walked over
			if (is_edge_in_list(pqEdges, curPath->current->connections[i]) == 1)
				continue;
			path = path_new();
			if (!path)
			{
				slog("Unable to allocate path data for pathfinding");
				return NULL;
			}
			path->edgeTaken = curPath->current->connections[i];
			path->current = pathfinding_get_other_from_edge(curPath->current,curPath->current->connections[i]);
			path->parent = curPath;
			priority = pathfinding_get_heuristic(path->current, endNode);
			if (priority < 0)
			{
				slog("Error getting the heuristic");
				return NULL;
			}
			pqlist_insert(pqEdges, path->edgeTaken, 1);
			pqlist_insert(pq, path, priority * -1);
			pqlist_insert(pqPaths, path, 1);
		}

		curPath = pqlist_delete_max(pq);
		if (curPath == NULL)
			break;
	}
	// free the temporary paths we built on the way
	path = pqlist_delete_max(pqPaths);
	while (path != NULL)
	{
		if (is_sub_path_on_path(path, curPath) == 0)
		{
			tmpPath = path;
			path_free(tmpPath);
		}
		path = pqlist_delete_max(pqPaths);
	}
	free(pqPaths);
	// free the other two queues
	while (pqlist_delete_max(pqEdges) != NULL);
	while (pqlist_delete_max(pq) != NULL);
	free(pqEdges);
	free(pq);
	return curPath;
}
