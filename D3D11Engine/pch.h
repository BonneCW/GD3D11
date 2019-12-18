#pragma once

#define WINVER 0x0601 // Windows 7
#define _WIN32_WINNT 0x0601 // Windows 7
#define NTDDI_VERSION 0x06010000 // Windows 7

#pragma warning(disable: 4731) // Change of ebp from inline assembly
#pragma warning(disable: 4244) // Loss of data during conversion
#include <Windows.h>
#include <d3d11.h>
#include <SimpleMath.h>
#include <future>
#include <list>
#include <map>
#include <mmsystem.h>
#include <set>
#include <signal.h>
#include <unordered_map>
#include <unordered_set>

#include "Logger.h"
#include "Types.h"
#include "VertexTypes.h"

#if _MSC_VER < 1900
;
#else
#define stdext std
#endif

#define VERSION_NUMBER "17.7-dev2"
__declspec(selectany) const char* VERSION_NUMBER_STR = VERSION_NUMBER;

extern bool GMPModeActive;

/** D3D7-Call logging */
#define DebugWriteValue(value, check) if (value == check) { LogInfo() << " - " << #check; }
#define DebugWriteFlag(value, check) if ((value & check) == check) { LogInfo() << " - " << #check; }
#define DebugWrite(debugMessage) DebugWrite_i(debugMessage, (void *) this);

/** Debugging */
#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }
#define SAFE_DELETE(x) delete x; x = nullptr;
//#define V(x) x

/** Writes a string of the D3D7-Call log */
void DebugWrite_i(LPCSTR lpDebugMessage, void * thisptr);

/** Computes the size in bytes of the given FVF */
int ComputeFVFSize(DWORD fvf);

