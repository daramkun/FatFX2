#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>

#include <atlconv.h>

#include <hlslcc.h>
#include <d3dcompiler.h>
#pragma comment ( lib, "d3dcompiler.lib" )

#include <FatFX.h>

const char * getShaderTypeText ( FFXSHADERTYPE shaderType )
{
	switch ( shaderType )
	{
	case FFXSHADERTYPE_VERTEXSHADER:						return "Vertex Shader";
	case FFXSHADERTYPE_PIXELSHADER:							return "Pixel Shader";
	case FFXSHADERTYPE_GEOMETRYSHADER:						return "Geometry Shader";
	case FFXSHADERTYPE_DOMAINSHADER:						return "Domain Shader";
	case FFXSHADERTYPE_HULLSHADER:							return "Hull Shader";
	case FFXSHADERTYPE_COMPUTESHADER:						return "Compute Shader";
	default:												return "Unknown Shader";
	}
}

FFXLANGUAGETYPE getLanguage ( GLLang lang )
{
	switch ( lang )
	{
	case LANG_120:											return FFXLANGUAGETYPE_GLSL_1_2;
	case LANG_330:											return FFXLANGUAGETYPE_GLSL_3_3;
	case LANG_440:											return FFXLANGUAGETYPE_GLSL_4_4;
	case LANG_ES_100:										return FFXLANGUAGETYPE_GLSLES_1_0;
	case LANG_ES_300:										return FFXLANGUAGETYPE_GLSLES_3_0;
	case LANG_ES_310:										return FFXLANGUAGETYPE_GLSLES_3_1;
	case LANG_METAL:										return FFXLANGUAGETYPE_METAL;
	default:												return "\0\0\0\0\0\0\0\0";
	}
}

unsigned getBitcount ( unsigned bits )
{
	return ( bits & 1 ? 1 : 0 ) +
		( bits & 2 ? 1 : 0 ) +
		( bits & 4 ? 1 : 0 ) +
		( bits & 8 ? 1 : 0 );
}

void printUsage () { printf ( "Make FatFX file USAGE: FatFX.exe [+-][vpghdc] <filenames> <target>\n" ); }

void printErrorMessage ( ID3DBlob * errMsg, FFXSHADERTYPE shaderType )
{
	printf ( "ERROR: %s Syntax Error.\n%s\n", getShaderTypeText ( shaderType ), ( char * ) errMsg->GetBufferPointer () );
	errMsg->Release ();
}

ID3DBlob * getFileContent ( const char * filename )
{
	FILE * fp = fopen ( filename, "rt" );
	fseek ( fp, 0, SEEK_END );
	size_t size = ftell ( fp );
	fseek ( fp, 0, SEEK_SET );
	char buffer [ 65536 ];
	fread ( buffer, size, 1, fp );
	fclose ( fp );

	ID3DBlob * blob;
	if ( FAILED ( D3DCreateBlob ( size + 1, &blob ) ) ) return nullptr;
	memcpy ( blob->GetBufferPointer (), buffer, size );

	return blob;
}

FFXLANGUAGECHUNK* getLanguageChunk ( FFXCONTAINER * container, FFXLANGUAGETYPE language )
{
	return FFX_isContainedLanguage ( container, language ) ?
		FFX_getLanguage ( container, language ) :
		FFX_addLanguage ( container, language );
}

FFXSHADER* getShaderBlock ( FFXLANGUAGECHUNK * chunk, FFXSHADERTYPE shaderType )
{
	return FFX_isContainedShader ( chunk, shaderType ) ?
		FFX_getShader ( chunk, shaderType ) :
		FFX_addShader ( chunk, shaderType );
}

void addShaderData ( FFXCONTAINER * container, FFXLANGUAGETYPE language, FFXSHADERTYPE shaderType, void * data, size_t dataSize )
{
	FFXLANGUAGECHUNK * languageChunk = getLanguageChunk(container, language );
	FFXSHADER * shader = getShaderBlock ( languageChunk, shaderType );
	FFX_setShaderData ( shader, FFX_createBlob ( data, dataSize ) );
}

