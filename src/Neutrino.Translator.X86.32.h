#ifndef _NEUTRINO_TRANSLATOR_X86_32_H_
#define _NEUTRINO_TRANSLATOR_X86_32_H_

#include "Neutrino.Translator.X86.Base.h"

namespace Neutrino {

	template <typename STRATEGY>
	class TranslationTableX8632 : public TranslationTableX86Base, STRATEGY {
		using STRATEGY::TouchStatic;
		using STRATEGY::TouchDynamic;

	private :
		static OpcodeFunc opcode0xFF[8];

	public :
		using STRATEGY::TouchDeferred;
		using STRATEGY::PushBasicBlock;
		using STRATEGY::LastBasicBlock;
		using STRATEGY::Reset;
		using STRATEGY::GetResult;

		static DWORD GetCodeMemSize();
		void SetCodeMem(BYTE *ptr);

		/* Opcodes */
		template <BYTE destSize>
		int OpcodeJmp(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		template <BYTE destSize>
		int OpcodeJxx(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		int OpcodeCallModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeJumpModRM(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		int OpcodeFarJump(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		int OpcodeCall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeRet(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);
		int OpcodeRetn(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		int OpcodeSyscall(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);		

		/* Operands */
		void OperandModRm(const BYTE *&pIn, BYTE *&pOut, int &szOut, TranslationState &state);

		/* Constructor */
		TranslationTableX8632(TranslationTableX86Base base);
	};

};

#include "Neutrino.Translator.X86.32.hpp"

#endif
