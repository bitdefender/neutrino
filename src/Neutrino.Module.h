#ifndef _NEUTRINO_MODULE_H_
#define _NEUTRINO_MODULE_H_

#ifdef __linux__
#include <dlfcn.h>
#include <link.h>
#elif _WIN32
#include <Windows.h>
#endif

namespace Neutrino {

#ifdef __linux__
	typedef void* module_t;
#elif _WIN32
	typedef HMODULE module_t;
#endif

	module_t OpenModule(const char *libName);
	void CloseModule(module_t module);
	void *FindFunction(module_t lib, const char *funcName);

};

#endif
