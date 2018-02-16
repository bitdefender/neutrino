#include "Neutrino.Bloom.Filter.h"
#ifndef _NEUTRINO_BLOOM_FILTER_HPP_
#define _NEUTRINO_BLOOM_FILTER_HPP_

#include "MurmurHash3.h"

#include <array>

namespace Neutrino {

	template <int SIZE>
	class HashGen {
	private:
		uint64_t c, st;
	public:
		HashGen(const uint8_t *data, std::size_t len) {
			std::array<uint64_t, 2> hashValue;
			MurmurHash3_x64_128(data, len, 0, hashValue.data());

			c = hashValue[0] % SIZE;
			st = hashValue[1] % SIZE;
		}

		size_t operator() () {
			size_t t = (size_t)c;

			c += st;
			c %= SIZE;

			return t;
		}
	};

	template <int SIZE>
	BloomFilter<SIZE>::BloomFilter(uint8_t numHashes)
		: hashCount(numHashes) {}


	template <int SIZE>
	void BloomFilter<SIZE>::Add(const uint8_t *data, std::size_t len) {
		HashGen<SIZE> hashValues(data, len);

		for (int n = 0; n < hashCount; n++) {
			bits[hashValues()] = true;
		}
	}

	template <int SIZE>
	bool BloomFilter<SIZE>::PossiblyContains(const uint8_t *data, std::size_t len) const {
		HashGen<SIZE> hashValues(data, len);

		for (int n = 0; n < hashCount; n++) {
			if (!bits[hashValues()]) {
				return false;
			}
		}

		return true;
	}
};


#endif