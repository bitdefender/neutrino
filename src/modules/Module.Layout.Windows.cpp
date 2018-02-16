#if defined _WIN32 || defined __CYGWIN__


#include <Windows.h>
#include <Psapi.h>

#include "Module.Layout.h"

#include <vector>

namespace Neutrino {

	class WinModuleLayout : public ModuleLayout {
	private :
		process_t process;
		char moduleName[MAX_PATH];

		std::vector<Module> modules;
	public :
		WinModuleLayout(process_t p) {
			process = p;
		}

		virtual bool Init() {
			return true;
		}

		virtual bool Query(void *addr, const Module *module, unsigned int &offset) {
			/*MEMORY_BASIC_INFORMATION32 mbi;

			for (unsigned int i = 0x10000; i < 0x7FFF0000; ) {

			}

			if (0 == VirtualQueryEx(process, addr, (PMEMORY_BASIC_INFORMATION)&mbi, sizeof(mbi))) {
				return false;
			}

			out.baseAddress = (void *)mbi.BaseAddress;
			out.allocationBase = (void *)mbi.AllocationBase;
			out.size = mbi.RegionSize;
			out.state = MemoryState(mbi.State);
			out.protection = MemoryProtect(mbi.Protect);
			out.moduleName = moduleName;

			moduleName[0] = '\0';
			if (MEM_IMAGE == mbi.Type) {
				GetModuleFileNameExA(
					process,
					(HMODULE)addr,
					moduleName,
					sizeof(moduleName)
				);
			}*/
			return true;
		}

		virtual bool Debug() {
			return true;
		}

		virtual bool Release() {
			return true;
		}
	};


	ModuleLayout *CreateModuleLayout(process_t process) {
		return new WinModuleLayout(process);
	}
};


#endif