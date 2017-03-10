using System;
using System.Collections.Generic;
using System.Text;

namespace Daramee.FatFX
{
	public class LanguageChunk : IChunk
	{
		public string Language { get; set; }
		public Dictionary<ShaderType, Shader> Shaders { get; private set; }

		public LanguageChunk ()
		{
			Shaders = new Dictionary<ShaderType, Shader> ();
		}
	}
}
