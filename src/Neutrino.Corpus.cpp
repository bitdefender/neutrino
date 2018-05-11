#include "Neutrino.Corpus.h"

#include "TinySHA1.h"

namespace Neutrino {
	Corpus::Corpus() : bloom(5) {
		corpus.rehash(1 << 16);
	}

	Corpus::~Corpus() {
		Clear();
	}

	std::shared_ptr<Test> Corpus::AddTest(const unsigned char *input, int size, TestState state) {
		sha1::SHA1 hash;
		sha1::Digest digest;
		
		hash.ProcessBytes(input, size);
		hash.GetDigest(digest);
		
		CorpusType::iterator r;

		if (bloom.PossiblyContains(digest.digest8, sizeof(digest.digest8)) && (corpus.end() != (r = corpus.find(digest)))) {
			if (r->second->state == TestState::NEW) {
				r->second->state = TestState::RENEW;

				if (TestState::EXCEPTED == state) {
					r->second->state = TestState::EXCEPTED;
				}
			}
			return r->second;
		} else {
			bloom.Add(digest.digest8, sizeof(digest.digest8));
			
			auto ret = std::shared_ptr<Test>(new Test(size, input, digest));
			
			ret->state = state;
			corpus[ret->name] = ret;

			return ret;
		}
	}

	/*std::shared_ptr<Test> Corpus::AddTest(const Test &test) {
		return AddTest(test.buffer, test.size, test.state);
	}*/

	std::shared_ptr<Test> Corpus::FindTest(const sha1::Digest &name) const {
		if (!bloom.PossiblyContains(name.digest8, sizeof(name))) {
			return std::shared_ptr<Test>(nullptr);
		}

		auto r = corpus.find(name);
		if (corpus.end() == r) {
			return std::shared_ptr<Test>(nullptr);
		}

		return r->second;
	}

	void Corpus::Optimize() {
		bloom.Reset();

		for (auto &it : corpus) {
		}
	}

	void Corpus::Stats() const {
		int nw = 0;
		int rnw = 0;
		int executed = 0;
		int evaluated = 0;
		int discarded = 0;

		for (auto &it : corpus) {
			switch (it.second->state) {
				case TestState::NEW: nw++; break;
				case TestState::RENEW: rnw++; break;
				case TestState::EXECUTED: executed++; break;
				case TestState::EVALUATED: evaluated++; break;
				case TestState::DISCARDED: discarded++; break;
			}
		}

		fprintf(
			stderr,
			"New: %d\n"
			"Renew: %d\n"
			"Executed: %d\n"
			"Evaluated: %d\n"
			"Discarded: %d\n",
			nw, rnw, executed, evaluated, discarded
		);

		fflush(stderr);
	}

	void Corpus::Clear() {
		corpus.clear();
	}
};
