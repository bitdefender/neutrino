#ifndef _NEUTRINO_TRANSLATOR_X86_BASE_H_
#define _NEUTRINO_TRANSLATOR_X86_BASE_H_

#include "Neutrino.Abstract.Translator.h"
#include "Neutrino.Result.h"
#include "Neutrino.Util.h"

#include <initializer_list>

namespace Neutrino {
	class TranslationTableX86Base {
	public :
		typedef int (TranslationTableX86Base::*OpcodeFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		typedef void (TranslationTableX86Base::*OperandFunc)(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

	private :
		void InitOpcodes(std::initializer_list<std::initializer_list<OpcodeFunc> > opcodes);
		void InitOperands(OperandFunc mrm, std::initializer_list<std::initializer_list<OperandFunc> > operands);
	public :
		
		OpcodeFunc opcodeTbl[2][0x100]; 
		OperandFunc operandTbl[2][0x100];
		OperandFunc modRmFunc;
		
		/* Opcodes */
		int OpcodeErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeNull(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeOverride(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeDefault(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <DWORD flag>
		int OpcodePrefix(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			CopyBytes<1>(pIn, pOut, szOut);
			state.flags |= flag;
			state.pfxCount++;
			return PARSED_PREFIX;
		}

		template <OpcodeFunc *funcs>
		int OpcodeExt(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			return (this->*funcs[state.subOpcode])(pIn, pOut, szOut, state);
		}

		/* Operands */
		void OperandErr(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandNull(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandOverride(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandNone(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandModRm(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandImm8(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandImm16(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandImm1632(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		void OperandImm32(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <OperandFunc f1, OperandFunc f2>
		void OperandAgg(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			(this->*f1)(pIn, pOut, szOut, state);
			(this->*f2)(pIn, pOut, szOut, state);
		}

		template <OperandFunc *funcs>
		void OperandExt(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state) {
			(this->*funcs[state.subOpcode])(pIn, pOut, szOut, state);
		}

		/* Constructors */
		TranslationTableX86Base();
		TranslationTableX86Base(OperandFunc mrm, std::initializer_list<std::initializer_list<OpcodeFunc> > opcodes, std::initializer_list<std::initializer_list<OperandFunc> > operands);
		TranslationTableX86Base(const TranslationTableX86Base &base, const TranslationTableX86Base &over);
	};


	extern TranslationTableX86Base tableX86Base;

	template <typename TABLE>
	class Translator {
	private :
		static TABLE table;
	public:
		virtual void Translate(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		
		static DWORD GetCodeMemSize();
		void SetCodeMem(BYTE *ptr);

		void Reset();
		AbstractResult *GetResult();

		void PushBasicBlock(UINTPTR bb);
		UINTPTR LastBasicBlock() const;

		void TouchDeferred();
	};
};

#include "Neutrino.Translator.X86.Base.hpp"

#endif
