#ifndef __DARAMEE_FATFX_H__
#define __DARAMEE_FATFX_H__

#include <stdio.h>
#include <string.h>

typedef const char *										FFXLANGUAGE;
#define FFXLANGUAGE_HLSL_STYLE_D3D9							"HLSL09\0\0"
#define FFXLANGUAGE_HLSL_STYLE_D3D1X						"HLSL1X\0\0"
#define FFXLANGUAGE_DXIL									"DXIL\0\0\0\0"
#define FFXLANGUAGE_GLSL_1_2								"GLSL12\0\0"
#define FFXLANGUAGE_GLSL_3_3								"GLSL33\0\0"
#define FFXLANGUAGE_GLSL_4_4								"GLSL44\0\0"
#define FFXLANGUAGE_GLSLES_1_0								"GLSLES10"
#define FFXLANGUAGE_GLSLES_3_0								"GLSLES30"
#define FFXLANGUAGE_GLSLES_3_1								"GLSLES31"
#define FFXLANGUAGE_METAL									"METAL\0\0\0"

typedef unsigned int										FFXSHADERTYPE;
#define FFXSHADERTYPE_VERTEXSHADER							( 1 << 0 )
#define FFXSHADERTYPE_PIXELSHADER							( 1 << 1 )
#define FFXSHADERTYPE_GEOMETRYSHADER						( 1 << 2 )
#define FFXSHADERTYPE_HULLSHADER							( 1 << 3 )
#define FFXSHADERTYPE_DOMAINSHADER							( 1 << 4 )
#define FFXSHADERTYPE_COMPUTESHADER							( 1 << 5 )

typedef unsigned int										FFXELEMENTTYPE;
#define FFXELEMENTTYPE_INT									1
#define FFXELEMENTTYPE_UINT									2
#define FFXELEMENTTYPE_FLOAT								3
#define FFXELEMENTTYPE_DOUBLE								4
#define FFXELEMENTTYPE_BOOL									5

#if defined ( __cplusplus )
extern "C"
{
#else
#	include <stdbool.h>
#endif

	typedef struct _FFXCONTAINER							FFXCONTAINER;
	typedef struct _FFXLANGUAGECHUNK						FFXLANGUAGECHUNK;
	typedef struct _FFXSHADER								FFXSHADER;

	typedef struct _FFXINPUTCHUNK							FFXINPUTCHUNK;

	typedef struct _FFXINPUTELEMENT
	{
		char semantic [ 32 ];
		unsigned int index;
		FFXELEMENTTYPE elementType;
		unsigned int count;
	}														FFXINPUTELEMENT;

	FFXCONTAINER* FFX_createContainer ();
	void FFX_destroyContainer ( FFXCONTAINER * container );

	unsigned int FFX_getContainerChunkCount ( FFXCONTAINER * container );

	bool FFX_isContainedInputChunk ( FFXCONTAINER * container );
	FFXINPUTCHUNK* FFX_addInputChunk ( FFXCONTAINER * container, FFXINPUTELEMENT * elements );
	void FFX_removeInputChunk ( FFXCONTAINER * container );
	FFXINPUTCHUNK* FFX_getInputChunk ( FFXCONTAINER * container );
	FFXINPUTELEMENT* FFX_getInputChunkData ( FFXINPUTCHUNK * chunk );

	bool FFX_isContainedLanguage ( FFXCONTAINER * container, FFXLANGUAGE language );
	FFXLANGUAGECHUNK* FFX_addLanguage ( FFXCONTAINER * container, FFXLANGUAGE language );
	void FFX_removeLanguage ( FFXCONTAINER * container, FFXLANGUAGE language );
	FFXLANGUAGECHUNK* FFX_getLanguage ( FFXCONTAINER * container, FFXLANGUAGE language );

	unsigned int FFX_getLanguageShaderCount ( FFXLANGUAGECHUNK * language );

	bool FFX_isContainedShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	FFXSHADER* FFX_addShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	void FFX_removeShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	FFXSHADER* FFX_getShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );

	void FFX_setShaderData ( FFXSHADER * shader, void * data, size_t dataSize );
	void FFX_getShaderData ( FFXSHADER * shader, void ** data, size_t * dataSize );

	FFXCONTAINER* FFX_loadContainer ( FILE * fp );
	void FFX_saveContainer ( FFXCONTAINER * container, FILE * fp );

#if defined ( __cplusplus )
}

#include <iostream>
#include <fstream>

	FFXCONTAINER* FFX_loadContainer ( std::istream & stream );
	//void FFX_saveContainer ( FFXCONTAINER * container, std::ostream & stream );
#endif

#endif