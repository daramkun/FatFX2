#ifndef __FATFX_INTERNAL_H__
#define __FATFX_INTERNAL_H__

#include <stdlib.h>

typedef void ( *__NR ) ( void * data );

typedef struct TLN { void * data; struct TLN * next; } *	__LL_NODE;
typedef struct TLL { __LL_NODE f, e; unsigned c; __NR nr; }	__LINKEDLIST;

static __LINKEDLIST* createLinkedList ( __NR nr )
{
	__LINKEDLIST * ret = ( __LINKEDLIST* ) malloc ( sizeof ( struct TLL ) );
	ret->f = ret->e = NULL;
	ret->c = 0;
	ret->nr = nr;

	return ret;
}

static void destroyLinkedList ( __LINKEDLIST * ll )
{
	__LL_NODE temp, temp2;

	if ( ll == NULL ) return;

	temp = ll->f;
	while ( temp )
	{
		temp2 = temp->next;
		ll->nr ( temp->data );
		free ( temp );
		temp = temp2;
	}

	free ( ll );
}

static void addLinkedListNode ( __LINKEDLIST * ll, void * data )
{
	__LL_NODE node;

	if ( ll == NULL ) return;

	node = ( __LL_NODE ) malloc ( sizeof ( struct TLN ) );
	node->data = data;
	node->next = NULL;

	if ( ll->f == NULL )
	{
		ll->f = node;
	}
	else
	{
		ll->e->next = node;
	}
	ll->e = node;
	++ll->c;
}

static void removeLinkedListNode ( __LINKEDLIST * ll, unsigned int index )
{
	__LL_NODE temp, temp2;
	unsigned i;

	if ( ll == NULL ) return;
	if ( ll->c <= index ) return;

	temp = temp2 = ll->f;
	for ( i = 0; i < index; ++i )
	{
		temp2 = temp;
		temp = temp->next;
	}

	if ( temp == ll->f )
	{
		ll->f = temp->next;
	}
	else
	{
		temp2->next = temp->next;
	}

	if ( ll->e == temp )
	{
		if ( ll->f == ll->e )
		{
			ll->f = ll->e = NULL;
		}
		else ll->e = temp2;
	}

	ll->nr ( temp->data );
	free ( temp );

	--ll->c;
}

static void * getLinkedListData ( __LINKEDLIST * ll, unsigned int index )
{
	__LL_NODE temp;
	unsigned i;

	if ( ll == NULL ) return NULL;
	if ( ll->c <= index ) return NULL;

	temp = ll->f;
	for ( i = 0; i < index; ++i )
		temp = temp->next;

	return temp->data;
}

static unsigned getLinkedListSize ( __LINKEDLIST * ll ) { return ll ? ll->c : 0; }

#endif