#ifndef _NEUTRINO_SIMULATION_TRACE_H_
#define _NEUTRINO_SIMULATION_TRACE_H_

#include "Neutrino.Types.h"
#include "Neutrino.Result.h"

#include "Neutrino.Environment.h"
#include "Neutrino.Strategy.Trace.X86.32.h"

namespace Neutrino {

	class SimulationTraceEnvironment : public AbstractEnvironment {
	private:
		TraceOutput out;

		struct Node {
			UINTPTR addr;
			int parent;
			int rank;

			bool found;

			std::vector<int> next;
		};

		std::vector<Node> graph;
		int coverage;

		int GetSet(int a);
		bool Union(int a, int b);
		void GenerateGraph();


	public :
		SimulationTraceEnvironment();

		virtual void InitExec(UINTPTR entry);
		virtual void Go(unsigned char *buffer, unsigned int size);
		virtual AbstractResult *GetResult();
		virtual int GetCoverage();
	};
};

#endif
