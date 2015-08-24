#ifndef LINKEDLIST_
#define LINKEDLIST_

// The node struct
struct Node
{
	void* data;
	struct Node* next;
};

typedef struct Node Node;

// Pointer to function that destructs the data of the nodes
typedef void(*destructor)(void*);

// The linked list
typedef struct
{
	int length;
	Node* head;
	Node* tail;
	destructor freeData;
} LinkedList;

// Linked list operations
LinkedList* createList(destructor freeData);

void deleteList(LinkedList* list);

void insertLast(LinkedList* list, void* data);

void deleteNode(LinkedList* list, Node* node);

void deleteAllNodes(LinkedList* list);

void concat(LinkedList* list1, LinkedList* list2);

#endif