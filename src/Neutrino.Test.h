#ifndef _NEUTRINO_TEST_H_
#define _NEUTRINO_TEST_H_

namespace Neutrino {
	class Test {
	public :
		int size;
		unsigned char *buffer;

		Test() : size(0), buffer(nullptr) {}

		Test(Test &&rhs) : size(rhs.size), buffer(rhs.buffer) {
			rhs.size = 0;
			rhs.buffer = nullptr;
		}

		~Test() {
			if (nullptr != buffer) {
				delete buffer;
			}
		}

		Test& operator=(Test&& rhs) {
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

