#include "Quark.Debugger.h"
#include "Neutrino.Test.h"

#include <string>
#include <experimental/filesystem> // C++-standard filesystem header file in VS15, VS17.
namespace fs = std::experimental::filesystem; // experimental for VS15, VS17.
//bool firstDbgBreak = true;



int main() {
	Quark::Debugger dbg("neutrino.exe");

	fs::path initialCorpusDir("./start_corpus");
	fs::path corpusDir("./corpus");
	fs::path outputDir("./output");
	fs::path exceptionsDir("./exceptions");
	fs::path dumpsDir("./dumps");

	fs::remove_all(corpusDir);
	fs::create_directory(corpusDir);

	fs::copy(initialCorpusDir, corpusDir, fs::copy_options::recursive);

	while (true) {
		std::string crashTestName;
		
		dbg.Perform(crashTestName);

		fs::path testPath = fs::path(crashTestName);
		testPath += ".tst";
		fs::copy(testPath, exceptionsDir, fs::copy_options::overwrite_existing);
		fs::remove(testPath);

		fs::path dumpPath = fs::path(crashTestName);
		dumpPath += ".dmp";
		fs::copy(dumpPath, dumpsDir, fs::copy_options::overwrite_existing);
		fs::remove(dumpPath);


		fs::remove_all(corpusDir);
		fs::create_directory(corpusDir);
		fs::copy(outputDir, corpusDir, fs::copy_options::recursive);
		fs::remove_all(outputDir);
		fs::create_directory(outputDir);
	}

	return 0;
}