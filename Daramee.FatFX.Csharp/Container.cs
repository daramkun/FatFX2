using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Daramee.FatFX
{
    public class Container
    {
		public Dictionary<string, LanguageChunk> Chunks { get; private set; }

		public Container () { Chunks = new Dictionary<string, LanguageChunk> (); }
		public Container ( Stream stream ) { if ( !Load ( stream ) ) throw new InvalidDataException (); }

		public LanguageChunk GetLanguageChunk ( string language )
		{
			if ( !Chunks.ContainsKey ( language ) ) return null;
			return Chunks [ language ] as LanguageChunk;
		}

		public bool Load ( Stream stream )
		{
			using ( BinaryReader reader = new BinaryReader ( stream, System.Text.Encoding.ASCII, true ) )
			{
				if ( Encoding.ASCII.GetString ( reader.ReadBytes ( 8 ), 0, 8 ) != "FatFX250" )
					return false;

				uint chunkCount = reader.ReadUInt32 ();
				for ( uint i = 0; i < chunkCount; ++i )
				{
					string chunkName = Encoding.ASCII.GetString ( reader.ReadBytes ( 8 ), 0, 8 ).Trim ( ' ', '\0', '\t' );

					LanguageChunk languageChunk = new LanguageChunk () { Language = chunkName.Trim () };

					uint shaderCount = reader.ReadUInt32 ();
					for ( uint j = 0; j < shaderCount; ++j )
					{
						ShaderType shaderType = ( ShaderType ) reader.ReadUInt32 ();
						uint dataSize = reader.ReadUInt32 ();
						byte [] data = reader.ReadBytes ( ( int ) dataSize );
						languageChunk.Shaders.Add ( shaderType, new Shader () { ShaderCode = data } );
					}

					Chunks.Add ( languageChunk.Language, languageChunk );
				}
			}

			return true;
		}

		public void Save ( Stream stream )
		{
			using ( BinaryWriter writer = new BinaryWriter ( stream, System.Text.Encoding.ASCII, true ) )
			{
				byte [] signatureBytes = Encoding.ASCII.GetBytes ( "FatFX250".ToCharArray (), 0, 8 );
				writer.Write ( signatureBytes );

				writer.Write ( ( uint ) Chunks.Count );
				foreach ( var pair in Chunks )
				{
					if ( pair.Value is LanguageChunk )
					{
						LanguageChunk chunk = pair.Value as LanguageChunk;
						writer.Write ( Encoding.ASCII.GetBytes ( chunk.Language.PadRight ( 8, '\0' ).ToCharArray (), 0, 8 ) );
						writer.Write ( ( uint ) chunk.Shaders.Count );
						foreach ( var pair2 in chunk.Shaders )
						{
							writer.Write ( ( uint ) pair2.Key );
							writer.Write ( ( uint ) pair2.Value.ShaderCode.Length );
							writer.Write ( pair2.Value.ShaderCode );
						}
					}
				}
			}
		}
    }
}
