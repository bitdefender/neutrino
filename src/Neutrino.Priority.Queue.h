#ifndef _NEUTRINO_PRIORITY_QUEUE_H_
#define _NEUTRINO_PRIORITY_QUEUE_H_

#include <utility>

namespace Neutrino {

	template <typename T, int SZ>
	class PriorityQueue {
	private:
		using Elem = std::pair<double, T>;
		Elem data[SZ];
		int size;

		int Sift(const double priority, int index = 0);
	public:
		PriorityQueue();
		~PriorityQueue();

		bool IsEmpty() const;
		bool IsFull() const;
		int Count() const;

		bool Enqueue(const double priority, T &input);
		bool Dequeue(double &priority, T &output);

		bool Top(T &output) const;
		bool ChangeTopPriority(const double newPriority);

		void Clear();
	};

};

#include "Neutrino.Priority.Queue.hpp"


#endif
