#include "Neutrino.Translator.X86.Base.h"

#ifndef _NEUTRINO_TRANSLATOR_X86_BASE_HPP_
#define _NEUTRINO_TRANSLATOR_X86_BASE_HPP_

#include <intrin.h> 

namespace Neutrino {

	static DWORD PopCount(DWORD x) {
		return __popcnt(x);
	}

	static bool ClearPrefixes(const BYTE *&pIn, TranslationState &state) {
		DWORD pfx = state.flags & (FLAG_O16 | FLAG_A16 | FLAG_LOCK | FLAG_REPZ | FLAG_REPNZ | FLAG_GS | FLAG_FS);
		pIn -= PopCount(pfx);
		return true;
	}

	template <typename TABLE>
	void Translator<TABLE>::Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		BYTE *outStart = pOut;
				
		state.Init();

		DWORD ret, tbl;
		do {
			if (5 >= szOut) {
				// clear prefixes
				// insert jmp to new buffer here
			}

			tbl = (state.flags & FLAG_EXT) ? 1 : 0;
			state.opCode = pIn[0];
			state.subOpcode = (pIn[1] >> 3) & 0x07;

			auto opcodeFunc = table.opcodeTbl[tbl][state.opCode];
			ret = (table.*opcodeFunc)(pIn, pOut, szOut, state);
		} while (ret != PARSED_OPCODE);

		tbl = (state.flags & FLAG_EXT) ? 1 : 0;
		//TranslateOperandFunc funcOperand = this->translateOperand[tbl][state.opCode];
		auto operandFunc = table.operandTbl[tbl][state.opCode];
		(table.*operandFunc)(pIn, pOut, szOut, state);

		if (0 != state.pfxFuncCount) {
			BYTE tmpCode[0x100];
			size_t tmpSize = pOut - outStart;

			/* This is really ugly */
			state.ripJumpDest += (QWORD)pIn;

			/* Roll back the out buffer */
			memcpy(tmpCode, outStart, tmpSize);
			pOut = outStart;
			szOut += (int)tmpSize;

			/* Place the prefix code */
			state.WritePrefix(pOut, szOut); //codePfx(pOut, szOut, state);

			/* Place the original code back */
			memcpy(pOut, tmpCode, tmpSize);
			pOut += tmpSize;
			szOut -= (int)tmpSize;
		}

		if (0 != state.sfxFuncCount) {
			state.WriteSuffix(pOut, szOut);
		}
	}

	template<typename TABLE>
	DWORD Translator<TABLE>::GetCodeMemSize() {
		return table.GetCodeMemSize();
	}

	template<typename TABLE>
	void Translator<TABLE>::SetCodeMem(BYTE *ptr) {
		table.SetCodeMem(ptr);
	}

	template <typename TABLE>
	void Translator<TABLE>::Reset() {
		table.Reset();
	}

	template<typename TABLE>
	inline AbstractResult * Translator<TABLE>::GetResult() {
		return table.GetResult();
	}

	template<typename TABLE>
	inline void Translator<TABLE>::PushBasicBlock(UINTPTR bb) {
		table.PushBasicBlock(bb);
	}

	template<typename TABLE>
	inline UINTPTR Translator<TABLE>::LastBasicBlock() const {
		return table.LastBasicBlock();
	}

	template<typename TABLE>
	inline void Translator<TABLE>::TouchDeferred() {
		table.TouchDeferred();
	}
};

#endif
