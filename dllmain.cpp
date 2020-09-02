#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files

#ifdef _WIN32
	#include <windows.h>
#endif

#include "imgui/imgui.h"
#include "GodotShaders.h"

#ifdef WIN32
# define FEXPORT __declspec(dllexport)
#else
# define FEXPORT // empty
#endif

extern "C" {
	FEXPORT gd::GodotShaders* CreatePlugin() {
		return new gd::GodotShaders();
	}
	FEXPORT void DestroyPlugin(gd::GodotShaders* ptr) {
		delete ptr;
	}
	FEXPORT int GetPluginAPIVersion() {
		return 3;
	}
	FEXPORT int GetPluginVersion() {
		return 2;
	}
	FEXPORT const char* GetPluginName() {
		return "GodotShaders";
	}
}

#ifdef _WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif