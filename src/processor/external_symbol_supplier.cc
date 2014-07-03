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

std::string ShellEscape(const std::string& arg)
{
  std::string result = "'";
  for (int i=0; i < arg.size(); i++) {
    if (arg[i] == '\'') {
      result += '\\';
    }
    result += arg[i];
  }
  result += "'";
  return result;
}

// Returns the part of 'path' following the final trailing slash.
//
// To support both Windows and Unix minidump paths, both '\' and '/'
// are considered path component separators.
std::string FileBasename(const std::string& path)
{
  int basename_start_pos = path.size();
  while (basename_start_pos > 0) {
    if (path[basename_start_pos-1] == '/' ||
        path[basename_start_pos-1] == '\\') {
      break;
    }
    --basename_start_pos;
  }
  return path.substr(basename_start_pos);
}

SymbolSupplier::SymbolResult ExternalSymbolSupplier::GetCStringSymbolData(const CodeModule *module,
                                       const SystemInfo *system_info,
                                       string *symbol_file,
                                       char **symbol_data) {
  // search for already-loaded debug info
  map<string,string>::const_iterator it = symbol_cache_.find(module->code_file());
  if (it != symbol_cache_.end()) {
    const std::string& content = it->second;
    if (content.empty()) {
      // debug info has been requested before but was not found previously
      return NOT_FOUND;
    } else {
      *symbol_data = const_cast<char*>(content.data());
      return FOUND;
    }
  }

  // run external command to fetch debug info
  std::string debug_file_basename = FileBasename(module->debug_file());
  stringstream symbol_content;
  stringstream fetch_command;
  fetch_command << symbol_fetch_command_ << " " << ShellEscape(debug_file_basename) << " " << ShellEscape(module->debug_identifier());
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
    // no matching debug info found,
    // cache the omission to avoid repeated lookups for the same module
    symbol_cache_[module->code_file()] = std::string();
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

