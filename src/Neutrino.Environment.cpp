#include "Neutrino.Environment.h"

namespace Neutrino {
	bool BasicBlock::Equals(UINTPTR rhs) {
		return address == rhs;
	}

	void Environment::FixDirectJump(Environment *env) {
		env->FixDirect();
	}

	void Environment::FixIndirectJump(Environment *env) {
		env->FixIndirect();
	}

	void Environment::FixDirect() {
		UINTPTR cAddr = trace[traceIndex];
		BasicBlock *bb = hash.Find(trace[traceIndex]);

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
	}

	void Environment::FixIndirect() {
		UINTPTR cAddr = trace[traceIndex];
		BasicBlock *bb = hash.Find(trace[traceIndex]);

		if (!bb->translated) {
			// do some actual translation
			bb->code = Translate(cAddr);
			bb->translated = true;
		}

		jumpBuff = (UINTPTR)bb->code;
	}

	bool Environment::AllocOutBuffer() {
		lastBuff = codeBuff.Alloc();
		outBuffer = lastBuff->buffer;
		outSize = lastBuff->size;

		return true;
	}

	void Environment::InitSolveDirectJump() {
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

	void Environment::InitSolveIndirectJump() {
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

	void Environment::InitExec(UINTPTR entry) {
		static const BYTE code[] = {
			0xE9, 0x00, 0x00, 0x00, 0x00
		};

		memcpy(outBuffer, code, sizeof(code));
		entry = (UINTPTR)outBuffer;
		*(UINTPTR *)(&(outBuffer[0x01])) = (UINTPTR)solveDirectJump - ((UINTPTR)outBuffer + sizeof(code));

		hash.SetStub(outBuffer);
		outBuffer += sizeof(code);
		outSize -= sizeof(code);
	}


	void Environment::Fixup(const CodePatch &dest) {
		BasicBlock *bb;

		switch (dest.jumpType) {
			case PATCH_TYPE_UNUSED :
				return;

			case PATCH_TYPE_DIRECT :
				bb = hash.Find(dest.destination);
				*dest.patch = (UINTPTR)&bb->code;
				break;

			case PATCH_TYPE_INDIRECT :
				*dest.patch = solveIndirectJump - ((UINTPTR)dest.patch + sizeof(UINTPTR));
				break;

			case PATCH_TYPE_REG1_BACKUP :
				*dest.patch = (UINTPTR)&regBackup1;
				break;

			case PATCH_TYPE_REG2_BACKUP:
				*dest.patch = (UINTPTR)&regBackup2;
				break;

			case PATCH_TYPE_TRACE_BASE:
				*dest.patch = (UINTPTR)&trace;
				break;

			case PATCH_TYPE_TRACE_INDEX:
				*dest.patch = (UINTPTR)&traceIndex;
				break;
		}
	}

	Environment::Environment(Translator &t) : translator(t) {
		AllocOutBuffer();

		InitSolveDirectJump();
		InitSolveIndirectJump();

		traceIndex = 0;


		virtualStack = (UINTPTR)(stackBuffer + sizeof(stackBuffer) - 4);
	}

	BYTE *Environment::Translate(UINTPTR addr) {
		InstructionState state;

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

	/*DWORD RetAddr_cdecl_0() {
		return (DWORD)GET_RETURN_ADDR();
	}*/

	#ifdef _MSC_VER
		#define GET_RETURN_ADDR _ReturnAddress
		#define CALLING_CONV(conv) __##conv
	#else
		#define GET_RETURN_ADDR() ({ int addr; asm volatile("mov 4(%%ebp), %0" : "=r" (addr)); addr; })
		#define CALLING_CONV(conv) __attribute__((conv))
	#endif

	#define _RET_ADDR_FUNC_2(conv, paramCount, ...) \
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
	_RET_ADDR_FUNC_(stdcall, 4, void *, void *, void *, void *);

	typedef DWORD(*TFunc)(unsigned int, unsigned char *);

	void Environment::Go(UINTPTR entry, unsigned int size, unsigned char *buffer) {

		BasicBlock *bbInit = hash.Find(entry);

		trace[traceIndex] = entry;
		
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

};
