#include "FatFX.h"

FFXCONTAINER * FFX_loadContainer ( std::istream & stream )
{
	FFXCONTAINER * container = FFX_createContainer ();

	char signatureBuffer [ 9 ] = { 0, };
	stream.read ( signatureBuffer, 8 );
	if ( strcmp ( "FatFX250", signatureBuffer ) == 0 )
	{
		unsigned int chunkCount;
		stream.read ( ( char* ) &chunkCount, 4 );

		for ( unsigned int i = 0; i < chunkCount; ++i )
		{
			stream.read ( signatureBuffer, 8 );

			FFXLANGUAGECHUNK * language = FFX_addLanguage ( container, signatureBuffer );
			unsigned int shaderCount;
			stream.read ( ( char* ) &shaderCount, 4 );

			for ( unsigned int j = 0; j < shaderCount; ++j )
			{
				FFXSHADERTYPE shaderType;
				stream.read ( ( char* ) &shaderType, 4 );
				FFXSHADER * shader = FFX_addShader ( language, ( FFXSHADERTYPE ) shaderType );

				unsigned int dataSize;
				stream.read ( ( char* ) &dataSize, 4 );
				char * data = new char [ dataSize ];
				stream.read ( data, dataSize );
				FFX_setShaderData ( shader, FFX_createBlob ( data, dataSize ) );

				delete [] data;
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
