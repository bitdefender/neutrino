#ifndef _NEUTRINO_STRATEGY_TRACE_X86_32_H_
#define _NEUTRINO_STRATEGY_TRACE_X86_32_H_

#include "Neutrino.Types.h"
#include "Neutrino.Abstract.Translator.h"
#include "Neutrino.Result.h"

#include "Neutrino.Strategy.Trace.h"

namespace Neutrino {

	class TraceStrategyX8632 : public TraceStrategy<TraceStrategyX8632> {
	private:
		UINTPTR regEax;
	public:
		bool TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);
		bool TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state);
	};
};

#endif