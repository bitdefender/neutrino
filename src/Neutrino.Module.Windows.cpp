#include "Neutrino.Module.h"

namespace Neutrino {
	module_t OpenModule(const char *libName) {
		HMODULE hModule;
		UINT oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
		SetErrorMode(oldErrorMode | SEM_FAILCRITICALERRORS);

		hModule = LoadLibrary(libName);

		SetErrorMode(oldErrorMode);

		return hModule;
	}

	void CloseModule(module_t module) {
		FreeLibrary(module);
	}

	void *FindFunction(module_t lib, const char *funcName) {
		return GetProcAddress(lib, funcName);
	}

};