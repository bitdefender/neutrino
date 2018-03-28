#include "Neutrino.Bit.Hacks.h"

#include <intrin.h> 

namespace Neutrino {

    DWORD PopCount(DWORD x) {
		return __popcnt(x);
	}

};
