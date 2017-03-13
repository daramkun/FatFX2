#include "FatFX.h"

#include <stdio.h>
#include <stdlib.h>

#include  "FatFX.Internal.h"

struct _FFXBLOB
{
	void * data;
	size_t dataSize;
};

FFXBLOB * FFX_createBlob ( void * data, size_t size )
{
	FFXBLOB * blob = ( FFXBLOB* ) malloc ( sizeof ( FFXBLOB ) );
	blob->data = malloc ( size );
	blob->dataSize = size;
	if ( data ) memcpy ( blob->data, data, size );
	return blob;
}

FFXBLOB * FFX_createBlobByInOutElement ( FFXINOUTELEMENT * elements )
{
	size_t count = 0;
	FFXINOUTELEMENT * temp = elements;
	while ( temp->semantic [ 0 ] != '\0' )
	{
		++count;
		++temp;
	}
	return FFX_createBlob ( elements, ( count + 1 ) * sizeof ( FFXINOUTELEMENT ) );
}

void FFX_destroyBlob ( FFXBLOB * blob )
{
	free ( blob->data );
	free ( blob );
}

void * FFX_getDataPointer ( FFXBLOB * blob ) { return blob->data; }
size_t FFX_getDataSize ( FFXBLOB * blob ) { return blob->dataSize; }

////////////////////////////////////////////////////////////////////////////////////////////////////

struct _FFXCONTAINER
{
	__LINKEDLIST * linkedList;
};

struct _FFXLANGUAGECHUNK
{
	FFXLANGUAGETYPE language;
	__LINKEDLIST * linkedList;
};

struct _FFXINOUT
{
	FFXINOUTELEMENT * elements;
	size_t elementCount;
};

struct _FFXSHADER
{
	FFXSHADERTYPE shaderType;
	FFXBLOB * blob;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

void ___containerDelete ( void * data )
{
	FFXLANGUAGECHUNK * chunk = ( FFXLANGUAGECHUNK* ) data;
	destroyLinkedList ( chunk->linkedList );
	free ( data );
}

FFXCONTAINER* FFX_createContainer ()
{
	FFXCONTAINER * ret = ( FFXCONTAINER* ) malloc ( sizeof ( FFXCONTAINER ) );
	ret->linkedList = createLinkedList ( ___containerDelete );
	return ret;
}

void FFX_destroyContainer ( FFXCONTAINER * container )
{
	destroyLinkedList ( container->linkedList );
	free ( container );
}

size_t FFX_getContainerChunkCount ( FFXCONTAINER * container ) { return getLinkedListSize ( container->linkedList ); }

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FFX_isContainedLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language )
{
	size_t size = getLinkedListSize ( container->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXLANGUAGECHUNK * chunk = ( FFXLANGUAGECHUNK* ) getLinkedListData ( container->linkedList, i );
		if ( strcmp ( chunk->language, language ) == 0 )
			return true;
	}
	return false;
}

void ___languageDelete ( void * data )
{
	FFXSHADER * shader = ( FFXSHADER* ) data;
	if ( shader->blob )
		FFX_destroyBlob ( shader->blob );
	free ( data );
}

FFXLANGUAGECHUNK* FFX_addLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language )
{
	FFXLANGUAGECHUNK * chunk;

	if ( FFX_isContainedLanguage ( container, language ) ) return NULL;

	chunk = ( FFXLANGUAGECHUNK* ) malloc ( sizeof ( FFXLANGUAGECHUNK ) );
	chunk->language = language;
	chunk->linkedList = createLinkedList ( ___languageDelete );

	addLinkedListNode ( container->linkedList, chunk );

	return chunk;
}

void FFX_removeLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language )
{
	size_t size = getLinkedListSize ( container->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXLANGUAGECHUNK * chunk = ( FFXLANGUAGECHUNK* ) getLinkedListData ( container->linkedList, i );
		if ( strcmp ( chunk->language, language ) == 0 )
		{
			removeLinkedListNode ( container->linkedList, i );
			return;
		}
	}
}

FFXLANGUAGECHUNK * FFX_getLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language )
{
	size_t size = getLinkedListSize ( container->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXLANGUAGECHUNK * chunk = ( FFXLANGUAGECHUNK* ) getLinkedListData ( container->linkedList, i );
		if ( strcmp ( chunk->language, language ) == 0 )
			return getLinkedListData(container->linkedList, i );
	}

	return NULL;
}

FFXLANGUAGECHUNK * FFX_getLanguageByIndex ( FFXCONTAINER * container, int index )
{
	return ( FFXLANGUAGECHUNK* ) getLinkedListData ( container->linkedList, index );
}

