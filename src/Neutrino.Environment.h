#ifndef _NEUTRINO_ENVIRONMENT_H_
#define _NEUTRINO_ENVIRONMENT_H_

#include <cstring>
#include <vector>

#include "Neutrino.Types.h"
#include "Neutrino.Translator.h"
#include "Neutrino.Memory.h"
#include "Neutrino.Result.h"
#include "Neutrino.Heap.h"

namespace Neutrino {

	struct BasicBlock {
		static Heap heap;

		UINTPTR address;
		BYTE *code;
		bool translated;

		struct BlockDest {
			UINTPTR destination;
			UINTPTR *patch;
		} dests[2];

		bool Equals(UINTPTR rhs);

		void * operator new(size_t size);
		void operator delete(void * p);
	};

	template <int SIZE> 
	class BlockHash {
	private:
		struct List {
			static Heap heap;
			void * operator new(size_t size);
			void operator delete(void * p);

			BasicBlock *item;
			List *next;
		};

		List *hash[SIZE];
		BYTE *stub;
		unsigned int Hash(UINTPTR a);
	public :
		BlockHash();

		/* Sets the default stub code for undiscovered basic blocks */
		void SetStub(BYTE *c);

		/* Returns a BasicBlock (initializes it if neccessary) */
		BasicBlock *Find(UINTPTR a);
	};

	template <int SIZE>
	class ExecBuffer {
	public :
		BYTE *buffer;
		static const int size;

		ExecBuffer();
		~ExecBuffer();
	};

	template <typename T> 
	class Allocator {
	private :
		std::vector<T *> allocated;

	public :
		T *Alloc();
		~Allocator();
	};

	class AbstractEnvironment {
	public :
		virtual void InitExec(UINTPTR entry) = 0;
		virtual void Go(unsigned int size, unsigned char *buffer) = 0;
		virtual AbstractResult *GetResult() = 0;
		virtual int GetCoverage() = 0;
	};

	template <typename STRATEGY>
	class Environment : public AbstractEnvironment {
	private :
		Translator<STRATEGY> translator;

		/* A hash for basic block lookup */
		BlockHash<0x10000> hash;

		/* Management structure for translated code */
		Allocator<ExecBuffer<1 << 20> > codeBuff;

		
		ExecBuffer<1 << 20> *lastBuff;
		BYTE *outBuffer;
		int outSize;
		int coverage;

		/* Paralell stack used for translation */
		BYTE stackBuffer[32768];
		UINTPTR virtualStack;
		UINTPTR jumpBuff;

		UINTPTR solveDirectJump, fixDirectJump;
		UINTPTR solveIndirectJump, fixIndirectJump;

		UINTPTR jumpReg;
		UINTPTR pEntry;

		static void FixDirectJump(Environment *env);
		static void FixIndirectJump(Environment *env);

		void FixDirect();
		void FixIndirect();

		void InitSolveDirectJump();
		void InitSolveIndirectJump();
		
		void Fixup(const CodePatch &dest);
		bool AllocOutBuffer();
	public :
		virtual void InitExec(UINTPTR entry);

		Environment();
		BYTE *Translate(UINTPTR addr);
		
		virtual void Go(unsigned int size, unsigned char *buffer);
		virtual AbstractResult *GetResult();
		virtual int GetCoverage();
	};

};

#include "Neutrino.Environment.hpp"

#endif
