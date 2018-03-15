#ifndef _NEUTRINO_STRATEGY_TRACE_X86_64_H_
#define _NEUTRINO_STRATEGY_TRACE_X86_64_H_

#include "Neutrino.Types.h"
#include "Neutrino.Abstract.Translator.h"
#include "Neutrino.Result.h"

#include "Neutrino.Strategy.Trace.h"

namespace Neutrino {

	class TraceStrategyX8664 : public TraceStrategy<TraceStrategyX8664> {
	private:
		UINTPTR regRax;
		UINTPTR regRcx;
	public:
		bool TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);
		bool TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state);
	};
};

#endif