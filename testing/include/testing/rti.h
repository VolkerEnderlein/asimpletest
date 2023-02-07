/**
========================================================================
	Institute of Mechatronics e.V.
	Reichenhainer Str. 88
	D-09126 Chemnitz (Germany)

	run time information structures
========================================================================
*/
#ifndef RTI_H__
#define RTI_H__

#ifdef WIN32
#define USE_WINDOWS_THREAD_TIME
#else // WIN32
#undef USE_WINDOWS_THREAD_TIME
#endif // WIN32

#ifdef USE_WINDOWS_THREAD_TIME
  #include <windows.h>
  #include <memory.h>
#else // USE_WINDOWS_THREAD_TIME
  #include <time.h>
#endif // USE_WINDOWS_THREAD_TIME

#ifdef USE_WINDOWS_THREAD_TIME

namespace alaska
{
	class CRuntimeInfo
	{
	public:
		CRuntimeInfo()
		{
			LARGE_INTEGER tmp = {0};
			kernelTime = tmp;
			userTime = tmp;
			timeDifference = 0;
		}
		virtual ~CRuntimeInfo() {}

		bool StartRuntimeMeasurement()
		{
			FILETIME crt, ext, krt, ust;
			if ( ! GetThreadTimes(GetCurrentThread(), &crt, &ext, &krt, &ust) )
				return false;

			memcpy(&kernelTime, &krt, sizeof(krt));
			memcpy(&userTime, &ust, sizeof(ust));

			return true;
		}
		bool StopRuntimeMeasurement()
		{
			FILETIME crt, ext, krt, ust;
			if ( ! GetThreadTimes(GetCurrentThread(), &crt, &ext, &krt, &ust) )
				return false;

			LARGE_INTEGER krnlTime, usrTime;
			memcpy(&krnlTime, &krt, sizeof(krt));
			memcpy(&usrTime, &ust, sizeof(ust));

			timeDifference = ((krnlTime.QuadPart - kernelTime.QuadPart) 
							+ (usrTime.QuadPart - userTime.QuadPart)) * 1e-7;
			return true;
		}
		double GetTimeDifference() const { return timeDifference; }

	private:
		double timeDifference;	// time difference from start to get in seconds

		LARGE_INTEGER kernelTime;
		LARGE_INTEGER userTime;
	};
}

#else // USE_WINDOWS_THREAD_TIME

class CRuntimeInfo
{
public:
	CRuntimeInfo()
	{
		clockTime = 0;
		timeDifference = 0;
	}
	virtual ~CRuntimeInfo() {}

	bool StartRuntimeMeasurement()
	{
		clockTime = clock();

		if ( -1 == clockTime )
			return false;

		return true;
	}
	
	bool StopRuntimeMeasurement()
	{
		clock_t clkTime = clock();

		if ( -1 == clkTime )
			return false;

		timeDifference = (double)(clkTime - clockTime) / CLOCKS_PER_SEC;

		return true;
	}

	double GetTimeDifference() const { return timeDifference; }

private:
	double timeDifference;	// time difference from start to get in seconds

	clock_t clockTime;
};

#endif // USE_WINDOWS_THREAD_TIME

#endif // RTI_H__
