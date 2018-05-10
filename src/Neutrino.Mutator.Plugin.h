#ifndef _NEUTRINO_MUTATOR_PLUGIN_H_
#define _NEUTRINO_MUTATOR_PLUGIN_H_

#include "Neutrino.Plugin.h"
#include "Neutrino.Test.h"

#include <vector>
#include <memory>

namespace Neutrino {
	class TestSource {
	public :
		enum class ExtractSingleType {
			EXTRACT_BEST,
			EXTRACT_PROBABILISTIC,
			EXTRACT_RANDOM
		};

		enum class ExtractMultipleType {
			EXTRACT_BEST,
			EXTRACT_PROBABILISTIC,
			EXTRACT_STOCHASTIC,
			EXTRACT_TOURNAMENT, /**/
			EXTRACT_RANDOM
		};

		/* Returns the maximum amount of tests that can be extracted */
		virtual int GetAvailableTestCount() const = 0;

		/* Extract a single test from the queue */
		virtual bool GetSingleTest(ExtractSingleType eType, std::shared_ptr<Test> &test) = 0;

		/* Extract multiple tests from the queue */
		//virtual bool GetMultipleTests(ExtractType eType, std::vector<Test> &tests) = 0;
	};

	class TestDestination {
	public :
		/* Call this function to add new tests to the evaluation queue */
		virtual bool EnqueueTest(ExternalTest &test) = 0;

		/* Call this function to add old tests to the mutation queue */
		//virtual bool RequeueTest(double priority, Test &test) = 0;
	};

	class MutatorPlugin : public Plugin {
	public:
		virtual void SetSource(TestSource *src) = 0;
		virtual void SetDestination(TestDestination *dst) = 0;

		virtual bool Perform() = 0;
	};
};


#endif
