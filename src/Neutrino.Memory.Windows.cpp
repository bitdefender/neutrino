#include "Neutrino.Memory.h"

#include <Windows.h>

namespace Neutrino {
	static const DWORD PageProtections[8] = {
		PAGE_NOACCESS,				// 0 ---
		PAGE_EXECUTE,				// 1 --X
		PAGE_READWRITE,				// 2 -W- (specified as RW)
		PAGE_EXECUTE_READWRITE,		// 3 -WX (specified as RWX)
		PAGE_READONLY,				// 4 R--
		PAGE_EXECUTE_READ,			// 5 R-X
		PAGE_READWRITE,				// 6 RW-
		PAGE_EXECUTE_READWRITE		// 7 RWX
	};

	void *Alloc(void *addr, size_t size, DWORD prot, int fd, off_t offset) {
		return VirtualAlloc(addr, size, MEM_RESERVE | MEM_COMMIT, PageProtections[prot]);
	}

	void Protect(void *addr, size_t size, DWORD newProt, DWORD &oldProt) {
		VirtualProtect(addr, size, PageProtections[newProt], (::PDWORD)&oldProt);
	}

	void Free(void *addr, size_t size) {
		VirtualFree(addr, size, MEM_RELEASE | MEM_FREE);
	}
};