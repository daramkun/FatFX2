#ifndef __DARAMEE_FATFX_H__
#define __DARAMEE_FATFX_H__

#include <stdio.h>
#include <string.h>

typedef const char *										FFXLANGUAGETYPE;
#define FFXLANGUAGETYPE_NOLANGUAGE_INPUT					"ISGN\0\0\0\0"
#define FFXLANGUAGETYPE_NOLANGUAGE_OUTPUT					"OSGN\0\0\0\0"
#define FFXLANGUAGETYPE_HLSL_STYLE_D3D9						"HLSL09\0\0"
#define FFXLANGUAGETYPE_HLSL_STYLE_D3D1X					"HLSL1X\0\0"
#define FFXLANGUAGETYPE_DXIL_STYLE_D3D9						"DXIL09\0\0"
#define FFXLANGUAGETYPE_DXIL_STYLE_D3D1X					"DXIL1X\0\0"
#define FFXLANGUAGETYPE_GLSL_1_2							"GLSL12\0\0"
#define FFXLANGUAGETYPE_GLSL_3_3							"GLSL33\0\0"
#define FFXLANGUAGETYPE_GLSL_4_4							"GLSL44\0\0"
#define FFXLANGUAGETYPE_GLSLES_1_0							"GLSLES10"
#define FFXLANGUAGETYPE_GLSLES_3_0							"GLSLES30"
#define FFXLANGUAGETYPE_GLSLES_3_1							"GLSLES31"
#define FFXLANGUAGETYPE_METAL								"METAL\0\0\0"

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
	typedef struct _FFXINOUTELEMENT
	{
		char semantic [ 32 ];
		unsigned int index;
		FFXELEMENTTYPE elementType;
		unsigned int count;
	}														FFXINOUTELEMENT;

	typedef struct _FFXBLOB									FFXBLOB;
	
	FFXBLOB* FFX_createBlob ( void * data, size_t size );
	FFXBLOB* FFX_createBlobByInOutElement ( FFXINOUTELEMENT * elements );
	void FFX_destroyBlob ( FFXBLOB * blob );
	void* FFX_getDataPointer ( FFXBLOB * blob );
	size_t FFX_getDataSize ( FFXBLOB * blob );

	////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct _FFXCONTAINER							FFXCONTAINER;

	FFXCONTAINER* FFX_createContainer ();
	void FFX_destroyContainer ( FFXCONTAINER * container );

	size_t FFX_getContainerChunkCount ( FFXCONTAINER * container );

	////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct _FFXLANGUAGECHUNK						FFXLANGUAGECHUNK;

	bool FFX_isContainedLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language );
	FFXLANGUAGECHUNK* FFX_addLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language );
	void FFX_removeLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language );
	FFXLANGUAGECHUNK* FFX_getLanguage ( FFXCONTAINER * container, FFXLANGUAGETYPE language );
	FFXLANGUAGECHUNK* FFX_getLanguageByIndex ( FFXCONTAINER * container, int index );

	size_t FFX_getLanguageShaderCount ( FFXLANGUAGECHUNK * language );

	////////////////////////////////////////////////////////////////////////////////////////////////

	typedef struct _FFXSHADER								FFXSHADER;
	
	bool FFX_isContainedShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	FFXSHADER* FFX_addShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	void FFX_removeShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	FFXSHADER* FFX_getShader ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType );
	FFXSHADER* FFX_getShaderByIndex ( FFXLANGUAGECHUNK * chunk, int index );

	void FFX_setShaderData ( FFXSHADER * shader, FFXBLOB * blob );
	FFXBLOB* FFX_getShaderData ( FFXSHADER * shader );

	////////////////////////////////////////////////////////////////////////////////////////////////

	FFXCONTAINER* FFX_loadContainer ( FILE * fp );
	void FFX_saveContainer ( FFXCONTAINER * container, FILE * fp );

#if defined ( __cplusplus )
}

#include <iostream>

	FFXCONTAINER* FFX_loadContainer ( std::istream & stream );
	//void FFX_saveContainer ( FFXCONTAINER * container, std::ostream & stream );
#endif

#endif