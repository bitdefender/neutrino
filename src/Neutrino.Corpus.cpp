#include "Neutrino.Corpus.h"

#include "TinySHA1.h"

namespace Neutrino {
	Corpus::Corpus() : bloom(5) {
		corpus.rehash(1 << 16);
	}

	std::shared_ptr<Test> Corpus::AddTest(const unsigned char *input, int size, TestState state) {
		Test tLookup(size, input);
		
		CorpusType::iterator r;

		if (bloom.PossiblyContains(tLookup.name.digest8, sizeof(tLookup.name.digest8)) && (corpus.end() != (r = corpus.find(tLookup.name)))) {
			if (r->second->state == TestState::NEW) {
				r->second->state = TestState::RENEW;

				if (TestState::EXCEPTED == state) {
					r->second->state = TestState::EXCEPTED;
				}
			}
			return r->second;
		} else {
			bloom.Add(tLookup.name.digest8, sizeof(tLookup.name.digest8));
			
			auto ret = std::shared_ptr<Test>(new Test(std::move(tLookup)));
			
			ret->state = state;
			corpus[ret->name] = ret;

			//int mumu = corpus.bucket_count();

			return ret;
		}
	}

	std::shared_ptr<Test> Corpus::AddTest(const Test &test) {
		return AddTest(test.buffer, test.size, test.state);
	}

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

		printf(
			"New: %d\n"
			"Renew: %d\n"
			"Executed: %d\n"
			"Evaluated: %d\n"
			"Discarded: %d\n",
			nw, rnw, executed, evaluated, discarded
		);
	}
};
