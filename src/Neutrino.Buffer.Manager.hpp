#include "Neutrino.Buffer.Manager.h"
#include "Neutrino.Memory.h"

namespace Neutrino {

	template<int LOG_SIZE>
	MemoryPool<LOG_SIZE>::MemoryPool() {
		data = (unsigned char *)::Neutrino::Alloc(nullptr, 1 << LOG_POOL_SIZE, PAGE_PROTECTION_WRITE | PAGE_PROTECTION_READ, 0, -1);

		freeCount = 0;

		if (nullptr == data) {
			return;
		}

		freeCount = sizeof(free) / sizeof(free[0]);
		for (unsigned int i = 0; i < freeCount; ++i) {
			free[i] = &data[i * (1 << LOG_SIZE)];
		}
	}

	template<int LOG_SIZE>
	MemoryPool<LOG_SIZE>::~MemoryPool() {
		::Neutrino::Free(data, 1 << LOG_POOL_SIZE);
	}

	template<int LOG_SIZE>
	unsigned char * MemoryPool<LOG_SIZE>::Alloc() {
		if (0 == freeCount) {
			return false;
		}

		freeCount -= 1;
		return free[freeCount];
	}

	template<int LOG_SIZE>
	void MemoryPool<LOG_SIZE>::Free(unsigned char *ptr) {
		free[freeCount] = ptr;
		freeCount++;
	}

	template<int LOG_SIZE>
	bool MemoryPool<LOG_SIZE>::Owns(unsigned char *ptr) const {
		return (data <= ptr) && (ptr < data + (1 << LOG_POOL_SIZE));
	}
		
};
