#include "Neutrino.Memory.h"

#include <sys/mman.h>

namespace Neutrino {
	static const DWORD PageProtections[8] = {
		PROT_NONE,				                        // 0 ---
		PROT_EXEC,              				// 1 --X
		PROT_WRITE,	                			// 2 -W- (specified as RW)
		PROT_WRITE | PROT_EXEC,		            // 3 -WX (specified as RWX)
		PROT_READ,				                // 4 R--
		PROT_READ | PROT_EXEC,			        // 5 R-X
		PROT_READ | PROT_WRITE,			    	// 6 RW-
		PROT_READ | PROT_WRITE | PROT_EXEC		// 7 RWX
	};

	void *Alloc(void *addr, size_t size, DWORD prot, int fd, off_t offset) {
        int flags = ((fd) < 0) ? MAP_SHARED | MAP_ANONYMOUS : MAP_SHARED;
		void *ret = mmap(addr, size, PageProtections[prot], flags, fd, offset);

		if (ret == ((void *)-1))
			return nullptr;

		return ret;
	}

	void Protect(void *addr, size_t size, DWORD newProt, DWORD &oldProt) {
		mprotect(addr, size, newProt);
	}

	void Free(void *addr, size_t size) {
		munmap(addr, size);
	}
};
