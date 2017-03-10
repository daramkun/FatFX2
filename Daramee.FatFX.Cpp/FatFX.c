#include "FatFX.h"

#include <stdio.h>
#include <stdlib.h>

#if !defined ( NULL )
#	define NULL												( void * ) 0
#endif

struct _FFXSHADER
{
	FFXSHADERTYPE shaderType;
	void * data;
	size_t dataSize;
};

struct __LANGUAGE_NODE
{
	FFXSHADER * shader;
	struct __LANGUAGE_NODE * next;
};

struct _FFXLANGUAGECHUNK
{
	unsigned char chunkType;
	struct __LANGUAGE_NODE * top;
	FFXLANGUAGE language;
};

struct _FFXINPUTCHUNK
{
	unsigned char chunkType;
	FFXINPUTELEMENT * elements;
};

struct __CONTAINER_NODE
{
	void * chunk;
	struct __CONTAINER_NODE * next;
};

struct _FFXCONTAINER
{
	struct __CONTAINER_NODE * top;
};

FFXCONTAINER* FFX_createContainer ()
{
	FFXCONTAINER * ret = ( FFXCONTAINER* ) malloc ( sizeof ( FFXCONTAINER ) );
	ret->top = NULL;

	return ret;
}

void FFX_destroyContainer ( FFXCONTAINER * container )
{
	struct __CONTAINER_NODE * chunks = container->top, * tempChunk;
	struct __LANGUAGE_NODE * shaders = NULL, * tempShader;

	if ( container == NULL ) return;

	while ( chunks != NULL )
	{
		switch ( ( ( unsigned char ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			{
				shaders = ( ( FFXLANGUAGECHUNK* ) chunks->chunk )->top;
				while ( shaders != NULL )
				{
					free ( shaders->shader->data );
					free ( shaders->shader );

					tempShader = shaders;
					shaders = shaders->next;
					free ( tempShader );
				}
			}
			break;

		case 1:		//< Input Elements Chunk
			{
				free ( ( ( FFXINPUTCHUNK* ) chunks->chunk )->elements );
			}
			break;
		}

		free ( chunks->chunk );

		tempChunk = chunks;
		chunks = chunks->next;
		free ( tempChunk );
	}

	free ( container );
}

unsigned int FFX_getContainerChunkCount ( FFXCONTAINER * container )
{
	struct __CONTAINER_NODE * chunks = container->top;
	unsigned int count = 0;

	while ( chunks != NULL )
	{
		++count;
		chunks = chunks->next;
	}

	return count;
}

bool FFX_isContainedInputChunk ( FFXCONTAINER * container )
{
	struct __CONTAINER_NODE * chunks = container->top;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			break;

		case 1:		//< Input Elements Chunk
			return true;
		}

		chunks = chunks->next;
	}

	return false;
}

FFXINPUTCHUNK* FFX_addInputChunk ( FFXCONTAINER * container, FFXINPUTELEMENT * elements )
{
	struct __CONTAINER_NODE * chunks = container->top, *cnode;
	FFXINPUTCHUNK * chunk;
	size_t count = 0;

	if ( FFX_isContainedInputChunk ( container ) ) return NULL;

	FFXINPUTELEMENT * temp = elements;
	while ( temp != NULL )
	{
		++count;
		++temp;
		if ( temp->semantic [ 0 ] == '\0' ) break;
	}

	chunk = ( FFXINPUTCHUNK* ) malloc ( sizeof ( FFXINPUTCHUNK ) );
	chunk->chunkType = 1;
	chunk->elements = ( FFXINPUTELEMENT* ) malloc ( sizeof ( FFXINPUTELEMENT ) * count );
	memcpy ( chunk->elements, elements, sizeof ( FFXINPUTELEMENT ) * count );

	cnode = ( struct __CONTAINER_NODE* ) malloc ( sizeof ( struct __CONTAINER_NODE ) );
	cnode->chunk = chunk;
	cnode->next = NULL;

	if ( chunks == NULL )
	{
		container->top = cnode;
	}
	else
	{
		while ( chunks->next != NULL )
			chunks = chunks->next;
		chunks->next = cnode;
	}

	return chunk;
}

void FFX_removeInputChunk ( FFXCONTAINER * container )
{
	struct __CONTAINER_NODE * chunks = container->top, *lastChunks = NULL;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			break;

		case 1:		//< Input Elements Chunk
			if ( lastChunks == NULL || lastChunks == container->top )
				container->top = container->top->next;
			else
			{
				lastChunks->next = chunks->next;
			}

			free ( ( ( FFXINPUTCHUNK* ) chunks->chunk )->elements );
			free ( chunks->chunk );
			free ( chunks );

			return;
		}

		lastChunks = chunks;
		chunks = chunks->next;
	}
}

FFXINPUTCHUNK * FFX_getInputChunk ( FFXCONTAINER * container )
{
	struct __CONTAINER_NODE * chunks = container->top;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			break;

		case 1:		//< Input Elements Chunk
			return ( FFXINPUTCHUNK* ) chunks->chunk;
		}

		chunks = chunks->next;
	}

	return NULL;
}

