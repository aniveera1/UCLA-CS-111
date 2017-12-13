/* NAME: Anirudh Veeraragavan
 * EMAIL: aveeraragavan@g.ucla.edu
 * ID: 004767663
 */

#include "SortedList.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
	SortedListElement_t* prev = list;
	SortedListElement_t* curr = prev->next;

	// Find first node greater than or equal to element, insert before said node
	while (curr)
	{
		if (strcmp(element->key, curr->key) <= 0)
		{
			break;
		}
		prev = curr;
		curr = curr->next;
	}

	if (opt_yield & INSERT_YIELD)
	{
		sched_yield();
	}

	element->prev = prev;
	element->next = curr;
	prev->next = element;

	// If not last element update curr, otherwise update head
	if (curr)
	{
		curr->prev = element;
	}
}

/**
 * SortedList_delete ... remove an element from a sorted list
 * @param SortedListElement_t *element ... element to be removed
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 */
int SortedList_delete( SortedListElement_t *element)
{
	if (element == NULL)
	{
		return 1;
	}

	SortedListElement_t* prev = element->prev;
	SortedListElement_t* next = element->next;

	if (opt_yield & DELETE_YIELD)
	{
		sched_yield();
	}

	if (prev == NULL && next == NULL)
	{
		return 0;
	}
	if (prev == NULL && next->prev == element)
	{
		next->prev = prev;
		return 0;
	}
	if (next == NULL && prev->next == element)
	{
		prev->next = next;
		return 0;
	}
	if (next->prev == element && prev->next == element)
	{
		prev->next = next;
		next->prev = prev;
		return 0;
	}
	return 1;
}

/**
 * SortedList_lookup ... search sorted list for a key
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
	if (list == NULL || key == NULL)
	{
		return NULL;
	}

	SortedListElement_t* curr = list;

	// Following is critical section because list could
	// be updated as we are checking for key
	if (opt_yield & LOOKUP_YIELD)
	{
		sched_yield();
	}

	while (curr)
	{
		if (curr->key && strcmp(curr->key, key) == 0)
		{
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 * @param SortedList_t *list ... header for the list
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list)
{
	// Check for empty list
	if (list == NULL)
	{
		return 0;
	}

	// Following is critical section because list could
	// be updated as we are checking its length
	if (opt_yield & LOOKUP_YIELD)
	{
		sched_yield();
	}

	if (list->next == NULL)
	{
		return 0;
	}

	SortedListElement_t* prev = list->next;
	SortedListElement_t* curr = prev->next;
	if (curr == NULL)
	{
		return 1;
	}

	int count = 1;
	while (curr)
	{
		// Validate pointers
		if ((curr->prev != prev && prev->next != curr) || curr->next == curr)
		{
			return -1;
		}

		prev = curr;
		curr = curr->next;
		count++;
	}
	return count;
}