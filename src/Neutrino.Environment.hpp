#ifndef _NEUTRINO_ENVIRONMENT_HPP_
#define _NEUTRINO_ENVIRONMENT_HPP_

namespace Neutrino {
	template<int SIZE>
	inline unsigned int BlockHash<SIZE>::Hash(UINTPTR addr) {
		return addr % (SIZE - 1);
	}

	template<int SIZE>
	inline BlockHash<SIZE>::BlockHash() {
		memset(hash, 0, sizeof(hash));
		stub = nullptr;
	}

	template<int SIZE>
	inline void BlockHash<SIZE>::SetStub(BYTE *code) {
		stub = code;
	}

	template<int SIZE>
	inline BasicBlock *BlockHash<SIZE>::Find(UINTPTR addr) {
		int ha = Hash(addr);
		for (List *it = hash[ha]; nullptr != it; it = it->next) {
			if (it->item->Equals(addr)) {
				return it->item;
			}
		}

		// TODO: use a faster allcoator here
		BasicBlock *ret = new BasicBlock();
		ret->address = addr;
		ret->code = stub;
		ret->translated = false;

		List *l = new List();
		l->item = ret;
		l->next = hash[ha];
		hash[ha] = l;

		return ret;
	}

	template<int SIZE> const int ExecBuffer<SIZE>::size = SIZE;

	template<int SIZE>
	inline ExecBuffer<SIZE>::ExecBuffer() {
		buffer = (BYTE *)Alloc(
			nullptr,
			SIZE,
			PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE | PAGE_PROTECTION_EXECUTE,
			-1,
			0
		);
	}

	template<int SIZE>
	inline ExecBuffer<SIZE>::~ExecBuffer() {
		Free(buffer, SIZE);
	}

	template<typename T>
	inline T * Allocator<T>::Alloc() {
		T *ret = new T();
		allocated.push_back(ret);
		return ret;
	}

	template<typename T>
	inline Allocator<T>::~Allocator() {
		for (auto &p : allocated) {
			delete &(*p);
		}
	}
};


#endif
