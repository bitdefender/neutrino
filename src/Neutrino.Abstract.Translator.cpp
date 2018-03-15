#include "Neutrino.Abstract.Translator.h"

namespace Neutrino {

	TranslationState::TranslationState() {
		flags = 0;
		opCode = 0;
		pfxCount = 0;
		patchCount = 0;

		for (int i = 0; i < 8; ++i) {
			patch[i].jumpType = PATCH_TYPE_UNUSED;
		}
	}

	void TranslationState::Patch(DWORD jump, UINTPTR dest, UINTPTR *addr) {
		patch[patchCount].jumpType = jump;
		patch[patchCount].destination = dest;
		patch[patchCount].patch = addr;
		patchCount++;
	}

};
