#include "Neutrino.Environment.h"
#ifndef _NEUTRINO_ENVIRONMENT_HPP_
#define _NEUTRINO_ENVIRONMENT_HPP_

#ifdef _MSC_VER
#define GET_RETURN_ADDR _ReturnAddress
#define CALLING_CONV(conv) __##conv
#else
//#define GET_RETURN_ADDR() ({ int addr; asm volatile("mov 4(%%ebp), %0" : "=r" (addr)); addr; })
#define GET_RETURN_ADDR() __builtin_return_address(0)
#define CALLING_CONV(conv) __attribute__((__##conv##__))
#endif

/*#define _RET_ADDR_FUNC_2(conv, paramCount, ...) \
		unsigned int CALLING_CONV(conv) RetAddr_##conv##_##paramCount (__VA_ARGS__) { \
			return (unsigned int)GET_RETURN_ADDR(); \
		}

#define _RET_ADDR_FUNC_(conv, paramCount, ...) _RET_ADDR_FUNC_2(conv, paramCount, __VA_ARGS__)

_RET_ADDR_FUNC_(cdecl, 0);
_RET_ADDR_FUNC_(cdecl, 1, void *);
_RET_ADDR_FUNC_(cdecl, 2, void *, void *);
_RET_ADDR_FUNC_(cdecl, 3, void *, void *, void *);
_RET_ADDR_FUNC_(cdecl, 4, void *, void *, void *, void *);

_RET_ADDR_FUNC_(stdcall, 0);
_RET_ADDR_FUNC_(stdcall, 1, void *);
_RET_ADDR_FUNC_(stdcall, 2, void *, void *);
_RET_ADDR_FUNC_(stdcall, 3, void *, void *, void *);
_RET_ADDR_FUNC_(stdcall, 4, void *, void *, void *, void *);*/

#include <cstdio>

namespace Neutrino {
	template<int SIZE>
	Heap BlockHash<SIZE>::List::heap(1 << 20);

	template<int SIZE>
	inline void * BlockHash<SIZE>::List::operator new(size_t size) {
		return heap.Alloc(size);
	}

	template<int SIZE>
	inline void BlockHash<SIZE>::List::operator delete(void *p) {
		heap.Free(p);
	}

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

