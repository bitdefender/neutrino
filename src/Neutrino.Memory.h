#ifndef _NEUTRINO_MEMORY_H_
#define _NEUTRINO_MEMORY_H_

#include "Neutrino.Types.h"

#include <sys/types.h>

namespace Neutrino {
	const DWORD PAGE_PROTECTION_READ = 0x4;
	const DWORD PAGE_PROTECTION_WRITE = 0x2;
	const DWORD PAGE_PROTECTION_EXECUTE = 0x1;

	void *Alloc(void *addr, size_t size, DWORD prot, int fd, off_t offset);
	void Protect(void *addr, size_t size, DWORD newProt, DWORD &oldProt);
	void Free(void *addr, size_t size);
}



#ifdef _WIN32

// virtual memory functions
	

#elif defined(__linux__)
	static const DWORD PageProtections[8] = {
		PROT_NONE,							// 0 ---
		PROT_EXEC,							// 1 --X
		PROT_WRITE,							// 2 -W-
		PROT_EXEC | PROT_WRITE,				// 3 -WX
		PROT_READ,							// 4 R--
		PROT_EXEC | PROT_READ,				// 5 R-X
		PROT_WRITE | PROT_READ,				// 6 RW-
		PROT_EXEC | PROT_WRITE | PROT_READ,	// 7 RWX
	};
// virtual memory functions
#define VIRTUAL_ALLOC(addr, size, protect, fd, offset) ({ int flags = ((fd) < 0) ? MAP_SHARED | MAP_ANONYMOUS : MAP_SHARED; \
		addr = mmap(addr, (size), (protect), flags, (fd), (offset)); addr; })
#define VIRTUAL_PROTECT(addr, size, newProtect, oldProtect) (0 == mprotect((addr), (size), (newProtect)))

#endif




#endif