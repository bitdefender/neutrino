#include "Neutrino.Definitions.h"

namespace Neutrino {
	template <>
	TranslationTableX8632<TraceStrategy<TraceStrategyX8632> > Translator<TranslationTableX8632<TraceStrategy<TraceStrategyX8632> > >::table(GetTableX86Base());
	
	template <>
	TranslationTableX8632<TupleStrategy> Translator<TranslationTableX8632<TupleStrategy> >::table(GetTableX86Base());

	template <>
	TranslationTableX8664<TraceStrategy<TraceStrategyX8664> > Translator<TranslationTableX8664<TraceStrategy<TraceStrategyX8664> > >::table(GetTableX86Base());

	template <>
	TranslationTableX8664<TupleStrategy> Translator<TranslationTableX8664<TupleStrategy> >::table(GetTableX86Base());


#ifdef _M_X64 
	template class Environment<Translator<TranslationTableX8664<TraceStrategy<TraceStrategyX8664> > >, TrampolineX8664, System >;
	template class Environment<Translator<TranslationTableX8664<TupleStrategy> >, TrampolineX8664, System >;
#else
	template class Environment<Translator<TranslationTableX8632<TraceStrategy<TraceStrategyX8632> > >, TrampolineX8632, System >;
	template class Environment<Translator<TranslationTableX8632<TupleStrategy> >, TrampolineX8632, System >;
#endif

};

#ifdef _M_X64 
	Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8664<Neutrino::TraceStrategy<Neutrino::TraceStrategyX8664> > >, Neutrino::TrampolineX8664, Neutrino::System > traceEnvironment;
	Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8664<Neutrino::TupleStrategy> >, Neutrino::TrampolineX8664, Neutrino::System > tupleEnvironment;
#else
	Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8632<Neutrino::TraceStrategy<Neutrino::TraceStrategyX8632> > >, Neutrino::TrampolineX8632, Neutrino::System > traceEnvironment;
	Neutrino::Environment<Neutrino::Translator<Neutrino::TranslationTableX8632<Neutrino::TupleStrategy> >, Neutrino::TrampolineX8632, Neutrino::System > tupleEnvironment;
#endif