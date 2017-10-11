#ifndef _PRIORITY_QUEUE_H
#define _PRIORITY_QUEUE_H
#include <SDL.h>

typedef struct PriorityNode_S
{
	void *data;
	int *priority;
	struct PriorityNode_S *next;
}PriorityNode;

typedef struct
{
	PriorityNode *nodeList;
	int numElements;
}PriorityQueue;

typedef struct
{
	PriorityNode *head;
}PriorityQueueList;

/**
@brief allocate memory for a new priorityqueue
@param numElements the number of elements that the pq can contain
@return a pointer to the priorityqueue
*/
PriorityQueue *pq_new(int numElements);

/*
@brief free the memory for a given priorityqueue
@param pq the queue to be free'ed
*/
void pq_free_queue(PriorityQueue *pq);

/*
@brief clear out the data from a given PriorityNode
@param pnode the node to be cleared
*/
void pq_delete(PriorityNode *pnode);

/*
@brief pop off the maximum priority data from the queue
@param pq the queue to remove from
@return a pointer to the data that has been removed
*/
void *pq_delete_max(PriorityQueue *pq);

/*
@brief insert new data into the queue
@param pq the queue to insert into
@param data the data to be inserted
@param priority the priority level of the data
*/
void pq_insert(PriorityQueue *pq, void *data, int priority);

/*
@brief pop off the maximum priority data from the list
@param pqlist the list the remove from
@return a pointer to the data that has been removed
*/
void *pqlist_delete_max(PriorityQueueList *pqlist);

/*
@brief declare a new priority queue without a set number of elements
@return a pointer to the new pqlist
*/
PriorityQueueList *pqlist_new();

/*
@brief insert some new data into a pqlist
@param pqlist the queue to insert into
@param data the data to be inserted
@param priority the priority level of the data
*/
void pqlist_insert(PriorityQueueList *pqlist, void *data, int priority);

#endif
#pragma once
