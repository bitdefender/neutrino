#include "Neutrino.Translator.X86.Base.h"

#include <cstdio>

namespace Neutrino {

#define OPCODE_NULL &TranslationTableX86Base::OpcodeNull
#define OPCODE_ERROR &TranslationTableX86Base::OpcodeErr
#define OPCODE_DEFAULT &TranslationTableX86Base::OpcodeDefault
#define OPCODE_OVERRIDE &TranslationTableX86Base::OpcodeOverride
#define OPCODE_PFX(flag) &TranslationTableX86Base::OpcodePrefix<(flag)>
#define OPCODE_EXT(s) &TranslationTableX86Base::OpcodeExt< s >

#define OPERAND_ERROR &TranslationTableX86Base::OperandErr
#define OPERAND_NONE &TranslationTableX86Base::OperandNone
#define OPERAND_NULL &TranslationTableX86Base::OperandNull
#define OPERAND_MODRM &TranslationTableX86Base::OperandModRm
#define OPERAND_IMM8 &TranslationTableX86Base::OperandImm8
#define OPERAND_IMM16 &TranslationTableX86Base::OperandImm16
#define OPERAND_IMM1632 &TranslationTableX86Base::OperandImm1632
#define OPERAND_AGG(o1, o2) &TranslationTableX86Base::OperandAgg<(o1), (o2)>
#define OPERAND_EXT(s) &TranslationTableX86Base::OperandExt< s >

	void TranslationTableX86Base::InitOpcodes(std::initializer_list<std::initializer_list<OpcodeFunc> > opcodes) {
		int i = 0;
		for (auto itRow = opcodes.begin(); (opcodes.end() != itRow) && (i < 2); ++i, ++itRow) {
			int j = 0;
			for (auto itCol = itRow->begin(); (itRow->end() != itCol) & (j < 0x100); ++j, ++itCol) {
				opcodeTbl[i][j] = *itCol;
			}

			for (; j < 0x100; ++j) {
				opcodeTbl[i][j] = OPCODE_NULL;
			}
		}

		for (; i < 2; ++i) {
			for (int j = 0; j < 0x100; ++j) {
				opcodeTbl[i][j] = OPCODE_NULL;
			}
		}
	}
	
	void TranslationTableX86Base::InitOperands(OperandFunc mrm, std::initializer_list<std::initializer_list<OperandFunc>> operands) {
		modRmFunc = mrm;

		int i = 0;
		for (auto itRow = operands.begin(); (operands.end() != itRow) && (i < 2); ++i, ++itRow) {
			int j = 0;
			for (auto itCol = itRow->begin(); (itRow->end() != itCol) & (j < 0x100); ++j, ++itCol) {
				operandTbl[i][j] = *itCol;
			}

			for (; j < 0x100; ++j) {
				operandTbl[i][j] = OPERAND_NULL;
			}
		}

		for (; i < 2; ++i) {
			for (int j = 0; j < 0x100; ++j) {
				operandTbl[i][j] = OPERAND_NULL;
			}
		}
	}
	
	/* Opcodes */

