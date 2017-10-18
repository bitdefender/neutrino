#include "Neutrino.Translator.h"

namespace Neutrino {

	void Translator::Translate(BYTE *&pIn, BYTE *&pOut, InstructionState &state) {
		state.flags = 0;

		DWORD ret, tbl;
		do {
			tbl = (state.flags & FLAG_EXT) ? 1 : 0;
			state.opCode = pIn[0];
			ret = (this->*translateOpcode[tbl][state.opCode])(pIn, pOut, state);
		} while (ret != PARSED_OPCODE);

		tbl = (state.flags & FLAG_EXT) ? 1 : 0;
		(this->*translateOperand[tbl][state.opCode])(pIn, pOut, state);
	}


	DWORD Translator::TranslateOpcodeErr(BYTE *&pIn, BYTE *&pOut, InstructionState &state) {
		__asm int 3;
	}

	DWORD Translator::TranslateOpcode(BYTE *&pIn, BYTE *&pOut, InstructionState &state) {
		pOut[0] = pIn[0];
		pOut++; pIn++;
		return PARSED_OPCODE;
	}

	void Translator::TranslateOperandErr(BYTE *&pIn, BYTE *&pOut, InstructionState &state) {
		__asm int 3;
	}

	void Translator::TranslateNoOperand(BYTE *& pIn, BYTE *&pOut, InstructionState &state) {
		// nothing to copy
	}

	/* = Opcode translation table ================================================= */

	Translator::TranslateOpcodeFunc Translator::translateOpcode[2][0x100] = {
		{
			/* 0x00 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x04 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x08 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x0C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x10 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
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
			/* 0x6C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x70 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x74 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x78 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x7C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x80 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x84 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x88 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x8C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x90 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x94 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x98 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x9C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xA0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xAC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xB0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xB4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xB8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xBC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

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
		},{
			/* 0x00 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x04 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x08 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x0C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x10 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
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
			/* 0x6C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x70 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x74 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x78 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x7C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x80 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x84 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x88 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x8C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0x90 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x94 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x98 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0x9C */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xA0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xA8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xAC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

			/* 0xB0 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xB4 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xB8 */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,
			/* 0xBC */ &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr, &Translator::TranslateOpcodeErr,

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

	Translator::TranslateOperandFunc Translator::translateOperand[2][0x100] = {
		{
			/* 0x00 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x04 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x08 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x0C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x10 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
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
			/* 0x6C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x70 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x74 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x78 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x7C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x80 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x84 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x88 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x8C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x90 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x94 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x98 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x9C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xA0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xAC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xB0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xB4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xB8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xBC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

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
		},{
			/* 0x00 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x04 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x08 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x0C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x10 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
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
			/* 0x6C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x70 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x74 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x78 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x7C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x80 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x84 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x88 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x8C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0x90 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x94 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x98 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0x9C */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xA0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xA8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xAC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

			/* 0xB0 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xB4 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xB8 */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,
			/* 0xBC */ &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr, &Translator::TranslateOperandErr,

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