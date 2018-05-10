#include "Neutrino.Buffer.Manager.h"

#include "Neutrino.Bit.Hacks.h"

namespace Neutrino {

	const PoolAllocatorFunc BufferManager::allocators[32] = {
		AllocMemoryPool<0>, 	// 0
		AllocMemoryPool<1>,		// 1
		AllocMemoryPool<2>,		// 2
		AllocMemoryPool<3>,		// 3
		AllocMemoryPool<4>,		// 4
		AllocMemoryPool<5>,		// 5
		AllocMemoryPool<6>,		// 6
		AllocMemoryPool<7>,		// 7
		AllocMemoryPool<8>,		// 8
		AllocMemoryPool<9>,		// 9
		AllocMemoryPool<10>,	// 10
		AllocMemoryPool<11>,	// 11
		AllocMemoryPool<12>,	// 12
		AllocMemoryPool<13>,	// 13
		AllocMemoryPool<14>,	// 14
		AllocMemoryPool<15>,	// 15
		AllocMemoryPool<16>,	// 16
		AllocMemoryPool<17>,	// 17
		AllocMemoryPool<18>,	// 18
		AllocMemoryPool<19>,	// 19
		AllocMemoryPool<20>,	// 20
		AllocMemoryPool<21>,	// 21
		AllocMemoryPool<22>,	// 22
		AllocMemoryPool<23>,	// 23
		nullptr,				// 24
		nullptr,				// 25
		nullptr,				// 26
		nullptr,				// 27
		nullptr,				// 28
		nullptr,				// 29
		nullptr,				// 30
		nullptr					// 31
	};

	unsigned char *BufferManager::Alloc(unsigned int size) {
		unsigned int bucket = BinLog2(NextPow2(size));

		for (unsigned int i = 0; i < pools[bucket].size(); ++i) {
			unsigned char *ret = pools[bucket][i]->Alloc();

			if (nullptr != ret) {
				if (0 != i) {
					// lift to front
					AbstractMemoryPool *tmp = pools[bucket][0];
					pools[bucket][0] = pools[bucket][i];
					pools[bucket][i] = tmp;
				}

				return ret;
			}
		}

		AbstractMemoryPool *pool = allocators[bucket]();
		pools[bucket].push_back(pool);

		if (1 != pools[bucket].size()) {
			AbstractMemoryPool *tmp = pools[bucket][0];
			pools[bucket][0] = pools[bucket][pools[bucket].size() - 1];
			pools[bucket][pools[bucket].size() - 1] = tmp;
		}

		return pool->Alloc();
	}

	void BufferManager::Free(unsigned char *ptr, unsigned int size) {
		unsigned int bucket = BinLog2(NextPow2(size));

		for (unsigned int i = 0; i < pools[bucket].size(); ++i) {
			if (pools[bucket][i]->Owns(ptr)) {
				pools[bucket][i]->Free(ptr);

				if (0 != i) {
					// lift to front
					AbstractMemoryPool *tmp = pools[bucket][0];
					pools[bucket][0] = pools[bucket][i];
					pools[bucket][i] = tmp;
				}

				return;
			}
		}
	}

};
