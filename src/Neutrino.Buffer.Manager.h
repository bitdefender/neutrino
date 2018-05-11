#ifndef _NEUTRINO_BUFFER_MANAGER_H_
#define _NEUTRINO_BUFFER_MANAGER_H_

#include <vector>

namespace Neutrino {

	class AbstractMemoryPool {
	public :
		virtual unsigned char *Alloc() = 0;
		virtual void Free(unsigned char *ptr) = 0;
		virtual bool Owns(unsigned char *ptr) const = 0;
	};

	typedef AbstractMemoryPool *(*PoolAllocatorFunc)();

#define LOG_POOL_SIZE 20 // 16MB pools

	template <int LOG_SIZE>
	class MemoryPool : public AbstractMemoryPool{
	private :
		unsigned char *data;
		unsigned char *free[1 << (LOG_POOL_SIZE - LOG_SIZE)];
		unsigned int freeCount;
	public :
		MemoryPool();
		~MemoryPool();

		virtual unsigned char *Alloc();
		virtual void Free(unsigned char *ptr);

		virtual bool Owns(unsigned char *ptr) const;
	};

	template <int SIZE> AbstractMemoryPool* AllocMemoryPool() {
		return new MemoryPool<SIZE>();
	}

	class BufferManager {
    private :
		static const PoolAllocatorFunc allocators[32];

		std::vector<AbstractMemoryPool *> pools[32];
    public :
		unsigned char *Alloc(unsigned int size);
		void Free(unsigned char *ptr, unsigned int size);
    };

};

#include "Neutrino.Buffer.Manager.hpp"

#endif
