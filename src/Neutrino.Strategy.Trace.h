#ifndef _NEUTRINO_STRATEGY_TRACE_H_
#define _NEUTRINO_STRATEGY_TRACE_H_

#include "Neutrino.Types.h"
#include "Neutrino.Abstract.Translator.h"
#include "Neutrino.Result.h"

namespace Neutrino {

	class TraceOutput : public AbstractResult {
	public :
		UINTPTR traceIndex;
		UINTPTR trace[1 << 26];
	};

	template <typename CODEGEN>
	class TraceStrategy {
	protected:
		TraceOutput out;
	public:
		TraceStrategy();

		void Reset();
		bool TouchStatic(BYTE *&pOut, int &szOut, TranslationState &state, UINTPTR dest);
		bool TouchDynamic(BYTE *&pOut, int &szOut, TranslationState &state);
		void TouchDeferred();

		AbstractResult *GetResult();

		void PushBasicBlock(UINTPTR bb);
		UINTPTR LastBasicBlock() const;
	};
};

#include "Neutrino.Strategy.Trace.hpp"

#endif