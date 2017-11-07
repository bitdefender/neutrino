#include "Neutrino.Translator.h"
#ifndef _NEUTRINO_TRANSLATOR_HPP_
#define _NEUTRINO_TRANSLATOR_HPP_

#include <cstring>

namespace Neutrino {

	/* Helpers */

	template<int count>
	inline bool Translator::CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut)	{
		if (count >= szOut) {
			return false;
		}

		memcpy(bOut, bIn, count);

		bOut += count;
		bIn += count;
		szOut -= count;
		return true;
	}

	template<DWORD flag>
	inline DWORD Translator::TranslatePrefix(const BYTE *&pIn, BYTE *&pOut, int & szOut, InstructionState &state) {
		CopyBytes<1>(pIn, pOut, szOut);

		state.flags |= flag;
		state.pfxCount++;
		return PARSED_PREFIX;
	}

	template<BYTE destSize>
	inline DWORD Translator::TranslateJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		const BYTE *pCode = code;
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(int8_t *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(DWORD *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *dest = &pIn[1] + offset + destSize;

		TraceWrite(pOut, szOut, state, (UINTPTR)dest);

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[2]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		pIn += 1 + destSize;
		return PARSED_OPCODE;
	}

	template<BYTE destSize>
	inline DWORD Translator::TranslateJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		static const BYTE code[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
		};

		const BYTE *pCode = code;
		UINTPTR offset;

		switch (destSize) {
		case 1:
			offset = *(int8_t *)(&pIn[1]);
			break;
		case 4:
			offset = (UINTPTR)(*(DWORD *)(&pIn[1]));
			break;
		default:
			offset = 0;
		};

		const void *fallthrough = &pIn[1] + destSize;
		const void *taken = (const BYTE *)fallthrough + offset;

		BYTE *pStart = pOut;

		CopyBytes<1 + destSize>(pIn, pOut, szOut);
		pIn += 1 + destSize;

		BYTE *pNow = pOut;

		TraceWrite(pOut, szOut, state, (UINTPTR)fallthrough);
		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)fallthrough, (UINTPTR *)&pOut[2]);

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		switch (destSize) {
		case 1:
			*(BYTE *)&pStart[1] = (BYTE)(pOut - pNow);
			break;
		case 4:
			*(DWORD *)&pStart[1] = (DWORD)(pOut - pNow);
			break;
		default:
			offset = 0;
		};

		TraceWrite(pOut, szOut, state, (UINTPTR)taken);
		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)taken, (UINTPTR *)&pOut[2]);

		state.flags |= FLAG_JUMP;

		pCode = code;
		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		return PARSED_OPCODE;
	}

	template<Translator::TranslateOpcodeFunc * funcs>
	inline DWORD Translator::TranslateExtOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		BYTE ext = (pIn[1] >> 3) & 0x07;

		return (this->*funcs[ext])(pIn, pOut, szOut, state);
	}

	template<Translator::TranslateOperandFunc f1, Translator::TranslateOperandFunc f2>
	inline void Translator::TranslateAggOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		(this->*f1)(pIn, pOut, szOut, state);
		(this->*f2)(pIn, pOut, szOut, state);
	}

	template<Translator::TranslateOperandFunc * funcs>
	inline void Translator::TranslateExtOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		BYTE ext = (pIn[0] >> 3) & 0x07;

		(this->*funcs[ext])(pIn, pOut, szOut, state);
	}

};

#endif
