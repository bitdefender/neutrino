#ifndef _MEMORY_LAYOUT_H_

//#include "VirtualMem.h"

#include <string>

#if defined _WIN32 || defined __CYGWIN__
#include <Windows.h>
#endif

namespace Neutrino {

#ifdef __linux__
	typedef int process_t;
#elif defined _WIN32 || defined __CYGWIN__
	typedef ::HANDLE process_t;
#endif

	class Module {
		std::string name;
		unsigned int base;
		unsigned int size;
	};

	class ModuleLayout {
	public:
		virtual bool Init() = 0;
		virtual bool Query(void *addr, const Module *module, unsigned int &offset) = 0;
		virtual bool Release() = 0;
	};

	ModuleLayout *CreateModuleLayout(process_t process);

};

#endif