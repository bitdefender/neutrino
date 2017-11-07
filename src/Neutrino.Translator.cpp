#include "Neutrino.Translator.h"

namespace Neutrino {

	void Translator::Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		state.flags = 0;

		DWORD ret, tbl;
		do {
			if (5 >= szOut) {
				// clear prefixes
				// insert jmp to new buffer here
			}

			tbl = (state.flags & FLAG_EXT) ? 1 : 0;
			state.opCode = pIn[0];
			ret = (this->*translateOpcode[tbl][state.opCode])(pIn, pOut, szOut, state);
		} while (ret != PARSED_OPCODE);

		tbl = (state.flags & FLAG_EXT) ? 1 : 0;
		(this->*translateOperand[tbl][state.opCode])(pIn, pOut, szOut, state);
	}

	bool Translator::TraceWrite(BYTE *&pOut, int &szOut, InstructionState &state, UINTPTR dest) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01,									// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,			// 0x12 - lea eax, [4 * eax + traceBase]
			0xC7, 0x00, 0x00, 0x00, 0x00, 0x00,					// 0x19 - mov [eax], dest
			0xA1, 0x00, 0x00, 0x00, 0x00						// 0x1F - mov eax, [eaxSave]
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;
		
		if (!CopyBytes<sizeof(code)>(pCode, pOut, szOut)) {
			return false;
		}

		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x01]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x06]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x0E]);
		state.Patch(PATCH_TYPE_TRACE_BASE, 0, (UINTPTR *)&pStart[0x15]);
		*(UINTPTR *)&pStart[0x1B] = dest;
		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x20]);

		return true;
	}

	DWORD Translator::TranslateOpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		__asm int 3;
	}

	DWORD Translator::TranslateOpcode(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		pOut[0] = pIn[0];
		pOut++; pIn++;
		szOut--;
		return PARSED_OPCODE;
	}

	inline DWORD Translator::TranslateCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		UINTPTR offset;

		static const BYTE code[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,						// 0x00 - push imm32
			0xff, 0x25, 0x00, 0x00, 0x00, 0x00					// 0x05 - call [dest]
		};

		const BYTE *pCode = code;

		offset = *(DWORD *)(&pIn[1]);
		const void *dest = &pIn[1] + offset + 4;

		TraceWrite(pOut, szOut, state, (UINTPTR)dest);

		UINTPTR *pPatch = (UINTPTR *)&pOut[1];

		state.Patch(PATCH_TYPE_DIRECT, (UINTPTR)dest, (UINTPTR *)&pOut[0x07]);
		state.flags |= FLAG_JUMP;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);
		*pPatch = ((UINTPTR)pIn + 5);

		pIn += 5;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	inline DWORD Translator::TranslateRet(const BYTE *& pIn, BYTE *& pOut, int & szOut, InstructionState & state) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01,									// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,			// 0x12 - lea eax, [4 * eax + traceBase]
			0x8F, 0x00,											// 0x19 - pop [eax]
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x1B - mov eax, [eaxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x20 - jmp SolveIndirect
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x01]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x06]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x0E]);
		state.Patch(PATCH_TYPE_TRACE_BASE, 0, (UINTPTR *)&pStart[0x15]);
		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x1C]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x21]);
		
		pIn++;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	DWORD Translator::TranslateRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		static const BYTE code[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01,									// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00,						// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00,			// 0x12 - lea eax, [4 * eax + traceBase]
			0x8F, 0x00,											// 0x19 - pop [eax]
			0xA1, 0x00, 0x00, 0x00, 0x00,						// 0x1B - mov eax, [eaxSave]
			0x8D, 0xA4, 0x24, 0x00, 0x00, 0x00, 0x00,			// 0x20 - lea esp, [esp + off]
			0xE9, 0x00, 0x00, 0x00, 0x00						// 0x27 - jmp SolveIndirect
		};

		const BYTE *pCode = code;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(code)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x01]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x06]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pStart[0x0E]);
		state.Patch(PATCH_TYPE_TRACE_BASE, 0, (UINTPTR *)&pStart[0x15]);
		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pStart[0x1C]);
		*(int *)&pStart[0x23] = (int)(*(short *)&pIn[1]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pStart[0x28]);
		
		pIn += 3;
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}


	DWORD Translator::TranslateCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		static const BYTE code1[] = {
			0x68, 0x00, 0x00, 0x00, 0x00,					// 0x00 - push imm32
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x05 - mov [ebxSave], ebx
			0x8B,											// 0x0B - mov ebx, [<---->]
		};

		const BYTE *pCode = code1;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(code1)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		TranslateModRMOperand(pIn, pOut, szOut, state);

		*(UINTPTR *)&pStart[1] = (UINTPTR)pIn;

		state.Patch(PATCH_TYPE_REG2_BACKUP, 0, (UINTPTR *)&pStart[0x07]);

		pStart[0x0C] &= 0xC7; // clear ext
		pStart[0x0C] |= 0x18; // set ext to EBX

		static const BYTE code2[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00, 					// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01, 								// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00, 					// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00, 		// 0x12 - lea eax, [4 * eax + traceBase]
			0x89, 0x18, 									// 0x19 - mov [eax], ebx
			0xA1, 0x00, 0x00, 0x00, 0x00, 					// 0x1B - mov eax, [eaxSave]
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x20 - mob ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x26 - jmp SolveIndirect
		};

		pCode = code2;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(code2)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pCont[0x01]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pCont[0x06]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pCont[0x0E]);
		state.Patch(PATCH_TYPE_TRACE_BASE, 0, (UINTPTR *)&pCont[0x15]);
		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pCont[0x1C]);
		state.Patch(PATCH_TYPE_REG2_BACKUP, 0, (UINTPTR *)&pCont[0x22]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x27]);
		
		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	DWORD Translator::TranslateJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		static const BYTE code1[] = {
			0x89, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x00 - mov [ebxSave], ebx
			0x8B,											// 0x06 - mov ebx, [<---->]
		};

		const BYTE *pCode = code1;
		BYTE *pStart = pOut;

		CopyBytes<sizeof(code1)>(pCode, pOut, szOut);
		pIn += 1; szOut -= 1;
		TranslateModRMOperand(pIn, pOut, szOut, state);

		state.Patch(PATCH_TYPE_REG2_BACKUP, 0, (UINTPTR *)&pStart[0x02]);

		pStart[0x07] &= 0xC7; // clear ext
		pStart[0x07] |= 0x18; // set ext to EBX

		static const BYTE code2[] = {
			0xA3, 0x00, 0x00, 0x00, 0x00,					// 0x00 - mov [eaxSave], eax
			0xA1, 0x00, 0x00, 0x00, 0x00, 					// 0x05 - mov eax, [traceIndex]
			0x8D, 0x40, 0x01, 								// 0x0A - lea eax, [eax + 0x1]
			0xA3, 0x00, 0x00, 0x00, 0x00, 					// 0x0D - mov [traceIndex], eax
			0x8D, 0x04, 0x85, 0x00, 0x00, 0x00, 0x00, 		// 0x12 - lea eax, [4 * eax + traceBase]
			0x89, 0x18, 									// 0x19 - mov [eax], ebx
			0xA1, 0x00, 0x00, 0x00, 0x00, 					// 0x1B - mov eax, [eaxSave]
			0x8B, 0x1D, 0x00, 0x00, 0x00, 0x00,				// 0x20 - mob ebx, [ebxSave]
			0xE9, 0x00, 0x00, 0x00, 0x00					// 0x26 - jmp SolveIndirect
		};

		pCode = code2;
		BYTE *pCont = pOut;
		CopyBytes<sizeof(code2)>(pCode, pOut, szOut);

		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pCont[0x01]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pCont[0x06]);
		state.Patch(PATCH_TYPE_TRACE_INDEX, 0, (UINTPTR *)&pCont[0x0E]);
		state.Patch(PATCH_TYPE_TRACE_BASE, 0, (UINTPTR *)&pCont[0x15]);
		state.Patch(PATCH_TYPE_REG1_BACKUP, 0, (UINTPTR *)&pCont[0x1C]);
		state.Patch(PATCH_TYPE_REG2_BACKUP, 0, (UINTPTR *)&pCont[0x22]);
		state.Patch(PATCH_TYPE_INDIRECT, 0, (UINTPTR *)&pCont[0x27]);

		state.flags |= FLAG_JUMP;
		return PARSED_OPCODE;
	}

	/* Operands */

	void Translator::TranslateOperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		__asm int 3;
	}

	void Translator::TranslateNoOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
		// nothing to copy
	}

	void Translator::TranslateModRMOperand(const BYTE *&pIn, BYTE *&pOut, int &szOut, InstructionState &state) {
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
				} else if (mod == 0x40) {
					CopyBytes<1>(pIn, pOut, szOut);
				}
			}
		}

		if (mod == 0x40) {
			// also copy byte displacement
			CopyBytes<1>(pIn, pOut, szOut);
		} else if ((mod == 0x80) || ((mod == 0x00) && (0x5 == (modRM & 0x7)))) {
			// also copy dword displacement
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

	inline void Translator::TranslateImm8Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, InstructionState & state) {
		CopyBytes<1>(pIn, pOut, szOut);
	}

	inline void Translator::TranslateImm1632Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, InstructionState & state) {
		if (state.flags & FLAG_O16) {
			CopyBytes<2>(pIn, pOut, szOut);
		} else {
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

	inline void Translator::TranslateImm32Operand(const BYTE *& pIn, BYTE *& pOut, int & szOut, InstructionState & state) {
		CopyBytes<4>(pIn, pOut, szOut);
	}

	/* = Opcode translation table ================================================= */

	Translator::TranslateOpcodeFunc Translator::translateOpcode0xFF[8] = {
		/* 0x00 */ &Translator::TranslateOpcode,
		/* 0x01 */ &Translator::TranslateOpcode,
		/* 0x02 */ &Translator::TranslateCallModRM,
		/* 0x03 */ &Translator::TranslateOpcodeErr,
		/* 0x04 */ &Translator::TranslateJumpModRM,
		/* 0x05 */ &Translator::TranslateOpcodeErr,
		/* 0x06 */ &Translator::TranslateOpcode,
		/* 0x07 */ &Translator::TranslateOpcodeErr
	};

	Translator::TranslateOpcodeFunc Translator::translateOpcode[2][0x100] = {
		{
			/* 0x00 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x04 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,
			/* 0x08 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x0C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslatePrefix<FLAG_EXT>,

			/* 0x10 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x14 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x18 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x1C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x20 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x24 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x28 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x2C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x30 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,
			/* 0x34 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x38 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,
			/* 0x3C */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x40 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x44 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x48 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x4C */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,

			/* 0x50 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x54 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x58 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x5C */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,

			/* 0x60 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x64 */ &Translator::TranslatePrefix<FLAG_FS>, &Translator::TranslatePrefix<FLAG_GS>, &Translator::TranslatePrefix<FLAG_O16>, &Translator::TranslateOpcodeErr,
			/* 0x68 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x6C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x70 */ &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>,
			/* 0x74 */ &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>,
			/* 0x78 */ &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>,
			/* 0x7C */ &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>, &Translator::TranslateJxx<1>,

			/* 0x80 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x84 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x88 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x8C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x90 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x94 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x98 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x9C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xA0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,
			/* 0xA4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA8 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xAC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xB0 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xB4 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xB8 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xBC */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,

			/* 0xC0 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateRetn, &Translator::TranslateRet,
			/* 0xC4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xC8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xCC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xD0 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xD4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xD8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xDC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xE0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xE4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xE8 */ &Translator::TranslateCall, &Translator::TranslateJmp<4>, &Translator::TranslateOpcodeErr, &Translator::TranslateJmp<1>,
			/* 0xEC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xF0 */ &Translator::TranslatePrefix<FLAG_LOCK>, &Translator::TranslateOpcodeErr, &Translator::TranslatePrefix<FLAG_REPNZ>, &Translator::TranslatePrefix<FLAG_REPZ>,
			/* 0xF4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xF8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xFC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, 
			/* 0xFF */ &Translator::TranslateExtOpcode<translateOpcode0xFF>
		},{
			/* 0x00 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x04 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x08 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x0C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x10 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x14 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x18 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x1C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x20 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x24 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x28 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x2C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x30 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x34 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x38 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x3C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x40 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x44 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x48 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x4C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x50 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x54 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x58 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x5C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x60 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x64 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x68 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x6C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr,

			/* 0x70 */ &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x74 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x78 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x7C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,

			/* 0x80 */ &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &TranslateJxx<4>,
			/* 0x84 */ &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &TranslateJxx<4>,
			/* 0x88 */ &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &TranslateJxx<4>,
			/* 0x8C */ &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &Translator::TranslateJxx<4>, &TranslateJxx<4>,

			/* 0x90 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x94 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x98 */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0x9C */ &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode, &Translator::TranslateOpcode,

			/* 0xA0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,
			/* 0xAC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode,

			/* 0xB0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xB4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcode,
			/* 0xB8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr,
			/* 0xBC */ &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcode, &Translator::TranslateOpcodeErr,

			/* 0xC0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xC4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xC8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xCC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xD0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xD4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xD8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xDC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xE0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xE4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xE8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xEC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xF0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xF4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xF8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xFC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr
		}
	};


	/* = Operand translation table ================================================ */

	Translator::TranslateOperandFunc Translator::translateOperand0xF6[8] = {
		/* 0x00 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
		/* 0x01 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
		/* 0x02 */ &Translator::TranslateModRMOperand,
		/* 0x03 */ &Translator::TranslateModRMOperand,
		/* 0x04 */ &Translator::TranslateModRMOperand,
		/* 0x05 */ &Translator::TranslateModRMOperand,
		/* 0x06 */ &Translator::TranslateModRMOperand,
		/* 0x07 */ &Translator::TranslateModRMOperand
	};

	Translator::TranslateOperandFunc Translator::translateOperand0xF7[8] = {
		/* 0x00 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm1632Operand>,
		/* 0x01 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm1632Operand>,
		/* 0x02 */ &Translator::TranslateModRMOperand,
		/* 0x03 */ &Translator::TranslateModRMOperand,
		/* 0x04 */ &Translator::TranslateModRMOperand,
		/* 0x05 */ &Translator::TranslateModRMOperand,
		/* 0x06 */ &Translator::TranslateModRMOperand,
		/* 0x07 */ &Translator::TranslateModRMOperand
	};

	Translator::TranslateOperandFunc Translator::translateOperand0xFF[8] = {
		/* 0x00 */ &Translator::TranslateModRMOperand,
		/* 0x01 */ &Translator::TranslateModRMOperand,
		/* 0x02 */ &Translator::TranslateNoOperand,
		/* 0x03 */ &Translator::TranslateNoOperand,
		/* 0x04 */ &Translator::TranslateNoOperand,
		/* 0x05 */ &Translator::TranslateNoOperand,
		/* 0x06 */ &Translator::TranslateModRMOperand,
		/* 0x07 */ &Translator::TranslateNoOperand
	};

	Translator::TranslateOperandFunc Translator::translateOperand[2][0x100] = {
		{
			/* 0x00 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x04 */ &Translator::TranslateOperandErr, &Translator::TranslateImm1632Operand, &Translator::TranslateOperandErr, &Translator::TranslateNoOperand,
			/* 0x08 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x0C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x10 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x14 */ &Translator::TranslateOperandErr, &Translator::TranslateImm1632Operand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x18 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x1C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x20 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x24 */ &Translator::TranslateOperandErr, &Translator::TranslateImm1632Operand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x28 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x2C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x30 */ &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand,
			/* 0x34 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x38 */ &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand,
			/* 0x3C */ &Translator::TranslateImm8Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x40 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x44 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x48 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x4C */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,

			/* 0x50 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x54 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x58 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x5C */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,

			/* 0x60 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x64 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x68 */ &Translator::TranslateImm1632Operand, 
			/* 0x69 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm1632Operand>,
			/* 0x6A */ &Translator::TranslateImm8Operand,
			/* 0x6B */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0x6C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x70 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x74 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x78 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x7C */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,

			/* 0x80 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>, 
			/* 0x81 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm1632Operand>,
			/* 0x82 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0x83 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0x84 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x88 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x8C */ &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x90 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x94 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x98 */ &Translator::TranslateOperandErr, &Translator::TranslateNoOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x9C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xA0 */ &Translator::TranslateOperandErr, &Translator::TranslateImm1632Operand, &Translator::TranslateOperandErr, &Translator::TranslateImm1632Operand,
			/* 0xA4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA8 */ &Translator::TranslateImm8Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateModRMOperand, &Translator::TranslateNoOperand,
			/* 0xAC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xB0 */ &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand,
			/* 0xB4 */ &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand, &Translator::TranslateImm8Operand,
			/* 0xB8 */ &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand,
			/* 0xBC */ &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand, &Translator::TranslateImm1632Operand,

			/* 0xC0 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0xC1 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0xC2 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0xC4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, 
			/* 0xC6 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0xC7 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm1632Operand>,
			/* 0xC8 */ &Translator::TranslateOperandErr, &Translator::TranslateNoOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xCC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xD0 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0xD4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xD8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xDC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xE0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xE4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xE8 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateOperandErr, &Translator::TranslateNoOperand,
			/* 0xEC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xF0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xF4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, 
			/* 0xF6 */ &Translator::TranslateExtOperand<translateOperand0xF6>,
			/* 0xF7 */ &Translator::TranslateExtOperand<translateOperand0xF7>,
			/* 0xF8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xFC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, 
			/* 0xFF */ &Translator::TranslateExtOperand<translateOperand0xFF>
		},{
			/* 0x00 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x04 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x08 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x0C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x10 */ &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x14 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x18 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x1C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x20 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x24 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x28 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x2C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x30 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x34 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x38 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x3C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x40 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x44 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x48 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x4C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x50 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x54 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x58 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x5C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x60 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x64 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x68 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x6C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr,

			/* 0x70 */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0x71 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x74 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x78 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x7C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand,

			/* 0x80 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x84 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x88 */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,
			/* 0x8C */ &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand, &Translator::TranslateNoOperand,

			/* 0x90 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x94 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x98 */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0x9C */ &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,

			/* 0xA0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand,
			/* 0xAC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand,

			/* 0xB0 */ &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xB4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateModRMOperand,
			/* 0xB8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, 
			/* 0xBA */ &Translator::TranslateAggOperand<&Translator::TranslateModRMOperand, &Translator::TranslateImm8Operand>,
			/* 0xBB */ &Translator::TranslateOperandErr,
			/* 0xBC */ &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr, &Translator::TranslateModRMOperand, &Translator::TranslateOperandErr,

			/* 0xC0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xC4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xC8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xCC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xD0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xD4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xD8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xDC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xE0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xE4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xE8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xEC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xF0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xF4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xF8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xFC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr
		}
	};
};