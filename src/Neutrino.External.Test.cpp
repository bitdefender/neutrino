#include "Neutrino.External.Test.h"

#include <cstdlib>

namespace Neutrino {
	ExternalTest::ExternalTest() {
		buffer = nullptr;
		size = capacity = 0;
	}

	ExternalTest::~ExternalTest() {
		if (nullptr != buffer) {
			free(buffer);
		}
	}

	unsigned char *ExternalTest::GetBuffer() const {
		return buffer;
	}

	unsigned int ExternalTest::GetSize() const {
		return size;
	}

	bool ExternalTest::SetCapacity(unsigned int cap) {
		if (cap > capacity) {
			unsigned char *tmp = (unsigned char *)realloc(buffer, cap);

			if (nullptr == tmp) {
				return false;
			}

			buffer = tmp;
			capacity = cap;
		}

		return true;
	}

	bool ExternalTest::SetSize(unsigned int sz) {
		if (sz > capacity) {
			if (!SetCapacity(sz)) {
				return false;
			}
		}

		size = sz;
		return true;
	}
};