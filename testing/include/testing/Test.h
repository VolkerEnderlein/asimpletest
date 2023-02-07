#ifndef ALASKA_TEST
#define ALASKA_TEST

#include <iostream>
#include "config.h"

namespace alaska {

	bool IsNightlyTestRun();	///< Abfragemöglichkeit ob der Test nachts mit Flag --nightly gestartet wurde (z.B. um interaktive Tests zu deaktivieren/überspringen/automatisieren)

	template<typename T> std::string toString(const T & v) {
		std::ostringstream o;
		o << v;
		return o.str();
	}
}

#ifdef USE_CATCH
	// dadurch ist normalerweise der CATCH_ - Prefix zu verwenden!
	#define CATCH_CONFIG_PREFIX_ALL		
	
	#define USE_CATCH_2  1
	#if USE_CATCH_2
		#include "testing/catch2.hpp"
	#else
		#include "testing/catch.hpp"
	#endif	
	#ifdef CATCH_CONFIG_PREFIX_ALL
		#define TEST(Name)					CATCH_TEST_CASE( #Name##__FILE__, "[old Style]" )
		#define TEST_CASE					CATCH_TEST_CASE
		#define SECTION						CATCH_SECTION
		#define CHECK_THROWS				CATCH_CHECK_THROWS
		#define CHECK_THROWS_AS				CATCH_CHECK_THROWS_AS
		#define REQUIRE						CATCH_REQUIRE
		#define REQUIRE_THROWS				CATCH_REQUIRE_THROWS
		#define REQUIRE_NOTHROW				CATCH_REQUIRE_NOTHROW
		#define REQUIRE_THROWS_WITH			CATCH_REQUIRE_THROWS_WITH
		#define REQUIRE_THROWS_AS			CATCH_REQUIRE_THROWS_AS
		#define TEST_CASE_METHOD			CATCH_TEST_CASE_METHOD
		#define INFO						CATCH_INFO
		#define TEMPLATE_TEST_CASE			CATCH_TEMPLATE_TEST_CASE
		#define TEMPLATE_PRODUCT_TEST_CASE	CATCH_TEMPLATE_PRODUCT_TEST_CASE
		#define TEMPLATE_LIST_TEST_CASE( ... ) INTERNAL_CATCH_EXPAND_VARGS( INTERNAL_CATCH_TEMPLATE_LIST_TEST_CASE( __VA_ARGS__ ) )
	#else
		#define TEST(Name)					TEST_CASE( #Name##__FILE__, "[old Style]" )
		#define CATCH_TEST_CASE				TEST_CASE				
		#define CATCH_SECTION				SECTION	
		#define CATCH_REQUIRE				REQUIRE				
		#define CATCH_CHECK					CHECK
		#define CATCH_CHECK_THROWS_AS		CHECK_THROWS_AS
		#define CATCH_REQUIRE_THROWS_AS		REQUIRE_THROWS_AS
		#define CATCH_REQUIRE_THROWS		REQUIRE_THROWS
		#define CATCH_REQUIRE_THROWS_WITH	REQUIRE_THROWS_WITH
		#define CATCH_REQUIRE_NOTHROW		REQUIRE_NOTHROW
	#endif


#pragma warning(disable : 4701)	 // Die möglicherweise nicht initialisierte lokale Variable ... wurde verwendet

// Hilfsfunktion um Zeilennummer beim vergleich zu erhalten und gleichzeeitig keinen Fehler zu werfen wenn z.B. char* Typen miteinander verglichen werden
template<class T1, class T2> inline bool _Equals(const T1 & v1, const T2 & v2) {
#pragma warning(disable : 4389)
#pragma warning(disable : 4800)
	return static_cast<bool>(v1 == v2);
#pragma warning(default : 4389)
#pragma warning(default : 4800)
}
#ifdef CATCH_CONFIG_PREFIX_ALL
	#define EQUALS(v1, v2)									CATCH_REQUIRE( _Equals(v1, v2) );

	#define LOWER(v1, v2)									CATCH_REQUIRE( (v1) <  (v2) );
	#define WITHIN(v1, v2, v3) {	 \
		auto m1 = v1;				 \
		CATCH_REQUIRE(m1 > v2 );	 \
		CATCH_REQUIRE(m1 < v3);		 \
	}
	
	// Klammerung der Condition für komplexere Ausdrücke (die per && / || verknüpft sind) notwendig! (oft in alten Tests der Fall)
	// verhindert leider die Text-Expandierung der Condition :(
	#define CHECK(Condition)							CATCH_CHECK( (Condition) );
	
