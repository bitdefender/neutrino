#include "Neutrino.Heap.h"

#include "Neutrino.Memory.h"

#include <cstdio>
#include <cstring>

namespace Neutrino {

	struct HeapZone {
		HeapZone *Prev; // 0xFFFFFFFF if this is the first block
		HeapZone *Next; // 0xFFFFFFFF if this is the last block
		DWORD Size; // size of this block
		DWORD Type; // 0 - free, 1 - allocated
	};

#define INVALID_HEAP_ZONE ((HeapZone *)-1ll)

	Heap::Heap(SIZE_T heapSize) {
		pHeap = nullptr;
		pFirstFree = nullptr;
		size = 0;

		Init(heapSize);
	}

	Heap::~Heap() {
		if (0 != size) {
			Destroy();
		}
	}

	bool Heap::Init(SIZE_T heapSize) {
		HeapZone *fz;
		unsigned char *tHeap;

		tHeap = pHeap = (unsigned char *)Neutrino::Alloc(nullptr, heapSize, PAGE_PROTECTION_READ | PAGE_PROTECTION_WRITE, -1, 0);

		if (!tHeap) {
			return false;
		}

		memset(tHeap, 0, heapSize);

		size = heapSize;

		fz = (HeapZone *)tHeap;

		fz->Next = INVALID_HEAP_ZONE;
		fz->Prev = INVALID_HEAP_ZONE;
		fz->Type = 0;
		fz->Size = (DWORD)(size - sizeof(HeapZone));

		pFirstFree = fz;
		return true;
	}

	bool Heap::Destroy() {
		if (pHeap) {
			Neutrino::Free(pHeap, size);
			pHeap = NULL;
			pFirstFree = NULL;
			size = 0;
		}

		return true;
	}


	void Heap::PrintInfo(HeapZone *fz) {
		printf("FirstFree: %08IX.\n", (UINTPTR)pFirstFree);
		printf("fz  Addr : %08IX.\n", (UINTPTR)fz);
		printf("fz->Next : %08IX.\n", (UINTPTR)fz->Next);
		printf("fz->Prev : %08IX.\n", (UINTPTR)fz->Prev);
		printf("fz->Type : %08X.\n", (DWORD)fz->Type);
		printf("fz->Size : %08zX.\n", (SIZE_T)fz->Size);
		printf("\n");
	}


	void *Heap::Alloc(SIZE_T sz) {
		BYTE *b;
		DWORD first;
		HeapZone *fz, *nfz;

		sz += 3;
		sz &= ~3L;

		//	SC_Lock (&dwMMLock);

		first = 1;

		fz = pFirstFree;

		do {
			//	SC_PrintInfo (fz);

			if (fz->Type == 0) {// free block
				if (sz + sizeof(HeapZone) <= fz->Size) {
					//	printf("Found a block of %d bytes. We need only %d.\n", fz->Size, sz);

					b = (BYTE *) fz + sizeof(HeapZone);

					nfz = (HeapZone *)((BYTE *) fz + sizeof(HeapZone) + sz);

					if (fz->Next != INVALID_HEAP_ZONE)
					{
						fz->Next->Prev = nfz;
					}

					nfz->Next = fz->Next;
					nfz->Prev = fz;
					nfz->Type = 0;
					nfz->Size = (DWORD)(fz->Size - sz - sizeof(HeapZone));

					fz->Next = nfz;
					fz->Type = 1;
					fz->Size = (DWORD)sz;

					if (first) {
						pFirstFree = nfz;
					}

					//	SC_Unlock (&dwMMLock);

					return b;
				}
				else {
					//	printf("Free block, but only %d in size!\n", fz->Size);
					first = 0;
				}
			}

			fz = (HeapZone *)fz->Next;

		} while (fz != INVALID_HEAP_ZONE);

		//	SC_Unlock (&dwMMLock);

		return NULL;
	}

	void Heap::List() {
		DWORD dwMaxSize;
		HeapZone *fz;

		fz = (HeapZone *)pHeap;

		//	SC_Lock (&dwMMLock);

		dwMaxSize = 0;

		do {

			//	SC_PrintInfo (fz);

			if (fz->Type == 1)
			{
				//	printf("fz->addr : %08X, fz->size : %08X.\n", fz, fz->Size);
				dwMaxSize += fz->Size;
			}

			fz = (HeapZone *)fz->Next;

		} while (fz != INVALID_HEAP_ZONE);

		//	printf("%d bytes of memory are in use.\n", dwMaxSize);

		//	SC_Unlock (&dwMMLock);
	}

	void Heap::Free(void *p) {
		HeapZone *fz, *wfz;

		//	SC_Lock (&dwMMLock);

		fz = (HeapZone *)((BYTE *)p - sizeof(HeapZone));

		//	SC_PrintInfo (fz);

		fz->Type = 0;

		wfz = fz->Next;

		if (wfz != INVALID_HEAP_ZONE) {// present?
			if (wfz->Type == 0) {// free?
				fz->Next = wfz->Next;
				fz->Size = fz->Size + wfz->Size + sizeof(HeapZone);
				fz->Type = 0;
			}
		}

		if (fz < pFirstFree) {
			pFirstFree = fz;
		}

		wfz = fz->Prev;

		if (wfz != INVALID_HEAP_ZONE) {// present?
			if (wfz->Type == 0) {// free?
				wfz->Next = fz->Next;
				wfz->Size = wfz->Size + fz->Size + sizeof(HeapZone);

				if (wfz < pFirstFree) {
					pFirstFree = wfz;
				}
			}
		}

		//	SC_Unlock (&dwMMLock);
	}

};
