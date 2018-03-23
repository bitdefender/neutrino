#include "Neutrino.Abstract.Translator.h"

namespace Neutrino {

	TranslationState::TranslationState() {
		flags = 0;
		opCode = 0;
		pfxCount = 0;
		patchCount = 0;

		ripJumpDest = 0;

		pfxFuncCount = 0;
		sfxFuncCount = 0;

		for (int i = 0; i < 8; ++i) {
			patch[i].jumpType = PATCH_TYPE_UNUSED;
		}
	}

	void TranslationState::Init() {
		flags = 0;
		pfxFuncCount = 0;
		sfxFuncCount = 0;
	}

	void TranslationState::Patch(DWORD jump, UINTPTR dest, UINTPTR *addr) {
		patch[patchCount].jumpType = jump;
		patch[patchCount].destination = dest;
		patch[patchCount].patch = addr;
		patchCount++;
	}

	void TranslationState::AddPrefix(CodePfxFunc pfx) {
		codePfx[pfxFuncCount] = pfx;
		pfxFuncCount++;
	}

	void TranslationState::AddSuffix(CodeSfxFunc sfx) {
		codeSfx[sfxFuncCount] = sfx;
		sfxFuncCount++;
	}

	void TranslationState::WritePrefix(BYTE *& pOut, int &szOut) {
		for (int i = 0; i < pfxFuncCount; ++i) {
			codePfx[i](pOut, szOut, *this);
		}
	}

	void TranslationState::WriteSuffix(BYTE *& pOut, int &szOut) {
		for (int i = sfxFuncCount - 1; i >= 0; --i) {
			codeSfx[i](pOut, szOut, *this);
		}
	}
};
