using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Daramee.FatFX
{
    public class Container
    {
		public Dictionary<string, IChunk> Chunks { get; private set; }

		public Container () { Chunks = new Dictionary<string, IChunk> (); }
		public Container ( Stream stream ) { if ( !Load ( stream ) ) throw new InvalidDataException (); }

		public LanguageChunk GetLanguageChunk ( string language )
		{
			if ( !Chunks.ContainsKey ( language ) ) return null;
			return Chunks [ language ] as LanguageChunk;
		}

		public InputChunk GetInputChunk ()
		{
			if ( !Chunks.ContainsKey ( "ISGN" ) ) return null;
			return Chunks [ "ISGN" ] as InputChunk;
		}

		public void AddInputChunk ( InputElement [] elements )
		{
			if ( Chunks.ContainsKey ( "ISGN" ) ) return;
			Chunks.Add ( "ISGN", new InputChunk () { Elements = elements } );
		}

		public bool Load ( Stream stream )
		{
			using ( BinaryReader reader = new BinaryReader ( stream, System.Text.Encoding.ASCII, true ) )
			{
				if ( Encoding.ASCII.GetString ( reader.ReadBytes ( 8 ), 0, 8 ) != "FatFX200" )
					return false;

				uint chunkCount = reader.ReadUInt32 ();
				for ( uint i = 0; i < chunkCount; ++i )
				{
					string chunkName = Encoding.ASCII.GetString ( reader.ReadBytes ( 8 ), 0, 8 ).Trim ( ' ', '\0', '\t' );
					if ( chunkName == "ISGN" )
					{
						uint elementSize = reader.ReadUInt32 ();
						byte [] elements = reader.ReadBytes ( ( int ) elementSize );
						IntPtr hglobal = Marshal.AllocHGlobal ( 44 );
						Marshal.Copy ( elements, 0, hglobal, ( int ) elementSize );
						InputElement [] ies = new InputElement [ elementSize / 44 ];
						for ( int j = 0; j < elementSize / 44; ++j )
						{
							ies [ j ] = Marshal.PtrToStructure<InputElement> ( hglobal );
							hglobal += 44;
						}
						Marshal.FreeHGlobal ( hglobal );

						AddInputChunk ( ies );
					}
					else
					{
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
			}

			return true;
		}

		public void Save ( Stream stream )
		{
			using ( BinaryWriter writer = new BinaryWriter ( stream, System.Text.Encoding.ASCII, true ) )
			{
				byte [] signatureBytes = Encoding.ASCII.GetBytes ( "FatFX200".ToCharArray (), 0, 8 );
				writer.Write ( signatureBytes );

				writer.Write ( ( uint ) Chunks.Count );
				if ( Chunks.ContainsKey ( "ISGN" ) )
				{
					InputChunk inputChunk = Chunks [ "ISGN" ] as InputChunk;
					writer.Write ( Encoding.ASCII.GetBytes ( "ISGN\0\0\0\0".ToCharArray (), 0, 8 ) );
					writer.Write ( inputChunk.Elements.Length * 44 );
					foreach ( InputElement element in inputChunk.Elements )
					{
						var hglobal =  Marshal.AllocHGlobal ( 44 );
						Marshal.StructureToPtr<InputElement> ( element, hglobal, false );
						byte [] bytes = new byte [ 44 ];
						Marshal.Copy ( hglobal, bytes, 0, 44 );
						Marshal.FreeHGlobal ( hglobal );
						writer.Write ( bytes );
					}
				}
				else
				{
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
}
