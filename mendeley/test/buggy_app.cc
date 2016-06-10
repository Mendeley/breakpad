#include <string>

#if defined(__linux)
#include "client/linux/handler/exception_handler.h"
#elif defined(__APPLE__)
#include "client/mac/handler/exception_handler.h"
#elif defined(WIN32)
#include "client/windows/handler/exception_handler.h"
#include <locale>
#include <codecvt>
#endif

using std::string;

void setupBreakpad(const string& outputDirectory) {
	google_breakpad::ExceptionHandler *exception_handler;

#if defined(__linux)
	exception_handler = new google_breakpad::ExceptionHandler(
		outputDirectory, /* minidump output directory */
		0,   /* filter */
		0,   /* minidump callback */
		0,   /* callback_context */
		true /* install_handler */
	);
#elif defined(__APPLE__)
	exception_handler = new google_breakpad::ExceptionHandler(
		outputDirectory, /* minidump output directory */
		0,   /* filter */
		0,   /* minidump callback */
		0,   /* callback_context */
		true, /* install_handler */
		0    /* port name, set to null so in-process dump generation is used. */
	);
#elif defined(WIN32)
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> strConv;
	exception_handler = new google_breakpad::ExceptionHandler(
		strConv.from_bytes(outputDirectory), /* minidump output directory */
		0,   /* filter */
		0,   /* minidump callback */
		0,   /* calback_context */
		google_breakpad::ExceptionHandler::HANDLER_ALL /* handler_types */
	);

	// call TerminateProcess() to prevent any further code from
	// executing once a minidump file has been written following a
	// crash.  See ticket #17814
	exception_handler->set_terminate_on_unhandled_exception(true);
#endif
}

// This variable is NOT used - it only exists to avoid
// the compiler to inline the function aBuggyFunction
// so we can have a full backtrace.
int avoidInlineFunction = 1;

void aBuggyFunction() {
	if (avoidInlineFunction == 2)
	{
		// It never uses this code path, it only exists to avoid
		// this function to be inlined.
		aBuggyFunction();
	}
	int* invalid_ptr = reinterpret_cast<int*>(0x42);
	*invalid_ptr = 0xdeadbeef;
}

int main(int, char**) {
	setupBreakpad(".");
	aBuggyFunction();
	return 0;
}
