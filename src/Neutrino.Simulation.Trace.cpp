#include "Neutrino.Simulation.Trace.h"

#include <cstdlib>
#include <cstdio>

namespace Neutrino {
	int SimulationTraceEnvironment::GetSet(int a) {
		if (graph[a].parent == a) {
			return a;
		}

		return graph[a].parent = GetSet(graph[a].parent);
	}

	bool SimulationTraceEnvironment::Union(int f, int t) {
		int pa = GetSet(f);
		int pb = GetSet(t);

		if (pa == pb) {
			return false;
		}

		if (graph[pa].rank < graph[pb].rank) {
			graph[pa].parent = pb;
		} else if (graph[pa].rank > graph[pb].rank) {
			graph[pb].parent = pa;
		} else {
			graph[pa].parent = pb;
			graph[pb].rank++;
		}

		graph[f].next.push_back(t);
		return true;
	}

	void SimulationTraceEnvironment::GenerateGraph() {
		int nCount = 130 + (rand() % 70);
		int nRets = 5 + (rand() % 5);

		graph.resize(nCount);

		for (int i = 0; i < nCount; ++i) {
			graph[i].rank = 1;
			graph[i].parent = i;
			graph[i].found = false;
			graph[i].addr = (i << 12) | (rand() & 0xFFF);
		}

		int e = 0;
		// make sure the cfg is connected
		while (e < nCount - 1) {
			int f = rand() % (nCount - nRets);
			int t = 1 + rand() % (nCount - 1);

			if (Union(f, t)) {
				e++;
			}
		}

		// add some additional cross and back edges
		int r = (nCount >> 1) + rand() % (nCount);
		for (int i = 0; i < r; ++i) {
			int f = rand() % (nCount - nRets);
			int t = 1 + rand() % (nCount - 1);

			graph[f].next.push_back(t);
		}

		for (int i = 0; i < nCount; ++i) {
			printf("%d: ", i);

			for (auto it : graph[i].next) {
				printf("%d ", it);
			}

			printf("\n");
		}
	}

	SimulationTraceEnvironment::SimulationTraceEnvironment() {
		coverage = 0;
		GenerateGraph();
	}

	void SimulationTraceEnvironment::InitExec(UINTPTR entry) {
	}

	void SimulationTraceEnvironment::Go(unsigned char *buffer, unsigned int size) {
		int t = 0;

		out.traceIndex = 0;

		do {
			if (false == graph[t].found) {
				coverage++;
				graph[t].found = true;
			}

			out.trace[out.traceIndex] = graph[t].addr;
			out.traceIndex++;

			if (0 == graph[t].next.size()) {
				break;
			}

			int r = rand() % graph[t].next.size();
			t = graph[t].next[r];
		} while (true);
	}

	AbstractResult *SimulationTraceEnvironment::GetResult() {
		return &out;
	}

	int SimulationTraceEnvironment::GetCoverage() {
		return coverage;
	}

};