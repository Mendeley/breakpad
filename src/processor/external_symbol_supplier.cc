#include "processor/external_symbol_supplier.h"

#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "google_breakpad/processor/code_module.h"
#include "processor/logging.h"

using std::stringstream;

namespace google_breakpad {

ExternalSymbolSupplier::ExternalSymbolSupplier(const string& fetch_command)
  : symbol_fetch_command_(fetch_command) {
}

SymbolSupplier::SymbolResult ExternalSymbolSupplier::GetSymbolFile(const CodeModule *module,
                                       const SystemInfo *system_info,
                                       string *symbol_file) {
  BPLOG_ERROR << "GetSymbolFile() is not implemented";
  return INTERRUPT;
}

SymbolSupplier::SymbolResult ExternalSymbolSupplier::GetSymbolFile(const CodeModule *module,
                                       const SystemInfo *system_info,
                                       string *symbol_file,
                                       string *symbol_data) {
  BPLOG_ERROR << "GetSymbolFile() is not implemented";
  return INTERRUPT;
}

SymbolSupplier::SymbolResult ExternalSymbolSupplier::GetCStringSymbolData(const CodeModule *module,
                                       const SystemInfo *system_info,
                                       string *symbol_file,
                                       char **symbol_data) {
  // search for already-loaded debug info
  map<string,string>::const_iterator it = symbol_cache_.find(module->code_file());
  if (it != symbol_cache_.end()) {
    *symbol_data = const_cast<char*>(symbol_cache_[module->code_file()].data());
    return FOUND;
  }

  // run external command to fetch debug info
  stringstream symbol_content;
  stringstream fetch_command;
  fetch_command << symbol_fetch_command_ << " " << module->debug_file() << " " << module->debug_identifier();
  FILE *child_proc = popen(fetch_command.str().data(), "r");
  if (!child_proc) {
    BPLOG_ERROR << "Failed to start symbol fetcher " << fetch_command.str();
    return INTERRUPT;
  }

  const int BUF_SIZE = 4096;
  char buffer[BUF_SIZE];
  while (!feof(child_proc)) {
    size_t nread = fread(buffer, 1, BUF_SIZE, child_proc);
    symbol_content.write(buffer, nread);
  }
  int status = pclose(child_proc);
  if (!WIFEXITED(status)) {
    BPLOG_INFO << fetch_command.str() << " failed";
    return INTERRUPT;
  }

  int exitCode = WEXITSTATUS(status);
  if (exitCode == 127) {
    // command not found
    BPLOG_INFO << "Failed to run symbol fetch command: " << fetch_command.str();
    return INTERRUPT;
  }

  if (exitCode != 0) {
    // no matching debug info found
    BPLOG_INFO << "No symbols found with " << fetch_command.str() << " (status: " << exitCode << ")";
    return NOT_FOUND;
  }

  // cache and return debug info
  symbol_cache_[module->code_file()] = symbol_content.str();
  return GetCStringSymbolData(module, system_info, symbol_file, symbol_data);
}

void ExternalSymbolSupplier::FreeSymbolData(const CodeModule *module) {
  map<string,string>::iterator it = symbol_cache_.find(module->code_file());
  if (it != symbol_cache_.end()) {
    symbol_cache_.erase(it);
  }
}

}

