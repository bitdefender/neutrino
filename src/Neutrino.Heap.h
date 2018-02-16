#ifndef _NEUTRINO_HEAP_H_
#define _NEUTRINO_HEAP_H_

#include "Neutrino.Types.h"

#define HEAP_SIZE 0x100000

namespace Neutrino {

	struct HeapZone;

	/* A self contained heap */
	class Heap {
	private:
		BYTE *pHeap;
		HeapZone *pFirstFree;
		DWORD size;
	public:
		Heap(DWORD heapSize);
		~Heap();

		bool Init(DWORD heapSize);
		bool Destroy();

		void PrintInfo(HeapZone *fz);
		void List();

		void *Alloc(DWORD size);
		void Free(void *ptr);
	};

};

#endif // _NEUTRINO_HEAP_H_


