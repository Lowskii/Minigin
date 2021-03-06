#include "MiniginPCH.h"
#include "Logger.h"
#include <ctime>
#include <io.h>
#include <fcntl.h>
#include <assert.h>


__int64 minigin::Logger::m_PerformanceTimerArr[] = { 0 };
double minigin::Logger::m_PcFreq = 0.0;
HANDLE minigin::Logger::m_ConsoleHandle = nullptr;

minigin::Logger::ConsoleLogger* minigin::Logger::m_pConsoleLogger = nullptr;
minigin::Logger::FileLogger* minigin::Logger::m_pFileLogger = nullptr;
char minigin::Logger::m_BreakBitField = static_cast<char>(LogLevel::Error);
wchar_t* minigin::Logger::m_pConvertBuffer = new wchar_t[m_ConvertBufferSize];

void minigin::Logger::Initialize()
{
	for (int i = 0; i < MAX_PERFORMANCE_TIMERS; ++i)
		m_PerformanceTimerArr[i] = -1;

	LARGE_INTEGER li;
	if (QueryPerformanceFrequency(&li))
	{
		m_PcFreq = double(li.QuadPart) / 1000.0;
		//LOG ERROR
	}

#if defined(DEBUG) | defined(_DEBUG)
	{
		// Redirect the CRT standard input, output, and error handles to the console
		FILE* pCout;
		freopen_s(&pCout, "CONIN$", "r", stdin);
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		freopen_s(&pCout, "CONOUT$", "w", stderr);

		//Clear the error state for each of the C++ standard stream objects. We need to do this, as
		//attempts to access the standard streams before they refer to a valid target will cause the
		//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
		//to always occur during startup regardless of whether anything has been read from or written to
		//the console or not.
		wcout.clear();
		cout.clear();
		wcerr.clear();
		cerr.clear();
		wcin.clear();
		cin.clear();
		cin.clear();

		//Set ConsoleHandle
		m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		m_pConsoleLogger = new ConsoleLogger();

		//Disable Close-Button
		HWND hwnd = GetConsoleWindow();
		if (hwnd != NULL)
		{
			HMENU hMenu = GetSystemMenu(hwnd, FALSE);
			if (hMenu != NULL) DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
		}
	}
#endif
}

void minigin::Logger::Release()
{
	ReleaseLoggers();

	delete[] m_pConvertBuffer;
	m_pConvertBuffer = nullptr;
}

int minigin::Logger::StartPerformanceTimer()
{
	int counter = 0;
	while (m_PerformanceTimerArr[counter] != -1)
	{
		++counter;
		if (counter == MAX_PERFORMANCE_TIMERS)
		{
			counter = -1;
			break;
		}
	}

	if (counter >= 0)
	{
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		m_PerformanceTimerArr[counter] = li.QuadPart;
	}

	return counter;
}

double minigin::Logger::StopPerformanceTimer(int timerId)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	double diff = double(li.QuadPart - m_PerformanceTimerArr[timerId]) / m_PcFreq;

	m_PerformanceTimerArr[timerId] = -1;

	return diff;
}

void minigin::Logger::StartFileLogging(const wstring& fileName)
{
	delete m_pFileLogger;

	m_pFileLogger = new FileLogger(fileName);
}

void minigin::Logger::StopFileLogging()
{
	delete m_pFileLogger;
}

void minigin::Logger::LogInfo(const wstring& msg, bool includeTimeStamp)
{
	Log(LogLevel::Info, msg, includeTimeStamp);
}

void minigin::Logger::LogWarning(const wstring& msg, bool includeTimeStamp)
{
	Log(LogLevel::Warning, msg, includeTimeStamp);
}

void minigin::Logger::LogError(const wstring& msg, bool includeTimeStamp)
{
	Log(LogLevel::Error, msg, includeTimeStamp);
}

void minigin::Logger::LogFixMe(const wstring& source, bool includeTimeStamp)
{
	Log(LogLevel::FixMe, source, includeTimeStamp);
}

bool minigin::Logger::LogHResult(HRESULT hr, const wstring& origin, bool includeTimeStamp)
{
	if (FAILED(hr))
	{
		if (FACILITY_WINDOWS == HRESULT_FACILITY(hr))
			hr = HRESULT_CODE(hr);

		wstringstream ss;
		if (origin.size() != 0)
		{
			ss << ">ORIGIN:\n";
			ss << origin;
			ss << "\n\n";
		}
		ss << "ERROR:\n";

		TCHAR* errorMsg;
		if (FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&errorMsg, 0, NULL) != 0)
		{
			ss << errorMsg;
		}
		else
		{
			ss << "Could not find a description for error ";
			ss << hr;
			ss << ".";
		}

		Log(LogLevel::Error, ss.str(), includeTimeStamp);
		return true;
	}

	return false;
}

void minigin::Logger::LogFormat(LogLevel level, const wchar_t* format, ...)
{
	va_list ap;

	va_start(ap, format);
	_vsnwprintf_s(&m_pConvertBuffer[0], m_ConvertBufferSize, m_ConvertBufferSize, format, ap);
	va_end(ap);
	Log(level, wstring(&m_pConvertBuffer[0]));
}

void minigin::Logger::Log(LogLevel level, const wstring& msg, bool includeTimeStamp)
{
	wstringstream stream;

	if (includeTimeStamp)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		stream << L"[" << st.wYear << L"-" << st.wMonth << L"-" << st.wDay << L" - ";
		stream << st.wHour << L":" << st.wMinute << L":" << st.wSecond << L":" << st.wMilliseconds << L"]";
	}

	switch (level)
	{
	case LogLevel::Info:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		stream << L"[INFO]    ";
		break;
	case LogLevel::Warning:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		stream << L"[WARNING] ";
		break;
	case LogLevel::Error:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_RED);
		stream << L"[ERROR]   ";
		break;
	case LogLevel::FixMe:
		SetConsoleTextAttribute(m_ConsoleHandle, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE);
		stream << L"[FIX-ME]   ";
		break;
	}

	stream << msg;
	stream << L"\n";

	//Use specific loggers to log
	if (m_pConsoleLogger)
		m_pConsoleLogger->Log(stream.str());
	if (m_pFileLogger)
		m_pFileLogger->Log(stream.str());

	//if error, break
	if (level == LogLevel::Error)
	{
		MessageBox(0, msg.c_str(), L"ERROR", 0);
	}

	if ((m_BreakBitField & static_cast<char>(level)) == static_cast<char>(level)) assert(false);
}
