#include "Neutrino.Environment.h"

namespace Neutrino {
	bool BasicBlock::Equals(UINTPTR rhs) {
		return address == rhs;
	}

	unsigned int __cdecl RetAddr_cdecl_2(unsigned int, unsigned char *) {
		return (unsigned int)GET_RETURN_ADDR(); \
	}
};
