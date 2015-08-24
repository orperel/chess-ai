#include <stdio.h>
#include <stdlib.h>
#include "LinkedList.h"
#include "Chess.h"

/** 
 * Create a new empty linked list. If failed to allocate memory return NULL. 
 * "freeData" is a pointer to function that destructs the data of the nodes. If freeData is NULL do regular free.
 * If malloc has failed set g_memError to true and return NULL.
 */
LinkedList* createList(destructor freeData)
{
	LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));

	if (list == NULL)
	{
		perror_message("malloc");;
		g_memError = true;
		return NULL;
	}

	list->length = 0;
	list->head = NULL;
	list->tail = NULL;
	list->freeData = freeData;
	return list;
}

/** Recursively delete the list nodes when deleting all nodes. */
void deleteListRecursive(Node* node, destructor freeData)
{
	if (node == NULL)
	{
		return;
	}

	deleteListRecursive(node->next, freeData);
	
	if (freeData != NULL)
	{
		freeData(node->data);
	}
	else
	{
		free(node->data);
	}
	free(node);
}

/** Delete all nodes but leave the list itself. */
void deleteAllNodes(LinkedList* list)
{
	deleteListRecursive(list->head, list->freeData);
	
	list->length = 0;
}

/** Delete the entire linked list. */
void deleteList(LinkedList* list)
{
	deleteAllNodes(list);
	free(list);
}

/** 
 * Insert a new node to the end of the list with the data "data".
 * Set g_memError to true if malloc has failed.
 */
void insertLast(LinkedList* list, void* data)
{
	Node* newNode = (Node*)malloc(sizeof(Node));
	if (newNode == NULL)
	{
		perror_message("malloc");;
		g_memError = true;
		return;
	}

	newNode->data = data;
	newNode->next = NULL;

	// Add first node
	if (list->length == 0)
	{
		list->head = newNode;
	}
	else
	{
		list->tail->next = newNode;
	}

	list->tail = newNode;

	list->length++;
}

/** Delete the node "node" from the list. If the node doesn't exist do nothing. */
void deleteNode(LinkedList* list, Node* node)
{
	Node* curr = list->head;

	// List is empty
	if (curr == NULL)
	{
		return;
	}

	// Search for the node to be deleted
	while ((curr->next != NULL) && (curr->next != node))
	{
		curr = curr->next;
	}
	
	if (list->length == 1)	// Single node
	{
		if (curr == node)
		{
			list->head = NULL;
			list->tail = NULL;
		}
		else
		{	// Node is different than the single head
			return;
		}
	}
	else if (curr->next == NULL)	// Node isn't in the list 
	{
		return;
	}
	else
	{	// curr->next == node, can be the last node
		if (node->next == NULL)
		{
			list->tail = NULL;
		}

		curr->next = node->next;
	}

	list->length--;
	if (list->freeData != NULL)
	{
		list->freeData(node->data);
	}
	else
	{
		free(node->data);
	}
	free(node);
}

/** Concatenate list2 after list1, the result will be in list1. */
void concat(LinkedList* list1, LinkedList* list2)
{
	if (list2->length == 0)
	{
		return;
	}
	
	if (list1->length == 0)
	{
		list1->head = list2->head;
		list1->tail = list2->tail;
	}
	else
	{	// List1 and List2 aren't empty
		list1->tail->next = list2->head;
		list1->tail = list2->tail;
	}
}