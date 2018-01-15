#ifndef _NEUTRINO_QUEUE_HPP_
#define _NEUTRINO_QUEUE_HPP_

#include <utility>

namespace Neutrino {
	template<typename T, int SZ>
	inline Queue<T, SZ>::Queue() {
		head = tail = 0;
	}

	template<typename T, int SZ>
	inline bool Queue<T, SZ>::IsEmpty() const {
		return head == tail;
	}

	template<typename T, int SZ>
	inline bool Queue<T, SZ>::IsFull() const {
		int nHead = (head + 1) % SZ;
		return nHead == tail;
	}

	template<typename T, int SZ>
	inline bool Queue<T, SZ>::Enqueue(T &input) {
		int nHead = (head + 1) % SZ;
		if (nHead == tail) {
			return false;
		}

		data[head] = std::move(input);
		head = nHead;
		return true;
	}

	template<typename T, int SZ>
	inline bool Queue<T, SZ>::Dequeue(T &output) {
		if (IsEmpty()) {
			return false;
		}

		output = std::move(data[tail]);
		tail = (tail + 1) % SZ;
		return true;
	}
};


#endif
