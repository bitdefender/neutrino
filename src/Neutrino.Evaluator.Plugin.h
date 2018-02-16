#ifndef _NEUTRINO_EVALUATOR_PLUGIN_H_
#define _NEUTRINO_EVALUATOR_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.Test.h"
#include "Neutrino.Result.h"
#include "Neutrino.Enum.Set.h"

namespace Neutrino {
	class EvaluatorPlugin : public Plugin {
	public:
		virtual const EnumSet<ResultType> *GetInputType() const = 0;
		virtual double Evaluate(const Test &test, const AbstractResult *result) = 0;
	};
};


#endif