		// TODO: use a faster allocator here
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
	inline T *Allocator<T>::Alloc() {
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

	template <typename TRANSLATOR, typename TRAMPOLINE>
	UINTPTR Environment<TRANSLATOR, TRAMPOLINE>::FixDirectJump(Environment<TRANSLATOR, TRAMPOLINE> *env) {
		return env->FixDirect();
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	UINTPTR Environment<TRANSLATOR, TRAMPOLINE>::FixIndirectJump(Environment<TRANSLATOR, TRAMPOLINE> *env) {
		return env->FixIndirect();
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	UINTPTR Environment<TRANSLATOR, TRAMPOLINE>::FixDirect() {
		UINTPTR cAddr = translator.LastBasicBlock();
		BasicBlock *bb = hash.Find(cAddr);

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
		//fprintf(stderr, "DIRECT   %p => %p\n", (void *)cAddr, bb->code);
		return jumpBuff;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	UINTPTR Environment<TRANSLATOR, TRAMPOLINE>::FixIndirect() {
		UINTPTR cAddr = translator.LastBasicBlock();
		BasicBlock *bb = hash.Find(cAddr);

		translator.TouchDeferred();

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
		//fprintf(stderr, "INDIRECT %p => %p\n", (void *)cAddr, bb->code);
		return jumpBuff;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	bool Environment<TRANSLATOR, TRAMPOLINE>::AllocOutBuffer() {
		lastBuff = codeBuff.Alloc();
		outBuffer = lastBuff->buffer;
		outSize = lastBuff->size;

		return true;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	void Environment<TRANSLATOR, TRAMPOLINE>::InitSolveDirectJump(BYTE *mem) {
		solveDirectJump = (UINTPTR)outBuffer;
		fixDirectJump = (UINTPTR)FixDirectJump;

		DWORD codeSize = TRAMPOLINE::MakeTrampoline(
			outBuffer, 
			(UINTPTR)&virtualStack,
			(UINTPTR)&jumpBuff,
			(UINTPTR)this,
			(UINTPTR)&fixDirectJump,
			(UINTPTR)mem
		);

		outBuffer += codeSize;
		outSize -= codeSize;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	void Environment<TRANSLATOR, TRAMPOLINE>::InitSolveIndirectJump(BYTE *mem) {
		solveIndirectJump = (UINTPTR)outBuffer;
		fixIndirectJump = (UINTPTR)FixIndirectJump;

		DWORD codeSize = TRAMPOLINE::MakeTrampoline(
			outBuffer,
			(UINTPTR)&virtualStack,
			(UINTPTR)&jumpBuff,
			(UINTPTR)this,
			(UINTPTR)&fixIndirectJump,
			(UINTPTR)mem
		);

		outBuffer += codeSize;
		outSize -= codeSize;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	void Environment<TRANSLATOR, TRAMPOLINE>::InitExec(UINTPTR entry) {
		static const BYTE code[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		pEntry = entry;

		memcpy(outBuffer, code, sizeof(code));
		entry = (UINTPTR)outBuffer;
		*(UINTPTR *)(&(outBuffer[0x01])) = (UINTPTR)solveDirectJump - ((UINTPTR)outBuffer + sizeof(code));

		hash.SetStub(outBuffer);
		outBuffer += sizeof(code);
		outSize -= sizeof(code);
	}


	template <typename TRANSLATOR, typename TRAMPOLINE>
	void Environment<TRANSLATOR, TRAMPOLINE>::Fixup(const CodePatch &dest) {
		BasicBlock *bb;

		switch (dest.jumpType) {
		case PATCH_TYPE_UNUSED:
			return;

		case PATCH_TYPE_DIRECT:
			bb = hash.Find(dest.destination);
			*dest.patch = (UINTPTR)&bb->code;
			break;

		case PATCH_TYPE_DIRECT_64:
			bb = hash.Find(dest.destination);
			*(DWORD *)dest.patch = (DWORD)((UINTPTR)&bb->code - ((UINTPTR)dest.patch + 4));
			break;

		case PATCH_TYPE_INDIRECT:
			*dest.patch = solveIndirectJump - ((UINTPTR)dest.patch + 4);
			break;

		case PATCH_TYPE_INDIRECT_64:
			*(DWORD *)dest.patch = (DWORD)((UINTPTR)solveIndirectJump - ((UINTPTR)dest.patch + 4));
			break;

		case PATCH_TYPE_JMP_REG_BKP:
			*dest.patch = (UINTPTR)&jumpReg;
			break;

		case PATCH_TYPE_TRANSLATOR_SLOT:
			*dest.patch = (UINTPTR)&translatorCodeMem[dest.destination];
			break;
		}
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	Environment<TRANSLATOR, TRAMPOLINE>::Environment() {
		AllocOutBuffer();

		DWORD trampolineMemSz = TRAMPOLINE::GetCodeMemSize(); 
		BYTE *trampolineMem = outBuffer;
		outBuffer += trampolineMemSz;
		outSize -= trampolineMemSz;

		DWORD translatorMemSz = TRANSLATOR::GetCodeMemSize();
		translatorCodeMem = (UINTPTR *)outBuffer;
		outBuffer += translatorMemSz;
		outSize -= translatorMemSz;

		*(UINTPTR *)outBuffer = 0x9090909090909090ULL;
		outBuffer += 8;
		outSize -= 8;
		
		InitSolveDirectJump(trampolineMem);
		InitSolveIndirectJump(trampolineMem);

		virtualStack = (UINTPTR)(stackBuffer + sizeof(stackBuffer) - sizeof(UINTPTR));
		coverage = 0;
	}

	template <typename TRANSLATOR, typename TRAMPOLINE>
	BYTE *Environment<TRANSLATOR, TRAMPOLINE>::Translate(UINTPTR addr) {
		TranslationState state;

		coverage++;

		const BYTE *rIn = (BYTE *)addr;
		BYTE *rOut = outBuffer, *rRet = outBuffer; // place buffer here
		int size = outSize;


		while (0 == (state.flags & FLAG_JUMP)) {
			translator.Translate(rIn, rOut, size, state);
		}

		outBuffer = rOut;
		outSize = size;

		for (unsigned int i = 0; i < state.patchCount; ++i) {
			Fixup(state.patch[i]);
		}

		return rRet;
	}

	typedef UINTPTR(*TFunc)(unsigned int, unsigned char *);

	UINTPTR CALLING_CONV(cdecl) RetAddr_cdecl_2(unsigned int, unsigned char *);

	template <typename TRANSLATOR, typename TRAMPOLINE>
	void Environment<TRANSLATOR, TRAMPOLINE>::Go(unsigned int size, unsigned char *buffer) {

		BasicBlock *bbInit = hash.Find(pEntry);

		translator.Reset();
		translator.PushBasicBlock(pEntry);

		TFunc funcPtr[] = {
			(TFunc)RetAddr_cdecl_2,
			(TFunc)bbInit->code
		};

		for (int i = 0; i < 2; ++i) {
			UINTPTR ret = (funcPtr[i])(size, buffer);

			if (0 == i) {
				BasicBlock *bbFin = hash.Find(ret);
				bbFin->code = (BYTE *)bbFin->address;
				bbFin->translated = true;
			}
		}
	}

	template<typename TRANSLATOR, typename TRAMPOLINE>
	inline AbstractResult *Environment<TRANSLATOR, TRAMPOLINE>::GetResult() {
		return translator.GetResult();
	}

	template<typename TRANSLATOR, typename TRAMPOLINE>
	inline int Environment<TRANSLATOR, TRAMPOLINE>::GetCoverage() {
		return coverage;
	}
};


#endif
