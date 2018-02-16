#ifndef _NEUTRINO_PRIORITY_QUEUE_HPP_
#define _NEUTRINO_PRIORITY_QUEUE_HPP_

#define PARENT(idx) (((idx) - 1) >> 1)
#define LCHILD(idx) (((idx) << 1) + 1)
#define RCHILD(idx) (((idx) << 1) + 2)

namespace Neutrino {
	template<typename T, int SZ>
	inline int PriorityQueue<T, SZ>::Sift(double priority, int index) {
		int sp = index;
		int spp = 0;
		double minP;

		do {
			spp = size;
			minP = priority;

			int tmp = LCHILD(sp);
			if ((tmp < size) && (data[tmp].first > minP)) {
				spp = tmp;
				minP = data[tmp].first;
			}

			tmp = RCHILD(sp);
			if ((tmp < size) && (data[tmp].first > data[spp].first)) {
				spp = tmp;
				minP = data[tmp].first;
			}

			if (spp == size) {
				break;
			}

			data[sp] = std::move(data[spp]);
			sp = spp;
		} while (true);

		return sp;
	}


	template<typename T, int SZ>
	inline PriorityQueue<T, SZ>::PriorityQueue() {
		size = 0;
	}

	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::IsEmpty() const {
		return 0 == size;
	}

	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::IsFull() const {
		return size == SZ;
	}

	template<typename T, int SZ>
	inline int PriorityQueue<T, SZ>::Count() const {
		return size;
	}

	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::Enqueue(const double priority, T &input) {
		if (IsFull()) {
			return false;
		}

		int sp = size, spp = PARENT(sp);
		size++;

		while ((sp > 0) && (priority > data[spp].first)) {
			data[sp] = std::move(data[spp]);
			sp = spp;
			spp = PARENT(sp);
		}

		data[sp].first = priority;
		data[sp].second = std::move(input); //= std::move(std::make_pair(priority, input));
		return true;
	}

	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::Dequeue(double &priority, T &output) {
		if (IsEmpty()) {
			return false;
		}

		priority = data[0].first;
		output = std::move(data[0].second);

		size--;
		int dest = Sift(data[size].first);

		data[dest] = std::move(data[size]);
		return true;
	}

	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::Top(T &output) const {
		if (IsEmpty()) {
			return false;
		}

		output = data[0].second;
		return true;
	}
	
	template<typename T, int SZ>
	inline bool PriorityQueue<T, SZ>::ChangeTopPriority(const double newPriority) {
		if (IsEmpty()) {
			return false;
		}

		double oldPriority = data[0].first;
		data[0].first = newPriority;

		if (oldPriority > newPriority) {
			return true;
		}

		Elem tmp = std::move(data[0]);
		int dest = Sift(newPriority);
		data[dest] = std::move(tmp);

		return true;
	}
};

#endif

