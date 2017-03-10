using System;
using System.Collections.Generic;
using System.Text;

namespace Daramee.FatFX
{
	[Flags]
    public enum ShaderType : uint
    {
		VertexShader = ( 1 << 0 ),
		PixelShader = ( 1 << 1 ),
		GeometryShader = ( 1 << 2 ),
		HullShader = ( 1 << 3 ),
		DomainShader = ( 1 << 4 ),
		ComputeShader = ( 1 << 5 ),
    }
}
