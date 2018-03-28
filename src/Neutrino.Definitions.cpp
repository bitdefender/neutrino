#include "Neutrino.Translator.X86.Base.h"

#include "Neutrino.Translator.X86.32.h"
#include "Neutrino.Translator.X86.64.h"

#include "Neutrino.Strategy.Trace.h"

#include "Neutrino.Strategy.Trace.X86.32.h"
#include "Neutrino.Strategy.Trace.X86.64.h"

#include "Neutrino.Strategy.Tuple.h"

namespace Neutrino {
	template <>
	TranslationTableX8632<TraceStrategy<TraceStrategyX8632> > Translator<TranslationTableX8632<TraceStrategy<TraceStrategyX8632> > >::table(tableX86Base);
	
	template <>
	TranslationTableX8632<TupleStrategy> Translator<TranslationTableX8632<TupleStrategy> >::table(tableX86Base);

	template <>
	TranslationTableX8664<TraceStrategy<TraceStrategyX8664> > Translator<TranslationTableX8664<TraceStrategy<TraceStrategyX8664> > >::table(tableX86Base);

	template <>
	TranslationTableX8664<TupleStrategy> Translator<TranslationTableX8664<TupleStrategy> >::table(tableX86Base);
};
