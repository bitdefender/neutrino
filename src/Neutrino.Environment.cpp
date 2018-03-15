#include "Neutrino.Environment.h"

namespace Neutrino {

	Heap BasicBlock::heap(1 << 20);

	bool BasicBlock::Equals(UINTPTR rhs) {
		return address == rhs;
	}

	void *BasicBlock::operator new(size_t size) {
		return heap.Alloc(size);
	}

	void BasicBlock::operator delete(void *p) {
		heap.Free(p);
	}

	UINTPTR __cdecl RetAddr_cdecl_2(unsigned int, unsigned char *) {
		return (UINTPTR)GET_RETURN_ADDR(); \
	}
};
