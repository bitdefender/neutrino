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
		SIZE_T size;
	public:
		Heap(SIZE_T heapSize);
		~Heap();

		bool Init(SIZE_T heapSize);
		bool Destroy();

		void PrintInfo(HeapZone *fz);
		void List();

		void *Alloc(SIZE_T size);
		void Free(void *ptr);
	};

};

#endif // _NEUTRINO_HEAP_H_


