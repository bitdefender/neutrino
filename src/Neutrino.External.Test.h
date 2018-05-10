#ifndef _EXTERNAL_TEST_H_
#define _EXTERNAL_TEST_H_

namespace Neutrino {

#define TEST_SOURCE_NEW 0xF0
#define TEST_SOURCE_EXCEPTED 0xF1

	enum class ExternalTestSource {
		NEW = TEST_SOURCE_NEW, // newly added test
		EXCEPTED = TEST_SOURCE_NEW // explicitly excepted test
	};

	class ExternalTest {
	private:
		unsigned char *buffer;
		unsigned int size, capacity;
	public:
		ExternalTest();
		~ExternalTest();

		unsigned char *GetBuffer() const;
		unsigned int GetSize() const;

		bool SetCapacity(unsigned int cap);
		bool SetSize(unsigned int size);
	};

};

#endif
