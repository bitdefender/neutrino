#ifndef _NEUTRINO_FAIR_QUEUE_H_
#define _NEUTRINO_FAIR_QUEUE_H_

#include <vector>

#include "Neutrino.Queue.h"

namespace Neutrino {

	template <typename T, int SZ>
	class FairQueue {
	private:
		std::vector<Queue<T, SZ> > queues;
		int cQueue, qCount;
	public:
		FairQueue(int buckets);
		~FairQueue();

		bool IsEmpty() const;
		bool IsFull() const;
		bool IsFull(int bucket) const;

		bool Enqueue(int bucket, T &input);
		bool Dequeue(T &output);

		void Clear();
	};

};

#include "Neutrino.Fair.Queue.hpp"


#endif
