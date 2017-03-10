#include "FatFX.h"

FFXCONTAINER * FFX_loadContainer ( std::istream & stream )
{
	FFXCONTAINER * container = FFX_createContainer ();

	char signatureBuffer [ 9 ] = { 0, };
	stream.read ( signatureBuffer, 8 );
	if ( strcmp ( "FatFX200", signatureBuffer ) == 0 )
	{
		unsigned int chunkCount;
		stream.read ( ( char* ) &chunkCount, 4 );

		for ( unsigned int i = 0; i < chunkCount; ++i )
		{
			stream.read ( signatureBuffer, 8 );
			if ( strcmp ( "ISGN", signatureBuffer ) == 0 )
			{
				unsigned int elementSize;
				stream.read ( ( char* ) &elementSize, 4 );
				FFXINPUTELEMENT * elements = ( FFXINPUTELEMENT* ) malloc ( elementSize );
				stream.read ( ( char* ) elements, elementSize );
				FFX_addInputChunk ( container, elements );
				free ( elements );
			}
			else
			{
				FFXLANGUAGECHUNK * language = FFX_addLanguage ( container, signatureBuffer );
				unsigned int shaderCount;
				stream.read ( ( char* ) &shaderCount, 4 );

				for ( unsigned int j = 0; j < shaderCount; ++j )
				{
					FFXSHADERTYPE shaderType;
					stream.read ( ( char* ) &shaderType, 4 );
					FFXSHADER * shader = FFX_addShader ( language, shaderType );

					size_t dataSize;
					stream.read ( ( char* ) &dataSize, 4 );

					void * data = malloc ( dataSize );
					stream.read ( ( char* ) data, dataSize );

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

/*void FFX_saveContainer ( FFXCONTAINER * container, std::ostream & stream )
{

}*/