FFXINPUTELEMENT * FFX_getInputChunkData ( FFXINPUTCHUNK * chunk )
{
	return chunk->elements;
}

bool FFX_isContainedLanguage ( FFXCONTAINER * container, FFXLANGUAGE language )
{
	struct __CONTAINER_NODE * chunks = container->top;
	FFXLANGUAGECHUNK * chunk;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			chunk = ( FFXLANGUAGECHUNK* ) chunks->chunk;
			if ( strcmp ( chunk->language, language ) == 0 )
				return true;
			break;

		case 1:		//< Input Elements Chunk
			break;
		}

		chunks = chunks->next;
	}

	return false;
}

FFXLANGUAGECHUNK* FFX_addLanguage ( FFXCONTAINER * container, FFXLANGUAGE language )
{
	struct __CONTAINER_NODE * chunks = container->top, * cnode;
	FFXLANGUAGECHUNK * chunk;

	if ( FFX_isContainedLanguage ( container, language ) ) return NULL;

	chunk = ( FFXLANGUAGECHUNK* ) malloc ( sizeof ( FFXLANGUAGECHUNK ) );
	chunk->chunkType = 0;
	chunk->language = language;
	chunk->top = NULL;

	cnode = ( struct __CONTAINER_NODE* ) malloc ( sizeof ( struct __CONTAINER_NODE ) );
	cnode->chunk = chunk;
	cnode->next = NULL;

	if ( chunks == NULL )
	{
		container->top = cnode;
	}
	else
	{
		while ( chunks->next != NULL )
			chunks = chunks->next;
		chunks->next = cnode;
	}

	return chunk;
}

void FFX_removeLanguage ( FFXCONTAINER * container, FFXLANGUAGE language )
{
	struct __CONTAINER_NODE * chunks = container->top, * lastChunks = NULL;
	FFXLANGUAGECHUNK * chunk;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			chunk = ( FFXLANGUAGECHUNK* ) chunks->chunk;
			if ( strcmp ( chunk->language, language ) == 0 )
			{
				if ( lastChunks == NULL || lastChunks == container->top )
					container->top = container->top->next;
				else
				{
					lastChunks->next = chunks->next;
				}

				free ( chunk );
				free ( chunks );

				return;
			}
			break;

		case 1:		//< Input Elements Chunk
			break;
		}

		lastChunks = chunks;
		chunks = chunks->next;
	}
}

FFXLANGUAGECHUNK * FFX_getLanguage ( FFXCONTAINER * container, FFXLANGUAGE language )
{
	struct __CONTAINER_NODE * chunks = container->top;
	FFXLANGUAGECHUNK * chunk;

	while ( chunks != NULL )
	{
		switch ( *( ( unsigned char* ) chunks->chunk ) )
		{
		case 0:		//< Language Chunk
			chunk = ( FFXLANGUAGECHUNK* ) chunks->chunk;
			if ( strcmp ( chunk->language, language ) == 0 )
				return chunk;
			break;

		case 1:		//< Input Elements Chunk
			break;
		}

		chunks = chunks->next;
	}

	return NULL;
}

unsigned int FFX_getLanguageShaderCount ( FFXLANGUAGECHUNK * language )
{
	struct __LANGUAGE_NODE * shaders = language->top;
	unsigned int count = 0;

	while ( shaders != NULL )
	{
		++count;
		shaders = shaders->next;
	}

	return count;

	return 0;
}

bool FFX_isContainedShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	struct __LANGUAGE_NODE * shaders = chunk->top;

	while ( shaders != NULL )
	{
		if ( shaders->shader->shaderType == shaderType )
			return true;

		shaders = shaders->next;
	}

	return false;
}

FFXSHADER* FFX_addShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	struct __LANGUAGE_NODE * shaders = chunk->top, * cnode;
	FFXSHADER * shader;
	if ( FFX_isContainedShader ( chunk, shaderType ) ) return NULL;

	shader = ( FFXSHADER* ) malloc ( sizeof ( FFXSHADER ) );
	shader->data = NULL;
	shader->dataSize = 0;
	shader->shaderType = shaderType;

	cnode = ( struct __LANGUAGE_NODE* ) malloc ( sizeof ( struct __LANGUAGE_NODE ) );
	cnode->shader = shader;
	cnode->next = NULL;

	if ( shaders == NULL )
	{
		chunk->top = cnode;
	}
	else
	{
		while ( shaders->next != NULL )
			shaders = shaders->next;
		shaders->next = cnode;
	}

	return shader;
}

void FFX_removeShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	struct __LANGUAGE_NODE * shaders = chunk->top, *lastShaders = NULL;
	FFXSHADER * shader;

	while ( shaders != NULL )
	{
		shader = shaders->shader;
		if ( shader->shaderType == shaderType )
		{
			if ( lastShaders == NULL || lastShaders == chunk->top )
				chunk->top = chunk->top->next;
			else
			{
				lastShaders->next = shaders->next;
			}

			free ( shader );
			free ( shaders );

			return;
		}

		lastShaders = shaders;
		shaders = shaders->next;
	}
}

