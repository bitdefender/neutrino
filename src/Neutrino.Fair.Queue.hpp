#include "Neutrino.Fair.Queue.h"
#ifndef _NEUTRINO_FAIR_QUEUE_HPP_
#define _NEUTRINO_FAIR_QUEUE_HPP_

namespace Neutrino {
	template<typename T, int SZ>
	inline FairQueue<T, SZ>::FairQueue(int buckets) : queues(buckets) {
		qCount = buckets;
		cQueue = 0;
	}

	template<typename T, int SZ>
	inline bool FairQueue<T, SZ>::IsEmpty() const {
		for (auto &it : queues) {
			if (!it.IsEmpty()) {
				return false;
			}
		}

		return true;
	}

	template<typename T, int SZ>
	inline bool FairQueue<T, SZ>::IsFull() const {
		for (auto &it : queues) {
			if (!it.IsFull()) {
				return false;
			}
		}

		return true;
	}

	template<typename T, int SZ>
	inline bool FairQueue<T, SZ>::IsFull(int bucket) const {
		return queues[bucket].IsFull();
	}

	template<typename T, int SZ>
	inline bool FairQueue<T, SZ>::Enqueue(int bucket, T &input) {
		return queues[bucket].Enqueue(input);
	}

	template<typename T, int SZ>
	inline bool FairQueue<T, SZ>::Dequeue(T &output) {
		if (IsEmpty()) {
			return false;
		}

		while (!queues[cQueue].Dequeue(output)) {
			cQueue++;
			cQueue %= qCount;
		}

		cQueue++;
		cQueue %= qCount;
		return true;
	}
};

#endif