#else
	#define EQUALS(v1, v2)									REQUIRE( _Equals(v1, v2) );

	#define LOWER(v1, v2)									REQUIRE( (v1) <  (v2) );
	#define WITHIN(v1, v2, v3) {	\
			auto m1 = v1;			\
			REQUIRE(m1 > v2 );		\
			REQUIRE(m1 < v3);		\
		}
#endif

	
	// neues MCECK-Makro zur Verwendung mit komplexen (Multi) verketteten Bedingungen
	#define MCHECK(Condition)							CHECK( (Condition) );
	//#define CHECK(Condition)							CATCH_REQUIRE( (Condition) );
	#define ASSUME_EXCEPTION(expr, exceptionType, excText)	CATCH_REQUIRE_THROWS_AS( expr, exceptionType );
#else

#include <string>
#include <sstream>
#include <iomanip>
#include <functional>

namespace alaska {
	
	struct Test	{
		Test(const char* fileName, const char* testName, const int lineNumber);

		virtual void run() const = 0;

		const char* m_fileName;
		const int m_lineNumber;
		const char* m_testName;
		bool m_failed;
		size_t m_checks;			///< checks per Test
		double m_testTime;			///< Zeitdauer für den Test in Sekunden


		static size_t m_numTests;	///< Anz. Testroutienen
		static size_t m_numCHecks;	///< Anz. Testaufrufe (wie CHECK,EQUAL...)
		static size_t m_numFailed;	///< Anz. fehlgeschlagener Checks

		static Test* firstTest;
		static Test* lastTest;
		Test* nextTest;

		virtual ~Test();

		template<class T1> void Check(const T1 & cond, const char* file, const int line) const {
			++m_numCHecks;

			if ( !cond )
			{
				char* value_ = "true";
				if ( toString(cond).c_str() ) 
					value_ = "false";
				std::cout << "CHECK failure\n"
							 "-------------\n"
							 "file: " << file << "\n"
							 "line: " << line << "\n"
							 "test: " << m_testName << "\n"
							 "text: " << value_ << "\n\n";
				m_numFailed++;
			}
		}

		template<class T1, class T2> void Equals(const T1 & v1, const T2 & v2, const char* file, const int line) const {
			++m_numCHecks;

#pragma warning(disable : 4389)
			if ( !(v1 == v2) )
#pragma warning(default : 4389)
			{
				std::cout << "EQUALS failure\n"
							 "--------------\n"
							 "file: " << file << "\n"
							 "line: " << line << "\n"
							 "test: " << m_testName << "\n"
							 "value 1:\n" << toString(v1) << "\n"
							 "value 2:\n" << toString(v2) << "\n"
							 "--------------\n";
				m_numFailed++;
			}
		}

		void Lower(const double & v1, const double & v2, const char* file, const int line) const {
			++m_numCHecks;

			if ( !(v1 < v2) ) {
				std::cout << std::setprecision(15)
						  << "LOWER failure\n"
							 "-------------\n"
							 "file: " << file << "\n"
							 "line: " << line << "\n"
							 "test: " << m_testName << "\n"
							 "value 1:\n" << v1 << "\n"
							 "value 2:\n" << v2 << "\n\n";
				m_numFailed++;
			}
		}

		void Within(const double & v1, const double & v2, const double & v3, const char* file, const int line) const {
			++m_numCHecks;

			if ( !(v1 > v2 && v1 < v3) ) {
				std::cout << std::setprecision(15)
						  << "WITHIN failure\n"
							 "--------------\n"
							 "file: " << file << "\n"
							 "line: " << line << "\n"
							 "test: " << m_testName << "\n"
							 "value 1:\n" << v1 << "\n"
							 "value 2:\n" << v2 << "\n"
							 "value 3:\n" << v3 << "\n\n";
				m_numFailed++;
			}
		}
		template<typename T> 
		void Assume_Exception(const std::function<void()> &fun, const std::string &expText, const char* file, const int line) const {
			++m_numCHecks;
			try {
				fun();
			}
			catch(const T & e) {
				if( expText != e.what()) {
					std::cout << std::setprecision(15)
							  << "ASSUME_EXCEPTION failure\n"
							  "--------------\n"
							  "file: " << file << "\n"
							  "line: " << line << "\n"
							  "test: " << m_testName << "\n"
							  "Test-Value:\n\""		<< expText << "\"\n"
							  "Excetion-Value:\n\"" << e.what() << "\"\n\n";
					m_numFailed++;
				}
			}
		}
	private:
		Test& operator=(const Test&);
	};
}

