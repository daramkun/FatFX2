using System;
using System.Runtime.InteropServices;

namespace Daramee.FatFX
{
	[StructLayout ( LayoutKind.Sequential, CharSet = CharSet.Ansi )]
	public struct InputElement
	{
		[MarshalAs ( UnmanagedType.ByValTStr, SizeConst = 32 )]
		public string Semantic;
		public uint Index;
		public ElementType ElementType;
		public uint ElementCount;
	}
}
