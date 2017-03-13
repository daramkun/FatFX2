using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;

namespace Daramee.FatFX
{
	public class Shader
	{
		public byte [] ShaderCode { get; set; }

		public override string ToString ()
		{
			return Encoding.ASCII.GetString ( ShaderCode );
		}

		public InOutElement [] ToInOutElements ()
		{
			IntPtr hglobal = Marshal.AllocHGlobal ( 44 );
			Marshal.Copy ( ShaderCode, 0, hglobal, ShaderCode.Length );
			InOutElement [] ies = new InOutElement [ ShaderCode.Length / 44 ];
			for ( int j = 0; j < ShaderCode.Length / 44; ++j )
			{
				ies [ j ] = Marshal.PtrToStructure<InOutElement> ( hglobal );
				hglobal += 44;
			}
			Marshal.FreeHGlobal ( hglobal );

			return ies;
		}

		public void SetInOutElements ( InOutElement [] elements )
		{
			using ( MemoryStream mems = new MemoryStream ( 44 * ( elements.Length + 1 ) ) )
			{
				foreach ( var element in elements )
				{
					var hglobal = Marshal.AllocHGlobal ( 44 );
					Marshal.StructureToPtr<InOutElement> ( element, hglobal, false );
					byte [] bytes = new byte [ 44 ];
					Marshal.Copy ( hglobal, bytes, 0, 44 );
					Marshal.FreeHGlobal ( hglobal );
					mems.Write ( bytes, 0, bytes.Length );
				}
				ShaderCode = mems.ToArray ();
			}
		}
	}
}