void addShaderData ( FFXCONTAINER * container, FFXLANGUAGETYPE language, FFXSHADERTYPE shaderType, ID3DBlob * blob )
{
	addShaderData ( container, language, shaderType, blob->GetBufferPointer (), blob->GetBufferSize () );
}

FFXINOUTELEMENT* getElements ( GLSLShader & result, bool isInputSignature )
{
	ShaderInfo shaderInfo = result.reflection;
	auto is = isInputSignature ? shaderInfo.psInputSignatures : shaderInfo.psOutputSignatures;
	FFXINOUTELEMENT * elements = new FFXINOUTELEMENT [ is.size () + 1 ];
	for ( unsigned i = 0; i < is.size (); ++i )
	{
		auto p = is [ i ];
		memset ( elements [ i ].semantic, 0, 32 );
		strcpy ( elements [ i ].semantic, p.semanticName.c_str () );
		elements [ i ].index = p.ui32SemanticIndex;
		elements [ i ].elementType = p.eComponentType == 1 ? FFXELEMENTTYPE_UINT : ( p.eComponentType == 2 ? FFXELEMENTTYPE_INT : FFXELEMENTTYPE_FLOAT );
		elements [ i ].count = getBitcount ( p.ui32Mask );
	}
	memset ( elements [ is.size () ].semantic, 0, 32 );
	elements [ is.size () ].count = 0;
	elements [ is.size () ].index = 0;
	elements [ is.size () ].elementType = 0;

	return elements;
}

void translateTo ( FFXCONTAINER * container, FFXSHADERTYPE shaderType, void * data, size_t dataSize, bool optimizationOption )
{
	addShaderData ( container, FFXLANGUAGETYPE_DXIL_STYLE_D3D1X, shaderType, data, dataSize );

	GLSLShader result;
	HLSLccSamplerPrecisionInfo samplerPrecisions;
	HLSLccReflection reflectionCallbacks;

	GLLang langs [] = { LANG_120, LANG_330, LANG_440, LANG_ES_100, LANG_ES_300, LANG_ES_310, LANG_METAL };

	unsigned int flags = HLSLCC_FLAG_GS_ENABLED | HLSLCC_FLAG_TESS_ENABLED | HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT |
		HLSLCC_FLAG_INOUT_SEMANTIC_NAMES | HLSLCC_FLAG_VULKAN_BINDINGS;

	for ( GLLang lang : langs )
	{
		if ( TranslateHLSLFromMem ( ( char * ) data, flags, lang, nullptr, nullptr, samplerPrecisions, reflectionCallbacks, &result ) == 0 )
		{
			if ( TranslateHLSLFromMem ( ( char * ) data, flags & HLSLCC_FLAG_UNIFORM_BUFFER_OBJECT,
				lang, nullptr, nullptr, samplerPrecisions, reflectionCallbacks, &result ) == 0 )
			{
				printf ( "ERROR: Cannot Translate HLSL to %s (%s).\n", getLanguage ( lang ), getShaderTypeText ( shaderType ) );
				continue;
			}
		}
		addShaderData ( container, getLanguage ( lang ), shaderType, ( void * ) result.sourceCode.c_str (), result.sourceCode.length () );

		FFXLANGUAGECHUNK * inLang = getLanguageChunk ( container, FFXLANGUAGETYPE_NOLANGUAGE_INPUT );
		if ( !FFX_getShader ( inLang, shaderType ) )
		{
			FFXSHADER * inShader = FFX_addShader ( inLang, shaderType );
			FFXINOUTELEMENT * elements = getElements ( result, true );
			FFX_setShaderData ( inShader, FFX_createBlobByInOutElement ( elements ) );
			delete [] elements;
		}

		FFXLANGUAGECHUNK * outLang = getLanguageChunk ( container, FFXLANGUAGETYPE_NOLANGUAGE_OUTPUT );
		if ( !FFX_getShader ( outLang, shaderType ) )
		{
			FFXSHADER * outShader = FFX_addShader ( outLang, shaderType );
			FFXINOUTELEMENT * elements = getElements ( result, false );
			FFX_setShaderData ( outShader, FFX_createBlobByInOutElement ( elements ) );
			delete [] elements;
		}
	}
}

