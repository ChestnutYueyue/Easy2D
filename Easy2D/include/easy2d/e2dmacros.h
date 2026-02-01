#pragma once

#ifndef WINVER
#	define WINVER 0x0A00       // Windows 10 or later
#endif

#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0A00 // Windows 10 or later
#endif

#ifndef NTDDI_VERSION
#	define NTDDI_VERSION NTDDI_WIN10
#endif

#ifndef UNICODE
#	define UNICODE
#endif

// Exclude rarely-used items from Windows headers
#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif

// Windows Header Files - 仅保留必要的头文件
#include <windows.h>

// 保留Direct2D/DWrite头文件兼容性（用于遗留代码）
// 这些将在未来版本中完全移除
#ifndef E2D_NO_DIRECT2D
#	include <d2d1.h>
#	include <dwrite.h>
#	include <wincodec.h>
#endif

#ifndef HINST_THISCOMPONENT
	EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#	define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

#if !defined(E2D_DEBUG) && defined(_DEBUG)
#	define E2D_DEBUG
#endif
