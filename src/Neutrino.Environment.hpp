#include "Neutrino.Environment.h"
#ifndef _NEUTRINO_ENVIRONMENT_HPP_
#define _NEUTRINO_ENVIRONMENT_HPP_

#ifdef _MSC_VER
#define GET_RETURN_ADDR _ReturnAddress
#define CALLING_CONV(conv) __##conv
#else
#define GET_RETURN_ADDR() ({ int addr; asm volatile("mov 4(%%ebp), %0" : "=r" (addr)); addr; })
#define CALLING_CONV(conv) __attribute__((conv))
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

	template <typename STRATEGY>
	void Environment<STRATEGY>::FixDirectJump(Environment<STRATEGY> *env) {
		env->FixDirect();
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::FixIndirectJump(Environment<STRATEGY> *env) {
		env->FixIndirect();
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::FixDirect() {
		UINTPTR cAddr = translator.LastBasicBlock();
		BasicBlock *bb = hash.Find(cAddr);

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::FixIndirect() {
		UINTPTR cAddr = translator.LastBasicBlock();
		BasicBlock *bb = hash.Find(cAddr);

		translator.TouchDeferred();

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
	}

	template <typename STRATEGY>
	bool Environment<STRATEGY>::AllocOutBuffer() {
		lastBuff = codeBuff.Alloc();
		outBuffer = lastBuff->buffer;
		outSize = lastBuff->size;

		return true;
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::InitSolveDirectJump() {
		static const BYTE code[] = {
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x00 - xchg esp, large ds:<dwVirtualStack>
			0x9C, 										// 0x06 - pushf
			0x60,										// 0x07 - pusha
			0x68, 0x46, 0x02, 0x00, 0x00,				// 0x08 - push 0x00000246 - NEW FLAGS
			0x9D,										// 0x0D - popf
			0x68, 0x00, 0x00, 0x00, 0x00,				// 0x0E - push <Environment>
			0xFF, 0x15, 0x00, 0x00, 0x00, 0x00,			// 0x13 - call <dwBranchHandler>
			0x83, 0xC4, 0x04,							// 0x19 - sub esp, 4
			0x61,										// 0x1C - popa
			0x9D,										// 0x1D - popf
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x1E - xchg esp, large ds:<dwVirtualStack>
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00			// 0x24 - jmp large dword ptr ds:<jumpbuff>	
		};

		memcpy(outBuffer, code, sizeof(code));
		solveDirectJump = (UINTPTR)outBuffer;
		fixDirectJump = (UINTPTR)FixDirectJump;

		*(UINTPTR *)(&(outBuffer[0x02])) = (UINTPTR)&virtualStack;
		*(UINTPTR *)(&(outBuffer[0x0F])) = (UINTPTR)this;
		*(UINTPTR *)(&(outBuffer[0x15])) = (UINTPTR)&fixDirectJump;
		*(UINTPTR *)(&(outBuffer[0x20])) = (UINTPTR)&virtualStack;
		*(UINTPTR *)(&(outBuffer[0x26])) = (UINTPTR)&jumpBuff;

		outBuffer += sizeof(code);
		outSize -= sizeof(code);
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::InitSolveIndirectJump() {
		static const BYTE code[] = {
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x00 - xchg esp, large ds:<dwVirtualStack>
			0x9C, 										// 0x06 - pushf
			0x60,										// 0x07 - pusha
			0x68, 0x46, 0x02, 0x00, 0x00,				// 0x08 - push 0x00000246 - NEW FLAGS
			0x9D,										// 0x0D - popf
			0x68, 0x00, 0x00, 0x00, 0x00,				// 0x0E - push <Environment>
			0xFF, 0x15, 0x00, 0x00, 0x00, 0x00,			// 0x13 - call <dwBranchHandler>
			0x83, 0xC4, 0x04,							// 0x19 - sub esp, 4
			0x61,										// 0x1C - popa
			0x9D,										// 0x1D - popf
			0x87, 0x25, 0x00, 0x00, 0x00, 0x00,			// 0x1E - xchg esp, large ds:<dwVirtualStack>
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00			// 0x24 - jmp large dword ptr ds:<jumpbuff>	
		};

		memcpy(outBuffer, code, sizeof(code));
		solveIndirectJump = (UINTPTR)outBuffer;
		fixIndirectJump = (UINTPTR)FixIndirectJump;

		*(UINTPTR *)(&(outBuffer[0x02])) = (UINTPTR)&virtualStack;
		*(UINTPTR *)(&(outBuffer[0x0F])) = (UINTPTR)this;
		*(UINTPTR *)(&(outBuffer[0x15])) = (UINTPTR)&fixIndirectJump;
		*(UINTPTR *)(&(outBuffer[0x20])) = (UINTPTR)&virtualStack;
		*(UINTPTR *)(&(outBuffer[0x26])) = (UINTPTR)&jumpBuff;

		outBuffer += sizeof(code);
		outSize -= sizeof(code);
	}

	template <typename STRATEGY>
	void Environment<STRATEGY>::InitExec(UINTPTR entry) {
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


	template <typename STRATEGY>
	void Environment<STRATEGY>::Fixup(const CodePatch &dest) {
		BasicBlock *bb;

		switch (dest.jumpType) {
		case PATCH_TYPE_UNUSED:
			return;

		case PATCH_TYPE_DIRECT:
			bb = hash.Find(dest.destination);
			*dest.patch = (UINTPTR)&bb->code;
			break;

		case PATCH_TYPE_INDIRECT:
			*dest.patch = solveIndirectJump - ((UINTPTR)dest.patch + sizeof(UINTPTR));
			break;

		case PATCH_TYPE_JMP_REG_BKP:
			*dest.patch = (UINTPTR)&jumpReg;
			break;
		}
	}

	template <typename STRATEGY>
	Environment<STRATEGY>::Environment() {
		AllocOutBuffer();

		InitSolveDirectJump();
		InitSolveIndirectJump();

		virtualStack = (UINTPTR)(stackBuffer + sizeof(stackBuffer) - 4);
		coverage = 0;
	}

	template <typename STRATEGY>
	BYTE *Environment<STRATEGY>::Translate(UINTPTR addr) {
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

	typedef DWORD(*TFunc)(unsigned int, unsigned char *);

	unsigned int __cdecl RetAddr_cdecl_2(unsigned int, unsigned char *);

	template <typename STRATEGY>
	void Environment<STRATEGY>::Go(unsigned int size, unsigned char *buffer) {

		BasicBlock *bbInit = hash.Find(pEntry);

		translator.Reset();
		translator.PushBasicBlock(pEntry);

		TFunc funcPtr[] = {
			(TFunc)RetAddr_cdecl_2,
			(TFunc)bbInit->code
		};

		for (int i = 0; i < 2; ++i) {
			DWORD ret = (funcPtr[i])(size, buffer);

			if (0 == i) {
				BasicBlock *bbFin = hash.Find(ret);
				bbFin->code = (BYTE *)bbFin->address;
				bbFin->translated = true;
			}
		}
	}

	template<typename STRATEGY>
	inline AbstractResult *Environment<STRATEGY>::GetResult() {
		return translator.GetResult();
	}

	template<typename STRATEGY>
	inline int Environment<STRATEGY>::GetCoverage() {
		return coverage;
	}
};


#endif
