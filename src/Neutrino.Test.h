#ifndef _NEUTRINO_TEST_H_
#define _NEUTRINO_TEST_H_

#include "TinySHA1.h"

#include <utility>

namespace Neutrino {
	enum class TestState {
		INVALID = 0,
		NEW, // newly added test
		RENEW, // new test but duplicate
		EXECUTED, // test has been executed
		EVALUATED, // test has been evaluated
		DISCARDED, // test has been discarded (score equals zero)
		EXCEPTED // explicitly excepted test
	};

	class Test {
	public :
		sha1::Digest name;
		TestState state;
		int size;
		unsigned char *buffer;

		Test() : state(TestState::INVALID), size(0), buffer(nullptr) {}

		Test(int sz, const unsigned char *buf) {
			state = TestState::NEW;
			size = sz;
			buffer = new unsigned char[size + 1];

			memcpy(buffer, buf, size);
			buffer[size] = '\0'; // just in case inputs are strings

			sha1::SHA1 hash;
			hash.ProcessBytes(buffer, size);
			hash.GetDigest(name);
		}

		Test(Test &&rhs) : state(rhs.state), name(std::move(rhs.name)), size(rhs.size), buffer(rhs.buffer) {
			rhs.state = TestState::INVALID;
			rhs.size = 0;
			rhs.buffer = nullptr;
		}

		~Test() {
			if (nullptr != buffer) {
				delete buffer;
			}
		}

		Test& operator=(Test&& rhs) {
			state = rhs.state;
			name = std::move(rhs.name);

			size = rhs.size;
			rhs.size = 0;

			if (nullptr != buffer) {
				delete buffer;
			}

			buffer = rhs.buffer;
			rhs.buffer = nullptr;

			return *this;
		}
	};
};



#endif

