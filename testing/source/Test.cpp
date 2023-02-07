#include "testing/config.h"
#include <list>
#include <set>

#ifdef USE_CATCH
#pragma warning(disable : 4702)
	//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
	#define CATCH_CONFIG_RUNNER
	#include "testing/Test.h"

static bool isNightlyRun{ false };

bool alaska::IsNightlyTestRun() {
	return isNightlyRun;
}

// eigene main zur Behandlung eigener/ergänzender Kommandozeilenargumente (-> https://github.com/catchorg/Catch2/blob/devel/docs/own-main.md#top)
int main(int argc, char* argv[])
{
	Catch::Session session; // There must be exactly one instance

	//int height = 0; // Some user variable you want to be able to set

	// Build a new parser on top of Catch's
	using namespace Catch::clara;
	auto cli
		= session.cli() // Get Catch's composite command line parser
		| Opt(isNightlyRun, "isNightlyRun") // bind variable to a new option, with a hint string
		["--nightly"]    // the option names it will respond to
		("is running nightly test?");        // description string for the help output

  // Now pass the new composite back to Catch so it uses that
	session.cli(cli);

	// Let Catch (using Clara) parse the command line
	int returnCode = session.applyCommandLine(argc, argv);
	if (returnCode != 0) // Indicates a command line error
		return returnCode;

	if (isNightlyRun)
		std::cout << "isNightlyRun: " << isNightlyRun << std::endl;

	return session.run();
}


/// Eigener EventListener zum Loggen der Section-Zeiten
struct SectionTimesListener : Catch::TestEventListenerBase {

	using TestEventListenerBase::TestEventListenerBase; // inherit constructor
	std::string currTestCase;

	struct Info {
		Info(const std::string &name, const double dur): Name(name), Duration(dur) {}
		std::string Name;	///< CaseName.SectionName
		double Duration;
	};

	~SectionTimesListener() {
		std::ofstream f("TimesListener.out");

		// Ausgabe mit größter Laufzeit zuerst
		stats.sort([](const auto &a, const auto &b) { return a.Duration > b.Duration; });
		std::set<std::string> match;
		double sumTime {};
		for (const auto &s : stats)
			if (match.insert(s.Name).second) {
				f << s.Name << " , " << s.Duration << /*" " << s.sectionInfo.description << */"\n";
				sumTime += s.Duration;
			}

		f << "Gesamtzeit summiert: " << sumTime << "s\n";

		//f << "\n";
		////f << "\nTestCaseStats:...\n";
		//for (const auto &s : stats2)
		//	f << s.testInfo.name << ": " /*<< s.totals.delta<< " "*/  << s.testInfo.description << "\n";
	}

	std::list<Info> stats;
	//std::list<Catch::TestCaseStats> stats2;

	void testCaseStarting(Catch::TestCaseInfo const& testInfo) override {
		//std::cout << "starting: " << testInfo.name << "\n";
		currTestCase = testInfo.name;
	}

	void testCaseEnded(Catch::TestCaseStats const& testCaseStats) override {
		// Tear-down after a test case is run
		//stats2.push_back(testCaseStats);
	}

	//void sectionStarting(Catch::SectionInfo const& sectionInfo) override {}

	void sectionEnded(Catch::SectionStats const& sectionStats) override {
		stats.emplace_back(currTestCase + "." + sectionStats.sectionInfo.name, sectionStats.durationInSeconds);
	}
};

// den Listener Registrieren
CATCH_REGISTER_LISTENER(SectionTimesListener)

#else

#include "testing/Test.h"
#include "testing/rti.h"
#include <fstream>
#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include "crtdbg.h"
#include <list>
#endif
#include <map>
#include <assert.h>

using namespace alaska;
using namespace std;

///< Klasse zum mitschneiden des Outputs auf cout; geklaut von "http://wordaligned.org/articles/cpp-streambufs" ,in den Kommentaren ist die Verwendung zu finden und bei bedarf eine generiche Version
class teebuf: public streambuf {
	streambuf *sb1,*sb2;
	bool testWrote;
	string currTestHeader;
public:
	///< Construct a streambuf which tees output to both input streambufs.
	teebuf(streambuf * sb1, streambuf * sb2): sb1(sb1), sb2(sb2),testWrote(false) {		}
	void TestHeader(const string &t) {
		currTestHeader = t;
		testWrote = false;
	}
protected:
	///< This tee buffer has no buffer. So every character "overflows" and can be put directly into the teed buffers.
	virtual int overflow(int c) {
		if (c == EOF) 
			return !EOF;
		else {
			if(!testWrote) {
				testWrote = true;
				cout << currTestHeader;
			}
			int const r1 = sb1->sputc(static_cast<char>(c));
			int const r2 = sb2->sputc(static_cast<char>(c));
			return r1 == EOF || r2 == EOF ? EOF : c;
		}
	}
	///< Sync both teed buffers.
	virtual int sync() {
		int const r1 = sb1->pubsync();
		int const r2 = sb2->pubsync();
		return r1 == 0 && r2 == 0 ? 0 : -1;
	}   
};

