#ifndef _NEUTRINO_TRAMPOLINE_X86_64_H_
#define _NEUTRINO_TRAMPOLINE_X86_64_H_

#include "Neutrino.Types.h"

namespace Neutrino {

	class TrampolineX8664 {
	public:
		static DWORD GetCodeMemSize();
		static DWORD MakeTrampoline(BYTE *outBuffer, UINTPTR virtualStack, UINTPTR jumpBuff, UINTPTR environment, UINTPTR destination, UINTPTR mem);
	};

};

#endif