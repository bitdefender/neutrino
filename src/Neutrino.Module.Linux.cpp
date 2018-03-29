#include "Neutrino.Module.h"

namespace Neutrino {
	module_t OpenModule(const char *libName) {
        return dlopen(libName, RTLD_LAZY);
	}

	void CloseModule(module_t module) {
		dlclose(module);
	}

	void *FindFunction(module_t lib, const char *funcName) {
		return dlsym(lib, funcName);
	}

};