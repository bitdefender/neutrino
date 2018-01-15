#include "Neutrino.Translator.h"
#ifndef _NEUTRINO_TRANSLATOR_HPP_
#define _NEUTRINO_TRANSLATOR_HPP_

#include <cstring>

#ifdef _MSC_VER
#define DEBUG_BREAK __asm int 3
#else
#define DEBUG_BREAK asm volatile("int $0x3")
#endif

namespace Neutrino {

	template <typename STRATEGY>
	void Translator<STRATEGY>::Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		state.flags = 0;

		DWORD ret, tbl;
		do {
			if (5 >= szOut) {
				// clear prefixes
				// insert jmp to new buffer here
			}

			tbl = (state.flags & FLAG_EXT) ? 1 : 0;
			state.opCode = pIn[0];

			TranslateOpcodeFunc funcOpcode = this->translateOpcode[tbl][state.opCode];
			ret = (this->*funcOpcode)(pIn, pOut, szOut, state);
		} while (ret != PARSED_OPCODE);

		tbl = (state.flags & FLAG_EXT) ? 1 : 0;
		TranslateOperandFunc funcOperand = this->translateOperand[tbl][state.opCode];
		(this->*funcOperand)(pIn, pOut, szOut, state);
	}

	/* Helpers */
	template<int count>
	inline bool CopyBytes(const BYTE *&bIn, BYTE *&bOut, int &szOut) {
		if (count >= szOut) {
			return false;
		}

		memcpy(bOut, bIn, count);

		bOut += count;
		bIn += count;
		szOut -= count;
		return true;
	}

	/*template <typename STRATEGY>
	bool Translator<STRATEGY>::TraceWrite(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest) {
		return TraceReached(pOut, szOut, state, dest);
	}*/

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateOpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
	}

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		pOut[0] = pIn[0];
		pOut++; pIn++;
		szOut--;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	template<DWORD flag>
	DWORD Translator<STRATEGY>::TranslatePrefix(const BYTE *&pIn, BYTE *&pOut, int & szOut, TranslationState &state) {
		CopyBytes<1>(pIn, pOut, szOut);

		state.flags |= flag;
		state.pfxCount++;
		return PARSED_PREFIX;
	}

	template <typename STRATEGY>
	template<BYTE destSize>
	DWORD Translator<STRATEGY>::TranslateJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
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

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[2]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		pIn += 1 + destSize;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	template<BYTE destSize>
	DWORD Translator<STRATEGY>::TranslateJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
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

		TouchStatic(pOut, szOut, state, (UINTPTR)fallthrough);
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

		TouchStatic(pOut, szOut, state, (UINTPTR)taken);
		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)taken, (UINTPTR *)&pOut[2]);

		state.flags |= FLAG_JUMP;

		pCode = code;
		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		UINTPTR offset;

		static const BYTE code[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,						// 0x00 - push imm32
			0xff, 0x25, 0x00, 0x00, 0x00, 0x00					// 0x05 - call [dest]
		};

		const BYTE *pCode = code;

		offset = *(DWORD *)(&pIn[1]);
		const void *dest = &pIn[1] + offset + 4;

		TouchStatic(pOut, szOut, state, (UINTPTR)dest);

		UINTPTR *pPatch = (UINTPTR *)&pOut[1];

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[0x07]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		*pPatch = ((UINTPTR)pIn + 5);

		pIn += 5;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateRet(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [ebxSave], ebx
			0x5B												// 0x06 - pop ebx
		};
		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		
		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x06 - jmp SolveIndirect
		};
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x07]);

		pIn++;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [ebxSave], ebx
			0x5B												// 0x06 - pop ebx
		};
		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;
		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov ebx, [ebxSave]
			0x8D, 0xA4, 0x24, 0x00, 0x00, 0x00, 0x00,			// 0x06 - lea esp, [esp + off]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x0D - jmp SolveIndirect
		};
		pCode = codeSfx;
		pStart = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);
		*(int *)&pStart[0x09] = (int)(*(short *)&pIn[1]); 
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x0E]);

		pIn += 3;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}


	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,					// 0x00 - push imm32
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x05 - mov [ebxSave], ebx
			0x8B,											// 0x0B - mov ebx, [<---->]
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		TranslateModRMOperand(pIn, pOut, szOut, state);

		*(UINTPTR *)&pStart[1] = (UINTPTR)pIn;

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x07]);

		pStart[0x0C] &= 0xC7; // clear ext
		pStart[0x0C] |= 0x18; // set ext to EBX

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x06 - jmp SolveIndirect
		};

		pCode = codeSfx;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);
				
		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pCont[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x07]);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	template <typename STRATEGY>
	DWORD Translator<STRATEGY>::TranslateJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		static const BYTE codePfx[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov [ebxSave], ebx
			0x8B,											// 0x06 - mov ebx, [<---->]
		};

		const BYTE *pCode = codePfx;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(codePfx)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		TranslateModRMOperand(pIn, pOut, szOut, state);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pStart[0x02]);

		pStart[0x07] &= 0xC7; // clear ext
		pStart[0x07] |= 0x18; // set ext to EBX

		TouchDynamic(pOut, szOut, state);

		static const BYTE codeSfx[] = {
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x06 - jmp SolveIndirect
		};

		pCode = codeSfx;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(codeSfx)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_JMP_REG_BKP, 0, (UINTPTR *)&pCont[0x02]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x07]);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}


	/* Operands */

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateOperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
	}

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateNoOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		// nothing to copy
	}

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateModRMOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		BYTE modRM = pIn[0];
		BYTE mod = modRM & 0xC0;

		CopyBytes<1>(pIn, pOut, szOut);

		if ((0x4 == (modRM & 0x7)) && (mod != 0xC0)) {
			// also copy the SIB byte
			BYTE sib = pIn[0];

			CopyBytes<1>(pIn, pOut, szOut);

			if (0x5 == (sib & 0x7)) {
				if ((mod == 0x00) || (mod == 0x80)) {
					CopyBytes<4>(pIn, pOut, szOut);
				}
				else if (mod == 0x40) {
					CopyBytes<1>(pIn, pOut, szOut);
				}
			}
		}

		if (mod == 0x40) {
			// also copy byte displacement
			CopyBytes<1>(pIn, pOut, szOut);
		}
		else if ((mod == 0x80) || ((mod == 0x00) && (0x5 == (modRM & 0x7)))) {
			// also copy dword displacement
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateImm8Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		CopyBytes<1>(pIn, pOut, szOut);
	}

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateImm1632Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		if (state.flags & FLAG_O16) {
			CopyBytes<2>(pIn, pOut, szOut);
		}
		else {
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

	template <typename STRATEGY>
	void Translator<STRATEGY>::TranslateImm32Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		CopyBytes<4>(pIn, pOut, szOut);
	}


	/* = Opcode translation table ================================================= */

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOpcodeFunc Translator<STRATEGY>::translateOpcode0xFF[8] = {
		/* 0x00 */ &Translator::TranslateOpcode,
		/* 0x01 */ &Translator::TranslateOpcode,
		/* 0x02 */ &Translator::TranslateCallModRM,
		/* 0x03 */ &Translator::TranslateOpcodeErr,
		/* 0x04 */ &Translator::TranslateJumpModRM,
		/* 0x05 */ &Translator::TranslateOpcodeErr,
		/* 0x06 */ &Translator::TranslateOpcode,
		/* 0x07 */ &Translator::TranslateOpcodeErr
	};

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOpcodeFunc Translator<STRATEGY>::translateOpcode[2][0x100] = {
		{
			/* 0x00 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x04 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x08 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x0C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslatePrefix<FLAG_EXT>,

			/* 0x10 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x14 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x18 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x1C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x20 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x24 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x28 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x2C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x30 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x34 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x38 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x3C */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x40 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x44 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x48 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x4C */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,

			/* 0x50 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x54 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x58 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x5C */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,

			/* 0x60 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x64 */ &Translator<STRATEGY>::TranslatePrefix<FLAG_FS>, &Translator<STRATEGY>::TranslatePrefix<FLAG_GS>, &Translator<STRATEGY>::TranslatePrefix<FLAG_O16>, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x68 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x6C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x70 */ &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>,
			/* 0x74 */ &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>,
			/* 0x78 */ &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>,
			/* 0x7C */ &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>, &Translator<STRATEGY>::TranslateJxx<1>,

			/* 0x80 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x84 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x88 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x8C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x90 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x94 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x98 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x9C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xA0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xA4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xA8 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xAC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xB0 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xB4 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xB8 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xBC */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,

			/* 0xC0 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateRetn, &Translator<STRATEGY>::TranslateRet,
			/* 0xC4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xC8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xCC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xD0 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xD4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xD8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xDC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xE0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xE4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xE8 */ &Translator<STRATEGY>::TranslateCall, &Translator<STRATEGY>::TranslateJmp<4>, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateJmp<1>,
			/* 0xEC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xF0 */ &Translator<STRATEGY>::TranslatePrefix<FLAG_LOCK>, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslatePrefix<FLAG_REPNZ>, &Translator<STRATEGY>::TranslatePrefix<FLAG_REPZ>,
			/* 0xF4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xF8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xFC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xFF */ &Translator<STRATEGY>::TranslateExtOpcode<translateOpcode0xFF>
		},{
			/* 0x00 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x04 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x08 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x0C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x10 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x14 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x18 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x1C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x20 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x24 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x28 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x2C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x30 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x34 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x38 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x3C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x40 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x44 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x48 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x4C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x50 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x54 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x58 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x5C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0x60 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x64 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x68 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x6C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,

			/* 0x70 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x74 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x78 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0x7C */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,

			/* 0x80 */ &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>,
			/* 0x84 */ &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>,
			/* 0x88 */ &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>,
			/* 0x8C */ &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>, &Translator<STRATEGY>::TranslateJxx<4u>,

			/* 0x90 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x94 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x98 */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0x9C */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,

			/* 0xA0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xA4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xA8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xAC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,

			/* 0xB0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xB4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xB8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xBC */ &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xC0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xC4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xC8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xCC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xD0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xD4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xD8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,
			/* 0xDC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,

			/* 0xE0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xE4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xE8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xEC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcode,

			/* 0xF0 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xF4 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xF8 */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr,
			/* 0xFC */ &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr, &Translator<STRATEGY>::TranslateOpcodeErr
		}
	};


	/* = Operand translation table ================================================ */

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOperandFunc Translator<STRATEGY>::translateOperand0xF6[8] = {
		/* 0x00 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
		/* 0x01 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
		/* 0x02 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x03 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x04 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x05 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x06 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x07 */ &Translator<STRATEGY>::TranslateModRMOperand
	};

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOperandFunc Translator<STRATEGY>::translateOperand0xF7[8] = {
		/* 0x00 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm1632Operand>,
		/* 0x01 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm1632Operand>,
		/* 0x02 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x03 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x04 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x05 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x06 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x07 */ &Translator<STRATEGY>::TranslateModRMOperand
	};

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOperandFunc Translator<STRATEGY>::translateOperand0xFF[8] = {
		/* 0x00 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x01 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x02 */ &Translator<STRATEGY>::TranslateNoOperand,
		/* 0x03 */ &Translator<STRATEGY>::TranslateNoOperand,
		/* 0x04 */ &Translator<STRATEGY>::TranslateNoOperand,
		/* 0x05 */ &Translator<STRATEGY>::TranslateNoOperand,
		/* 0x06 */ &Translator<STRATEGY>::TranslateModRMOperand,
		/* 0x07 */ &Translator<STRATEGY>::TranslateNoOperand
	};

	template <typename STRATEGY>
	typename Translator<STRATEGY>::TranslateOperandFunc Translator<STRATEGY>::translateOperand[2][0x100] = {
		{
			/* 0x00 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x04 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x08 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x0C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x10 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x14 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x18 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x1C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x20 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x24 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x28 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x2C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x30 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x34 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x38 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x3C */ &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x40 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x44 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x48 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x4C */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,

			/* 0x50 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x54 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x58 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x5C */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,

			/* 0x60 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x64 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x68 */ &Translator<STRATEGY>::TranslateImm1632Operand,
			/* 0x69 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm1632Operand>,
			/* 0x6A */ &Translator<STRATEGY>::TranslateImm8Operand,
			/* 0x6B */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0x6C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x70 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x74 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x78 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x7C */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,

			/* 0x80 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0x81 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm1632Operand>,
			/* 0x82 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0x83 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0x84 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x88 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x8C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x90 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x94 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x98 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x9C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xA0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateImm1632Operand,
			/* 0xA4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xA8 */ &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0xAC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xB0 */ &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand,
			/* 0xB4 */ &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand, &Translator<STRATEGY>::TranslateImm8Operand,
			/* 0xB8 */ &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand,
			/* 0xBC */ &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand, &Translator<STRATEGY>::TranslateImm1632Operand,

			/* 0xC0 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0xC1 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0xC2 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0xC4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xC6 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0xC7 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm1632Operand>,
			/* 0xC8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xCC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xD0 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0xD4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xD8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xDC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xE0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xE4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xE8 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0xEC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xF0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xF4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xF6 */ &Translator<STRATEGY>::TranslateExtOperand<translateOperand0xF6>,
			/* 0xF7 */ &Translator<STRATEGY>::TranslateExtOperand<translateOperand0xF7>,
			/* 0xF8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xFC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xFF */ &Translator<STRATEGY>::TranslateExtOperand<translateOperand0xFF>
		},{
			/* 0x00 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x04 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x08 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x0C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x10 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x14 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x18 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x1C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x20 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x24 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x28 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x2C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x30 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x34 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x38 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x3C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x40 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x44 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x48 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x4C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x50 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x54 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x58 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x5C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0x60 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x64 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x68 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x6C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,

			/* 0x70 */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0x71 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x74 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x78 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0x7C */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,

			/* 0x80 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x84 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x88 */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,
			/* 0x8C */ &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand, &Translator<STRATEGY>::TranslateNoOperand,

			/* 0x90 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x94 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x98 */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0x9C */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,

			/* 0xA0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xA4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xA8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0xAC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,

			/* 0xB0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xB4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0xB8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xBA */ &Translator<STRATEGY>::TranslateAggOperand<&Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateImm8Operand>,
			/* 0xBB */ &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xBC */ &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xC0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xC4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xC8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xCC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xD0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xD4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0xD8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,
			/* 0xDC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,

			/* 0xE0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xE4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xE8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xEC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateModRMOperand,

			/* 0xF0 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xF4 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xF8 */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr,
			/* 0xFC */ &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr, &Translator<STRATEGY>::TranslateOperandErr
		}
	};
};

#endif
