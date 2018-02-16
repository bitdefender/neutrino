#ifndef _NEURTINO_BLOOM_FILTER_H_
#define _NEURTINO_BLOOM_FILTER_H_

#include <vector>
#include <bitset>

namespace Neutrino {
	template <int SIZE>
	class BloomFilter {
	private:
		uint8_t hashCount;
		std::bitset<SIZE> bits;
			
	public:
		BloomFilter(uint8_t numHashes);

		void Add(const uint8_t *data, std::size_t len);
		bool PossiblyContains(const uint8_t *data, std::size_t len) const;
	};
};

#include "Neutrino.Bloom.Filter.hpp"

#endif