int main(int argc, char** argv) {
#ifdef _MSC_VER
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#ifdef _DEBUG
	_CrtSetBreakAlloc(-1);
#endif
#endif
	
	string filter;
	
	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-t")) {
			if(argc > i+1)			
				filter = argv[++i];
			else {
				cout << "miss Testname after Option -t!";
				return -1;
			}
		}
	}

	CRuntimeInfo StoppUhr;
	StoppUhr.StartRuntimeMeasurement();
	printf("\n");

	// Fehlerausgabe umbiegen
	//std::ofstream out("test.err");
	stringstream errStr;
	//std::streambuf* sbuf = std::cerr.rdbuf();
	cerr.rdbuf(errStr.rdbuf());

	list<Test*> TestTimes;
	ofstream log("test.log");

	ofstream outStr("test.out");
	streambuf * const coutbuf = cout.rdbuf();	// initial buffer of std::cout
	teebuf tee(coutbuf, outStr.rdbuf());		// create tee buffer that directs everything to console and file
	cout.rdbuf(&tee);							// replace default cout buffer by teebuf

	for ( Test* test = Test::firstTest; test != 0; test = test->nextTest ) {
		if( !filter.empty() && filter != test->m_testName)
			continue;
			
		cerr.rdbuf(errStr.rdbuf());
		
		const size_t prevNumChecks = Test::m_numCHecks;

		tee.TestHeader("\nAusgaben von Test: \"" + std::string(test->m_testName) + "\" (File: " + test->m_fileName + "):\n");
		CRuntimeInfo TestUhr;
		try {	
			log << "starting: " <<  test->m_testName << "(file: " << test->m_fileName << ", line: " << test->m_lineNumber << ")\n";
			log.flush();
			TestUhr.StartRuntimeMeasurement();
			
			test->run();
			
			TestUhr.StopRuntimeMeasurement();
		}
		catch(const std::exception & e) {
			TestUhr.StopRuntimeMeasurement();
			cout << "exception within TEST\n"
					"----------------------\n"
					"file: " << test->m_fileName << "\n"
					"line: " << test->m_lineNumber << "\n"
					"test: " << test->m_testName << "\n"
					"exception: " << e.what() << "\n\n";
			test->m_numFailed++;
			test->m_failed = true;
		}

		test->m_checks = Test::m_numCHecks - prevNumChecks;
		test->m_testTime = TestUhr.GetTimeDifference();
		TestTimes.push_back(test);
	}
	tee.TestHeader("");
	
	log.clear();
	TestTimes.sort([](const auto v1, const auto v2) -> bool {	return v1->m_testTime > v2->m_testTime; }); // Absteigend!
	for(const auto test: TestTimes) {
		log << "Runtime: "	<< test->m_testTime << " s"
			<< " checks: "  << test->m_checks
			<< " test: "    << test->m_testName
			<< " file: "	<< test->m_fileName 
			<< " line: "	<< test->m_lineNumber 
			<< (test->m_failed ? "\tFAILED" : "") 
			<< " \n";
	}
	if(!errStr.str().empty()) {
		ofstream out("test.err");
		out << errStr.str();
		out.close();
	}
		
	StoppUhr.StopRuntimeMeasurement();
	cout << "Runtime:" <<  StoppUhr.GetTimeDifference() << " sec.\n\n";
	cout << Test::m_numFailed << "/" << Test::m_numCHecks << " checks failed. (in ";
	if(filter.empty())
		cout << Test::m_numTests << " tests";
	else 
		cout << "Tests with name: "<< filter;
	cout << ")\n\n";

	if ( !Test::m_numFailed )
		cout << "All tests passed.\n\n";

	// reset std::cout default buffer to prevent error at termination
	cout.rdbuf(coutbuf);
	outStr.close();

	return (int)Test::m_numFailed;
}

namespace alaska
{
	Test::Test(const char* fileName, const char* testName, const int lineNumber)
		: m_fileName(fileName)
		, m_lineNumber(lineNumber)
		, m_testName(testName)
		, m_failed(false)
		, m_checks(0)
	{
		if ( firstTest )
		{
			lastTest->nextTest = this;
			lastTest = this;
		} else {
			firstTest = lastTest = this;
		}
		m_numTests++;
		nextTest = 0;
	}

	Test::~Test()
	{
	}

	Test* Test::firstTest = 0;
	Test* Test::lastTest;
	size_t Test::m_numFailed = 0;
	size_t Test::m_numTests = 0;
	size_t Test::m_numCHecks = 0;
}
#endif