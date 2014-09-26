# Breakpad

This is a fork of [Google Breakpad](https://code.google.com/p/google-breakpad/),
a multi-platform crash reporting system, which is used by [Mendeley Desktop](http://www.mendeley.com/download-mendeley-desktop)
under Windows, Mac and Linux.

Mendeley's additions to breakpad include:

* A CMake-based build system for the crash capturing client library, debug symbol
extraction and stacktrace output (minidump_stackwalk) tools.

* Support in minidump_stackwalk for fetching debug symbols from arbitrary sources
by invoking a user-provided command instead of looking in a specific local filesystem
directory. We use this to fetch debug symbols on-demand from an archive hosted in S3.

* A python script which fetches debug symbols from a symbol server with a given HTTP URL

* A simple multi-platform end-to-end test that builds a buggy app with the crash capturing library
installed, extracts debug symbols from it, runs the test app and symbolizes the resulting
crash dump.

* Compatibility with C++11

## Building Breakpad

````
git clone https://github.com/Mendeley/breakpad.git
mkdir breakpad-build
cd breakpad-build
cmake ../breakpad/mendeley
make
````

## Usage

The overall flow for capturing crashes and debugging them as follows:

1. Build the breakpad tools and libraries
2. Link the breakpad library with your application
3. Early in your app's startup code, create an instance of `google_breakpad::ExceptionHandler`. This
   is defined separately for each platform in `client/<platform>/handler/exception_handler.h`
4. When the app crashes, it will write a .dmp file to the directory specified when the `ExceptionHandler`
   object was created.
5. When building a release build of your app, run the dump_syms tool on the generated DLLs and binaries
   to produce .sym files which contain mappings from program locations to source locations.
6. Upload the .sym files to a location which is accessible via a HTTP URL. See [this StackOverflow comment](http://stackoverflow.com/questions/5278997/setting-up-a-public-or-private-symbol-server-over-http/23614715#23614715) for details of the expected structure of the symbol server
7. When your app crashes on a user's system, get the .dmp file and use minidump_stackwalk to produce a stacktrace
   from the .dmp file.

You can also debug the .dmp files in Visual Studio on Windows.

## Getting a stacktrace from a minidump

When you have a .dmp file captured by the breakpad library after an application crashes and have
uploaded it to a location accessible via a HTTP URL, you can use the minidump_stackwalk tool to extract
a stack trace from the minidump.

````
./minidump_stackwalk -m <path to .dmp file> -e '../breakpad/mendeley/fetch-symbols.py -s <URL of your symbol server>'
````
