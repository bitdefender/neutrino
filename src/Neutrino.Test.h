#ifndef _NEUTRINO_TEST_H_
#define _NEUTRINO_TEST_H_

#include "TinySHA1.h"
#include "Neutrino.Buffer.Manager.h"
#include "Neutrino.External.Test.h"

#include <utility>

namespace Neutrino {
	enum class TestState {
		INVALID = 0,
		RENEW, // new test but duplicate
		EXECUTED, // test has been executed
		EVALUATED, // test has been evaluated
		DISCARDED, // test has been discarded (score equals zero)
		NEW = TEST_SOURCE_NEW, // newly added test
		EXCEPTED = TEST_SOURCE_EXCEPTED // explicitly excepted test
	};

	class TestData {
	public :
		sha1::Digest name;
		TestState state;
		int size;
		unsigned char *buffer;

		TestData() : state(TestState::INVALID), size(0), buffer(nullptr) {}

		TestData(const sha1::Digest &rhs) : state(TestState::INVALID), size(0), buffer(nullptr), name(rhs) { }

		TestData(TestData &&rhs) : state(rhs.state), name(std::move(rhs.name)), size(rhs.size), buffer(rhs.buffer) {
			rhs.state = TestState::INVALID;
			rhs.size = 0;
			rhs.buffer = nullptr;
		}
	};

	class Test : public TestData{
	private:
		static BufferManager mgr;
	public:


		Test() : TestData() {}

		Test(int sz, const unsigned char *buf) {
			state = TestState::NEW;
			size = sz;
			buffer = mgr.Alloc(size + 1);
			/*if (!mgr.Alloc(size + 1, buffer)) {
				__asm int 3;
			}*/

			memcpy(buffer, buf, size);
			buffer[size] = '\0'; // just in case inputs are strings

			sha1::SHA1 hash;
			hash.ProcessBytes(buffer, size);
			hash.GetDigest(name);
		}

		Test(int sz, const unsigned char *buf, const sha1::Digest &digest) : TestData(digest) {
			state = TestState::NEW;
			size = sz;
			buffer = mgr.Alloc(size + 1);
			/*if (!mgr.Alloc(size + 1, buffer)) {
			__asm int 3;
			}*/

			memcpy(buffer, buf, size);
			buffer[size] = '\0'; // just in case inputs are strings
		}

		Test(Test &&rhs) : TestData(std::move((TestData &&)rhs)) { }

		~Test() {
			if (nullptr != buffer) {
				mgr.Free(buffer, size - 1);
				//delete buffer;
				//mgr.Free(buffer);
			}
		}

		Test& operator=(Test&& rhs) {
			state = rhs.state;
			name = std::move(rhs.name);

			size = rhs.size;
			rhs.size = 0;

			if (nullptr != buffer) {
				mgr.Free(buffer, size - 1);
				//delete buffer;
				//mgr.Free(buffer);
			}

			buffer = rhs.buffer;
			rhs.buffer = nullptr;

			return *this;
		}
	};
};



#endif

