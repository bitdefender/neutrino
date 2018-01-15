#ifndef _NEUTRINO_STRATEGY_TUPLE_H_
#define _NEUTRINO_STRATEGY_TUPLE_H_

#include "Neutrino.Types.h"
#include "Neutrino.Translator.h"
#include "Neutrino.Result.h"

namespace Neutrino {
	class TupleOutput : public AbstractResult {
	public:
		BYTE tuple[(1 << 16) + 4];
		UINTPTR lastHash;
		UINTPTR lastBlock;
	};

	class TupleStrategy {
	private:
		WORD Hash(UINTPTR dest);
		TupleOutput out;
		UINTPTR regPtr1, regPtr2;
	public:
		TupleStrategy();

		void Reset();
		bool TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);
		bool TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state);
		void TouchDeferred();

		AbstractResult *GetResult();

		void PushBasicBlock(UINTPTR bb);
		UINTPTR LastBasicBlock() const;
	};
};

#endif