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

#endif
