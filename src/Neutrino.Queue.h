#ifndef _NEUTRINO_QUEUE_H_
#define _NEUTRINO_QUEUE_H_

namespace Neutrino {
	template <typename T, int SZ>
	class Queue {
	private:
		T data[SZ];
		int head, tail;
	public:
		Queue();

		bool IsEmpty() const;
		bool IsFull() const;

		bool Enqueue(T &input);
		bool Dequeue(T &output);
	};
};

#include "Neutrino.Queue.hpp"

#endif