FFXSHADER * FFX_getShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	struct __LANGUAGE_NODE * shaders = chunk->top;

	while ( shaders != NULL )
	{
		if ( shaders->shader->shaderType == shaderType )
			return shaders->shader;

		shaders = shaders->next;
	}

	return NULL;
}

void FFX_setShaderData ( FFXSHADER * shader, void * data, size_t dataSize )
{
	if ( shader == NULL ) return;

	shader->data = malloc ( dataSize );
	shader->dataSize = dataSize;

	memcpy ( shader->data, data, dataSize );
}

void FFX_getShaderData ( FFXSHADER * shader, void ** data, size_t * dataSize )
{
	if ( shader == NULL ) return;

	*data = shader->data;
	*dataSize = shader->dataSize;
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
	if ( strcmp ( "FatFX200", signatureBuffer ) == 0 )
	{
		fread ( &languageCount, 4, 1, fp );
		for ( i = 0; i < languageCount; ++i )
		{
			fread ( signatureBuffer, 8, 1, fp );

			if ( strcmp ( signatureBuffer, "ISGN" ) == 0 )
			{
				fread ( &j, 4, 1, fp );
				FFXINPUTELEMENT * elements = ( FFXINPUTELEMENT* ) malloc ( j );
				fread ( elements, j, 1, fp );
				FFX_addInputChunk ( container, elements );
				free ( elements );
			}
			else
			{
				language = FFX_addLanguage ( container, signatureBuffer );

				fread ( &shaderCount, 4, 1, fp );
				for ( j = 0; j < shaderCount; ++j )
				{
					fread ( &shaderType, 4, 1, fp );
					shader = FFX_addShader ( language, ( FFXSHADERTYPE ) shaderType );

					fread ( &dataSize, 4, 1, fp );
					data = malloc ( dataSize );
					fread ( data, dataSize, 1, fp );
					FFX_setShaderData ( shader, data, dataSize );

					free ( data );
				}
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
	FFXINPUTELEMENT * elements;
	unsigned int chunkCount, elementCount = 0, shaderCount = 0;
	FFXLANGUAGE languages [] =
	{
		FFXLANGUAGE_HLSL_STYLE_D3D9,
		FFXLANGUAGE_HLSL_STYLE_D3D1X,
		FFXLANGUAGE_DXIL,
		FFXLANGUAGE_GLSL_1_2,
		FFXLANGUAGE_GLSL_3_3,
		FFXLANGUAGE_GLSL_4_4,
		FFXLANGUAGE_GLSLES_1_0,
		FFXLANGUAGE_GLSLES_3_0,
		FFXLANGUAGE_GLSLES_3_1,
		FFXLANGUAGE_METAL,
	};
	FFXLANGUAGECHUNK * language;
	FFXSHADERTYPE shaderTypes [] =
	{
		FFXSHADERTYPE_VERTEXSHADER,
		FFXSHADERTYPE_PIXELSHADER,
		FFXSHADERTYPE_GEOMETRYSHADER,
		FFXSHADERTYPE_HULLSHADER,
		FFXSHADERTYPE_DOMAINSHADER,
		FFXSHADERTYPE_COMPUTESHADER,
	};
	unsigned int i, j;
	void * data;
	size_t dataSize;

	fwrite ( "FatFX200", 8, 1, fp );

	chunkCount = FFX_getContainerChunkCount ( container );
	fwrite ( &chunkCount, 4, 1, fp );

	if ( FFX_isContainedInputChunk ( container ) )
	{
		fwrite ( "ISGN\0\0\0\0", 8, 1, fp );
		FFXINPUTELEMENT * temp = elements = FFX_getInputChunkData ( FFX_getInputChunk ( container ) );

		while ( temp != NULL )
		{
			++elementCount;
			++temp;
			if ( temp->semantic [ 0 ] == '\0' ) break;
		}
		elementCount *= sizeof ( FFXINPUTELEMENT );

		fwrite ( &elementCount, 4, 1, fp );
		fwrite ( elements, elementCount, 1, fp );
	}

	for ( i = 0; i < _countof ( languages ); ++i )
	{
		if ( FFX_isContainedLanguage ( container, languages [ i ] ) )
		{
			fwrite ( languages [ i ], 8, 1, fp );
			language = FFX_getLanguage ( container, languages [ i ] );
			shaderCount = FFX_getLanguageShaderCount ( language );
			fwrite ( &shaderCount, 4, 1, fp );

			for ( j = 0; j < _countof ( shaderTypes ); ++j )
			{
				fwrite ( &shaderTypes [ j ], 4, 1, fp );
				FFX_getShaderData ( FFX_getShader ( language, shaderTypes [ j ] ), &data, &dataSize );
				fwrite ( &dataSize, 4, 1, fp );
				fwrite ( data, dataSize, 1, fp );
			}
		}
	}
}
