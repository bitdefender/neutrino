#ifndef _NEUTRINO_MODULE_H_
#define _NEUTRINO_MODULE_H_

namespace Neutrino {

#ifdef __linux__
	#include <dlfcn.h>
	#include <link.h>
	typedef void* module_t;

#elif _WIN32
	#include <Windows.h>

	typedef HMODULE module_t;
#endif

	module_t OpenModule(const char *libName);
	void CloseModule(module_t module);
	void *FindFunction(module_t lib, const char *funcName);

};

#endif
