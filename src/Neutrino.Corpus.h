#ifndef _NEUTRINO_CORPUS_H_
#define _NEUTRINO_CORPUS_H_

#include "Neutrino.Bloom.Filter.h"
#include "Neutrino.Test.h"

#include <unordered_map>
#include <functional>
#include <memory>

namespace Neutrino {

	class Corpus {
	private :
		BloomFilter<1 << 20> bloom;

		struct SHA1DigestHash {
			inline std::size_t operator()(const sha1::Digest &v) const {
				std::hash<uint32_t> hBase;
				return
					hBase(v.digest32[0]) +
					31 * hBase(v.digest32[1]) +
					41 * hBase(v.digest32[2]) +
					53 * hBase(v.digest32[3]) +
					97 * hBase(v.digest32[4]);
			}
		};

		using CorpusType = std::unordered_map<sha1::Digest, std::shared_ptr<Test>, SHA1DigestHash>;

		CorpusType corpus;
	public :

		Corpus();

		std::shared_ptr<Test> AddTest(const unsigned char *input, int size, TestState state);
		std::shared_ptr<Test> AddTest(const Test &test);

		std::shared_ptr<Test> FindTest(const sha1::Digest &name) const;

		void Optimize();

		void Stats() const;
	};

};

#endif