size_t FFX_getLanguageShaderCount ( FFXLANGUAGECHUNK * language )
{
	if ( language == NULL ) return 0;
	return getLinkedListSize ( language->linkedList );
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool FFX_isContainedShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	size_t size = getLinkedListSize ( chunk->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXSHADER * shader = ( FFXSHADER* ) getLinkedListData ( chunk->linkedList, i );
		if ( shader->shaderType == shaderType )
			return true;
	}
	return false;
}

FFXSHADER* FFX_addShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	FFXSHADER * shader;

	if ( FFX_isContainedShader ( chunk, shaderType ) ) return NULL;

	shader = ( FFXSHADER* ) malloc ( sizeof ( FFXSHADER ) );
	shader->blob = NULL;
	shader->shaderType = shaderType;

	addLinkedListNode ( chunk->linkedList, shader );

	return shader;
}

void FFX_removeShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	size_t size = getLinkedListSize ( chunk->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXSHADER * shader = ( FFXSHADER* ) getLinkedListData ( chunk->linkedList, i );
		if ( shader->shaderType == shaderType )
		{
			removeLinkedListNode ( chunk->linkedList, i );
			return;
		}
	}
}

FFXSHADER * FFX_getShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	size_t size = getLinkedListSize ( chunk->linkedList );
	for ( unsigned i = 0; i < size; ++i )
	{
		FFXSHADER * shader = ( FFXSHADER* ) getLinkedListData ( chunk->linkedList, i );
		if ( shader->shaderType == shaderType )
			return shader;
	}
	return NULL;
}

FFXSHADER * FFX_getShaderByIndex ( FFXLANGUAGECHUNK * chunk, int index )
{
	return ( FFXSHADER* ) getLinkedListData ( chunk->linkedList, index );
}

void FFX_setShaderData ( FFXSHADER * shader, FFXBLOB * blob )
{
	if ( shader == NULL ) return;
	if ( shader->blob != NULL ) FFX_destroyBlob ( shader->blob );
	shader->blob = blob;
}

FFXBLOB* FFX_getShaderData ( FFXSHADER * shader )
{
	if ( shader == NULL ) return NULL;
	return shader->blob;
}

FFXCONTAINER * FFX_loadContainer ( FILE * fp )
{
	char signatureBuffer [ 9 ] = { 0, };
	unsigned int i, j;
	unsigned int languageCount, shaderCount, shaderType, dataSize;
	void * data;

	FFXCONTAINER * container = FFX_createContainer ();
	FFXLANGUAGECHUNK * language;
	FFXSHADER * shader;

	fread ( signatureBuffer, 8, 1, fp );
	if ( strcmp ( "FatFX250", signatureBuffer ) == 0 )
	{
		fread ( &languageCount, 4, 1, fp );
		for ( i = 0; i < languageCount; ++i )
		{
			fread ( signatureBuffer, 8, 1, fp );

			language = FFX_addLanguage ( container, signatureBuffer );

			fread ( &shaderCount, 4, 1, fp );
			for ( j = 0; j < shaderCount; ++j )
			{
				fread ( &shaderType, 4, 1, fp );
				shader = FFX_addShader ( language, ( FFXSHADERTYPE ) shaderType );

				fread ( &dataSize, 4, 1, fp );
				data = malloc ( dataSize );
				fread ( data, dataSize, 1, fp );

				FFX_setShaderData ( shader, FFX_createBlob ( data, dataSize ) );

				free ( data );
			}
		}
	}
	else
	{
		FFX_destroyContainer ( container );
		return NULL;
	}

	return container;
}

void FFX_saveContainer ( FFXCONTAINER * container, FILE * fp )
{
	unsigned int chunkCount, elementCount = 0, shaderCount = 0;
	FFXLANGUAGECHUNK * language;
	FFXSHADER * shader;
	FFXBLOB * blob;
	unsigned int i, j;
	void * data;
	unsigned int dataSize;

	fwrite ( "FatFX250", 8, 1, fp );

	chunkCount = ( unsigned int ) FFX_getContainerChunkCount ( container );
	fwrite ( &chunkCount, 4, 1, fp );

	for ( i = 0; i < chunkCount; ++i )
	{
		language = FFX_getLanguageByIndex ( container, i );

		fwrite ( language->language, 8, 1, fp );
		shaderCount = ( unsigned int ) FFX_getLanguageShaderCount ( language );
		fwrite ( &shaderCount, 4, 1, fp );

		for ( j = 0; j < shaderCount; ++j )
		{
			shader = FFX_getShaderByIndex ( language, j );
			fwrite ( &shader->shaderType, 4, 1, fp );
			blob = FFX_getShaderData ( shader );
			
			dataSize = ( unsigned int ) FFX_getDataSize ( blob );
			fwrite ( &dataSize, 4, 1, fp );
			data = FFX_getDataPointer ( blob );
			fwrite ( data, dataSize, 1, fp );
		}
	}
}