int main ( int argc, char * argv [] )
{
	printf ( "=========== FatFX v2.50 ===========\n" );
	printf ( "* Created by Daramee<daramkun@live.com>\n" );
	printf ( "* Thanks to Unity-Technologies/HLSLcc  d8176fd55ad66f82ece978039135d77965ae544e\n" );

	if ( argc <= 3 )
	{
		printUsage ();
		return -1;
	}

	char * flag = argv [ 1 ];
	size_t flagLength = strlen ( flag );
	if ( argc <= 2 + flagLength - 1 )
	{
		printUsage ();
		return -2;
	}

	bool optimizationOption = false;
	DWORD compileFlag = D3DCOMPILE_PACK_MATRIX_ROW_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS;
	if ( flag [ 0 ] == '+' )
		compileFlag |= ( D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION );
	else if ( flag [ 0 ] == '-' )
	{
		compileFlag |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		optimizationOption = true;
	}
	else
	{
		printUsage ();
		return -3;
	}

	FFXCONTAINER * container = FFX_createContainer ();

	for ( unsigned i = 1; i < flagLength; ++i )
	{
		USES_CONVERSION;

		ID3DBlob * code;
		ID3DBlob * errMsg;
		switch ( flag [ i ] )
		{
		case 'v':
			{
				D3D_SHADER_MACRO macro [] = { { "VERTEX_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "vs_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_VERTEXSHADER );
					break;
				}
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_VERTEXSHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_VERTEXSHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;

		case 'p':
			{
				D3D_SHADER_MACRO macro [] = { { "PIXEL_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "ps_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_PIXELSHADER );
					break;
				}
				
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_PIXELSHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_PIXELSHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;

		case 'g':
			{
				D3D_SHADER_MACRO macro [] = { { "GEOMETRY_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "gs_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_GEOMETRYSHADER );
					break;
				}
				
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_GEOMETRYSHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_GEOMETRYSHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;

		case 'h':
			{
				D3D_SHADER_MACRO macro [] = { { "HULL_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "hs_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_HULLSHADER );
					break;
				}
				
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_HULLSHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_HULLSHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;

		case 'd':
			{
				D3D_SHADER_MACRO macro [] = { { "DOMAIN_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "ds_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_DOMAINSHADER );
					break;
				}
				
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_DOMAINSHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_DOMAINSHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;

		case 'c':
			{
				D3D_SHADER_MACRO macro [] = { { "COMPUTE_SHADER", "" }, { nullptr, nullptr } };
				if ( FAILED ( D3DCompileFromFile ( A2W ( argv [ 1 + i ] ), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE,
					"main", "cs_5_0", 0, 0, &code, &errMsg ) ) )
				{
					printErrorMessage ( errMsg, FFXSHADERTYPE_COMPUTESHADER );
					break;
				}
				
				addShaderData ( container, FFXLANGUAGETYPE_HLSL_STYLE_D3D1X, FFXSHADERTYPE_COMPUTESHADER, getFileContent ( argv [ 1 + i ] ) );
				translateTo ( container, FFXSHADERTYPE_COMPUTESHADER, code->GetBufferPointer (), code->GetBufferSize (), optimizationOption );
			}
			break;
		}
	}

	FILE * fp = fopen ( argv [ 1 + flagLength ], "wb" );
	FFX_saveContainer ( container, fp );
	fclose ( fp );

	FFX_destroyContainer ( container );

	return 0;
}