#define TEST(Name)                                        \
    struct Test##Name: alaska::Test                       \
    {                                                     \
        Test##Name(): Test(__FILE__, #Name, __LINE__) {}  \
                                                          \
        virtual void run() const;                         \
                                                          \
    } Test##Name##Instance;                               \
                                                          \
    void Test##Name::run() const

#define CHECK(Condition)                    \
    try                                     \
    {                                       \
        Check(Condition, __FILE__, __LINE__); \
    }                                       \
    catch (const std::exception & e)        \
    {                                       \
        std::cout << "exception within CHECK\n"				\
					 "----------------------\n"				\
					 "file: " << __FILE__ << "\n"           \
					 "line: " << __LINE__ << "\n"           \
					 "test: " << m_testName << "\n"         \
					 "text: " << #Condition << "\n"         \
					 "exception: " << e.what() << "\n\n";   \
        m_numFailed++;                          \
    }

#define EQUALS(v1, v2)                      \
    try                                     \
    {                                       \
        Equals(v1, v2, __FILE__, __LINE__); \
    }                                       \
    catch (const std::exception & e)        \
    {                                       \
        std::cout << "exception within EQUALS\n"		   \
					 "-----------------------\n"		   \
					 "file: " << __FILE__ << "\n"          \
					 "line: " << __LINE__ << "\n"          \
					 "test: " << m_testName << "\n"        \
					 "text 1:\n" << #v1 << "\n"            \
					 "text 2:\n" << #v2 << "\n"            \
					 "exception: " << e.what() << "\n\n";  \
        m_numFailed++;                          \
    }

#define LOWER(v1, v2)                       \
    try                                     \
    {                                       \
        Lower(v1, v2, __FILE__, __LINE__);  \
    }                                       \
    catch (const std::exception & e)        \
    {                                       \
        std::cout << "exception within LOWER\n"			   \
					 "----------------------\n"			   \
					 "file: " << __FILE__ << "\n"          \
					 "line: " << __LINE__ << "\n"          \
					 "test: " << m_testName << "\n"        \
					 "text 1:\n" << #v1 << "\n"            \
					 "text 2:\n" << #v2 << "\n"            \
					 "exception: " << e.what() << "\n\n";  \
        m_numFailed++;                          \
    }

#define WITHIN(v1, v2, v3)									\
    try														\
    {														\
        Within(v1, v2, v3, __FILE__, __LINE__);				\
    }														\
    catch (const std::exception & e)						\
    {														\
        std::cout << "exception within WITHIN\n"			\
					 "-----------------------\n"			\
					 "file: " << __FILE__ << "\n"			\
					 "line: " << __LINE__ << "\n"			\
					 "test: " << m_testName << "\n"			\
					 "text 1:\n" << #v1 << "\n"				\
					 "text 2:\n" << #v2 << "\n"				\
					 "text 3:\n" << #v3 << "\n"				\
					 "exception: " << e.what() << "\n\n";	\
        m_numFailed++;										\
    }

#define ASSUME_EXCEPTION(action, exptType, exptText)									\
	try {																				\
		Assume_Exception<exptType>( [&]() { action; }, exptText, __FILE__, __LINE__);	\
	}																					\
	catch (const std::exception & /*e*/) {												\
		std::cout << "unexpected exception within ASSUME_EXCEPTION\n"					\
					 "-----------------------\n"										\
					 "file: " << __FILE__ << "\n"										\
					 "line: " << __LINE__ << "\n"										\
					 "test: " << m_testName << "\n\n";									\
		m_numFailed++;																	\
	}

#endif
#endif
