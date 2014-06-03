# Breakpad

This is a fork of [Google Breakpad](https://code.google.com/p/google-breakpad/),
a multi-platform crash reporting system, which is used by [Mendeley Desktop](http://www.mendeley.com/download-mendeley-desktop)
under Windows, Mac and Linux.

Mendeley's additions to breakpad include:

* A CMake-based build system for the crash capturing client library, debug symbol
extraction and stacktrace output (minidump_stackwalk) tools.

* Support in minidump_stackwalk for fetching debug symbols from arbitrary sources
by invoking a user-provided command instead of looking in a specific local filesystem
directory. We use this to fetch debug symbols on-demand from an archive in S3.

* A simple multi-platform end-to-end test that builds a buggy app with the crash capturing library
installed, extracts debug symbols from it, runs the test app and symbolizes the resulting
crash dump.
