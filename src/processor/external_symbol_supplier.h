#pragma once

#include <map>
#include <string>

#include "google_breakpad/processor/symbol_supplier.h"

namespace google_breakpad {

using std::map;
using std::string;

// Fetches symbol data by running external command, supplying
// the debug ID and binary name of the module to fetch symbols for, ie.
// <fetch command> <binary name> <debug ID>
//
// The external command should write the symbol file data to stdout
// and exit with a zero status if found or exit with a non-zero
// status if symbols could not be found for the given binary.
//
class ExternalSymbolSupplier : public SymbolSupplier {
	public:

  // Construct an ExternalSymbolSupplier which runs fetch_command
  // to retreive debug symbols for a code module
	ExternalSymbolSupplier(const string& fetch_command);

  // implements SymbolSupplier.

  // the GetSymbolFile() functions are non-implemented stubs.
  // minidump_stackwalk only uses the GetCStringSymbolData() function
	virtual SymbolResult GetSymbolFile(const CodeModule *module,
	                                   const SystemInfo *system_info,
	                                   string *symbol_file);
	virtual SymbolResult GetSymbolFile(const CodeModule *module,
	                                   const SystemInfo *system_info,
                                     string *symbol_file,
	                                   string *symbol_data);

	virtual SymbolResult GetCStringSymbolData(const CodeModule *module,
	                                   const SystemInfo *system_info,
	                                   string *symbol_file,
	                                   char **symbol_data);
	virtual void FreeSymbolData(const CodeModule *module);

	private:
		// external command to run to locate the symbol file
		string symbol_fetch_command_;

		// and return its contents
		// map from binary filename to content
		map<string, string> symbol_cache_;
};

}

