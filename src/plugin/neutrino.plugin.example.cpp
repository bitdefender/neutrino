#include "../Neutrino.Plugin.h"

extern "C" {
	__declspec(dllexport) Neutrino::PluginInfo NeutrinoModuleInfo = {
		{0, 0, 1},
		Neutrino::PluginType::UNKNOWN,
		"Example plugin",
		"Neutrino plugin example."
	};
}




#ifdef _MSC_VER
#include <Windows.h>
BOOL WINAPI DllMain(
	_In_ HINSTANCE hinstDLL,
	_In_ DWORD     fdwReason,
	_In_ LPVOID    lpvReserved
) {
	return TRUE;
}
#endif