	inline int TranslationTableX86Base::OpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
		return 0;
	}

	inline int TranslationTableX86Base::OpcodeNull(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
		return 0;
	}

	inline int TranslationTableX86Base::OpcodeOverride(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
		return 0;
	}

	inline int TranslationTableX86Base::OpcodeDefault(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		CopyBytes<1>(pIn, pOut, szOut);
		return PARSED_OPCODE;
	}

	/* Operands */

	void TranslationTableX86Base::OperandErr(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state)
	{
	}

	inline void TranslationTableX86Base::OperandNull(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		DEBUG_BREAK;
	}

	void TranslationTableX86Base::OperandOverride(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		DEBUG_BREAK;
	}

	void TranslationTableX86Base::OperandNone(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
	}

	inline void TranslationTableX86Base::OperandModRm(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
		(this->*modRmFunc)(pIn, pOut, szOut, state);
	}

	void TranslationTableX86Base::OperandImm8(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		CopyBytes<1>(pIn, pOut, szOut);
	}

	void TranslationTableX86Base::OperandImm16(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		CopyBytes<2>(pIn, pOut, szOut);
	}

	void TranslationTableX86Base::OperandImm1632(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		if (state.flags & FLAG_O16) {
			CopyBytes<2>(pIn, pOut, szOut);
		}
		else {
			CopyBytes<4>(pIn, pOut, szOut);
		}
	}

	void TranslationTableX86Base::OperandImm32(const BYTE *& pIn, BYTE *& pOut, int & szOut, TranslationState & state) {
		CopyBytes<4>(pIn, pOut, szOut);
	}

	TranslationTableX86Base::TranslationTableX86Base() {
		modRmFunc = &TranslationTableX86Base::OperandNull;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 0x100; ++j) {
				opcodeTbl[i][j] = OPCODE_NULL;
			}
		}
	}

	TranslationTableX86Base::TranslationTableX86Base(OperandFunc mrm, std::initializer_list<std::initializer_list<OpcodeFunc> > opcodes, std::initializer_list<std::initializer_list<OperandFunc> > operands) {
		InitOpcodes(opcodes);
		InitOperands(mrm, operands);
	}

	TranslationTableX86Base::TranslationTableX86Base(const TranslationTableX86Base &base, const TranslationTableX86Base &over) {
		modRmFunc = over.modRmFunc;
		if (&TranslationTableX86Base::OperandNull == modRmFunc) {
			modRmFunc = base.modRmFunc;
		}

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 0x100; ++j) {
				opcodeTbl[i][j] = over.opcodeTbl[i][j];

				if (OPCODE_NULL == opcodeTbl[i][j]) {
					opcodeTbl[i][j] = base.opcodeTbl[i][j];
				}

				if (nullptr == opcodeTbl[i][j]) {
					DEBUG_BREAK;
				}
			}
		}

		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 0x100; ++j) {
				operandTbl[i][j] = over.operandTbl[i][j];

				if (&TranslationTableX86Base::OperandNull == operandTbl[i][j]) {
					operandTbl[i][j] = base.operandTbl[i][j];
				}

				if (nullptr == operandTbl[i][j]) {
					DEBUG_BREAK;
				}
			}
		}
	}


	TranslationTableX86Base::OpcodeFunc opcode0xFF[8] = {
		/* 0x00 */ OPCODE_DEFAULT,
		/* 0x01 */ OPCODE_DEFAULT,
		/* 0x02 */ OPCODE_OVERRIDE, //TranslateCallModRM,
		/* 0x03 */ OPCODE_ERROR,
		/* 0x04 */ OPCODE_OVERRIDE, //TranslateJumpModRM,
		/* 0x05 */ OPCODE_ERROR,
		/* 0x06 */ OPCODE_DEFAULT,
		/* 0x07 */ OPCODE_ERROR
	};

	TranslationTableX86Base::OperandFunc operand0xF6[8] = {
		/* 0x00 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
		/* 0x01 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
		/* 0x02 */ OPERAND_MODRM,
		/* 0x03 */ OPERAND_MODRM,
		/* 0x04 */ OPERAND_MODRM,
		/* 0x05 */ OPERAND_MODRM,
		/* 0x06 */ OPERAND_MODRM,
		/* 0x07 */ OPERAND_MODRM
	};

	TranslationTableX86Base::OperandFunc operand0xF7[8] = {
		/* 0x00 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM1632),
		/* 0x01 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM1632),
		/* 0x02 */ OPERAND_MODRM,
		/* 0x03 */ OPERAND_MODRM,
		/* 0x04 */ OPERAND_MODRM,
		/* 0x05 */ OPERAND_MODRM,
		/* 0x06 */ OPERAND_MODRM,
		/* 0x07 */ OPERAND_MODRM
	};

	TranslationTableX86Base::OperandFunc operand0xFF[8] = {
		/* 0x00 */ OPERAND_MODRM,
		/* 0x01 */ OPERAND_MODRM,
		/* 0x02 */ OPERAND_NONE,
		/* 0x03 */ OPERAND_NONE,
		/* 0x04 */ OPERAND_NONE,
		/* 0x05 */ OPERAND_NONE,
		/* 0x06 */ OPERAND_MODRM,
		/* 0x07 */ OPERAND_NONE
	};


	const TranslationTableX86Base &GetTableX86Base() {
		static TranslationTableX86Base instance(&TranslationTableX86Base::OperandOverride, 
		{ 
			{
				/* 0x00 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x04 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0x08 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x0C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_PFX(FLAG_EXT),

				/* 0x10 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x14 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x18 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x1C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x20 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x24 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x28 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x2C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x30 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x34 */ OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x38 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x3C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x40 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x44 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x48 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x4C */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,

				/* 0x50 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x54 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x58 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x5C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0x60 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x64 */ OPCODE_PFX(FLAG_FS), OPCODE_PFX(FLAG_GS), OPCODE_PFX(FLAG_O16), OPCODE_ERROR,
				/* 0x68 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x6C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x70 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x74 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x78 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x7C */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,

				/* 0x80 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x84 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0x88 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x8C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,

				/* 0x90 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x94 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x98 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0x9C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,

				/* 0xA0 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xA4 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xA8 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xAC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0xB0 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xB4 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xB8 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xBC */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0xC0 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0xC4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xC8 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xCC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0xD0 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xD4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xD8 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xDC */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,

				/* 0xE0 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xE4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xE8 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0xEC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0xF0 */ OPCODE_PFX(FLAG_LOCK), OPCODE_ERROR, OPCODE_PFX(FLAG_REPNZ), OPCODE_PFX(FLAG_REPZ),
				/* 0xF4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xF8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xFC */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xFF */ OPCODE_EXT(opcode0xFF)
			},{
				/* 0x00 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x04 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x08 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x0C */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x10 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0x14 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR,
				/* 0x18 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x1C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,

				/* 0x20 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x24 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x28 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR,
				/* 0x2C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,

				/* 0x30 */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x34 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x38 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x3C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0x40 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x44 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x48 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x4C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0x50 */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x54 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x58 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x5C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0x60 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR,
				/* 0x64 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x68 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x6C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0x70 */ OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x74 */ OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x78 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0x7C */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0x80 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x84 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x88 */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,
				/* 0x8C */ OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE, OPCODE_OVERRIDE,

				/* 0x90 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x94 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x98 */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0x9C */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0xA0 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xA4 */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xA8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xAC */ OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,

				/* 0xB0 */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xB4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xB8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR,
				/* 0xBC */ OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_DEFAULT, OPCODE_ERROR,

				/* 0xC0 */ OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xC4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xC8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xCC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0xD0 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xD4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT, OPCODE_DEFAULT,
				/* 0xD8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xDC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,

				/* 0xE0 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xE4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xE8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,
				/* 0xEC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_DEFAULT,

				/* 0xF0 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xF4 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xF8 */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR,
				/* 0xFC */ OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR, OPCODE_ERROR
			} 
		}, { 
			{
				/* 0x00 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x04 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_NONE,
				/* 0x08 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x0C */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x10 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x14 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x18 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x1C */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_MODRM,

				/* 0x20 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x24 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x28 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x2C */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x30 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x34 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x38 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x3C */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x40 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x44 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x48 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x4C */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,

				/* 0x50 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x54 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x58 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x5C */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,

				/* 0x60 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x64 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x68 */ OPERAND_IMM1632,
				/* 0x69 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM1632),
				/* 0x6A */ OPERAND_IMM8,
				/* 0x6B */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0x6C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x70 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x74 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x78 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x7C */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,

				/* 0x80 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0x81 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM1632),
				/* 0x82 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0x83 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0x84 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,
				/* 0x88 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x8C */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,

				/* 0x90 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x94 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x98 */ OPERAND_NONE, OPERAND_NONE, OPERAND_ERROR, OPERAND_NONE,
				/* 0x9C */ OPERAND_NONE, OPERAND_NONE, OPERAND_ERROR, OPERAND_ERROR,

				/* 0xA0 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_ERROR, OPERAND_IMM1632,
				/* 0xA4 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0xA8 */ OPERAND_IMM8, OPERAND_IMM1632, OPERAND_NONE, OPERAND_NONE,
				/* 0xAC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_NONE, OPERAND_NONE,

				/* 0xB0 */ OPERAND_IMM8, OPERAND_IMM8, OPERAND_IMM8, OPERAND_IMM8,
				/* 0xB4 */ OPERAND_IMM8, OPERAND_IMM8, OPERAND_IMM8, OPERAND_IMM8,
				/* 0xB8 */ OPERAND_IMM1632, OPERAND_IMM1632, OPERAND_IMM1632, OPERAND_IMM1632,
				/* 0xBC */ OPERAND_IMM1632, OPERAND_IMM1632, OPERAND_IMM1632, OPERAND_IMM1632,

				/* 0xC0 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0xC1 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0xC2 */ OPERAND_NONE, OPERAND_NONE,
				/* 0xC4 */ OPERAND_NONE, OPERAND_NONE,
				/* 0xC6 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0xC7 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM1632),
				/* 0xC8 */ OPERAND_AGG(OPERAND_IMM16, OPERAND_IMM8),
				/* 0xC9 */ OPERAND_NONE, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xCC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0xD0 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0xD4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xD8 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xDC */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,

				/* 0xE0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xE4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xE8 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0xEC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0xF0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xF4 */ OPERAND_ERROR, OPERAND_ERROR,
				/* 0xF6 */ OPERAND_EXT(operand0xF6),
				/* 0xF7 */ OPERAND_EXT(operand0xF7),
				/* 0xF8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xFC */ OPERAND_NONE, OPERAND_NONE, OPERAND_MODRM,
				/* 0xFF */ OPERAND_EXT(operand0xFF)
			}, {
				/* 0x00 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x04 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x08 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x0C */ OPERAND_ERROR, OPERAND_MODRM, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x10 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,
				/* 0x14 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM, OPERAND_ERROR,
				/* 0x18 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x1C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,

				/* 0x20 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x24 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x28 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR,
				/* 0x2C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,

				/* 0x30 */ OPERAND_ERROR, OPERAND_NONE, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x34 */ OPERAND_NONE, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x38 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x3C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0x40 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x44 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x48 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x4C */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,

				/* 0x50 */ OPERAND_ERROR, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x54 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x58 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x5C */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,

				/* 0x60 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR,
				/* 0x64 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x68 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x6C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM, OPERAND_MODRM,

				/* 0x70 */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0x71 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x74 */ OPERAND_MODRM, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x78 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0x7C */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM, OPERAND_MODRM,

				/* 0x80 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x84 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x88 */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,
				/* 0x8C */ OPERAND_NONE, OPERAND_NONE, OPERAND_NONE, OPERAND_NONE,

				/* 0x90 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x94 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x98 */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,
				/* 0x9C */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM,

				/* 0xA0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xA4 */ OPERAND_ERROR, OPERAND_MODRM, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xA8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xAC */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0xAD */ OPERAND_ERROR,
				/* 0xAE */ OPERAND_MODRM,
				/* 0xAF */ OPERAND_MODRM,

				/* 0xB0 */ OPERAND_ERROR, OPERAND_MODRM, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xB4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM, OPERAND_MODRM,
				/* 0xB8 */ OPERAND_ERROR, OPERAND_ERROR,
				/* 0xBA */ OPERAND_AGG(OPERAND_MODRM, OPERAND_IMM8),
				/* 0xBB */ OPERAND_ERROR,
				/* 0xBC */ OPERAND_MODRM, OPERAND_MODRM, OPERAND_MODRM, OPERAND_ERROR,

				/* 0xC0 */ OPERAND_ERROR, OPERAND_MODRM, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xC4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xC8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xCC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0xD0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xD4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM, OPERAND_MODRM,
				/* 0xD8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xDC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,

				/* 0xE0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xE4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xE8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,
				/* 0xEC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_MODRM,

				/* 0xF0 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xF4 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xF8 */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR,
				/* 0xFC */ OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR, OPERAND_ERROR
			}
		});
		return instance;
	}
};
