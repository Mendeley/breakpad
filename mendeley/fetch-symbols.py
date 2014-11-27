#!/usr/bin/env python

# This is a script to fetch debug symbols for a binary from
# a symbol server with an HTTP endpoint, for use with
# minidump_stackwalk's '-e' option which runs an external
# command to fetch debug symbols needed to generate a stacktrace
# from a minidump.
#
# Given a binary name and build ID, it will fetch symbols
# from $SERVER_URL/$BINARY_NAME/$BUILD_ID/$BINARY_BASENAME.sym
# and cache them locally in a temporary directory to speed
# up future requests.

from __future__ import print_function
from distutils.dir_util import mkpath

import argparse
import urllib2
import os
import tempfile
import time
import sys

# path to local symbol cache for faster retrieval in future
CACHE_ROOT = '%s/%s' % (tempfile.gettempdir(), 'symbol-cache')

# TODO - For Windows binaries, attempt to fetch from the Microsoft symbol server
# if symbols are not found in our own symbol server.
#
# The Microsoft symbol server only has .pdb files which need to be processed to
# generate .sym files that can be used by minidump_stackwalk
#
# Note for when we come to implement this, HTTP requests to the MSFT symbol
# server must have the user agent 'Microsoft-Symbol-Server' followed
# by a version number which, at the time of testing, has to be >= 6.3.
# Otherwise the server responds with a redirect to a non-existent URL.
#
# root URL for Microsoft's symbol server.
# See http://support.microsoft.com/kb/311503
# MSFT_SYMBOL_SERVER_URL = 'http://msdl.microsoft.com/download/symbols'
# MSFT_SYMBOL_STORE_USER_AGENT = "Microsoft-Symbol-Server/10.0.0.0"

# Length of time to remember failed cache lookups for in seconds
MAX_MISSING_CACHE_AGE = 300

def main():
    parser = argparse.ArgumentParser(
"""Fetch symbol files for a binary from an HTTP symbol server and cache them locally.
""")
    parser.add_argument('-s', type=str,
      action='append',
      dest='symbol_servers',
      help='Add a symbol server to search for available symbols',
      required=True)
    parser.add_argument('debug_file_name', type=str, help='The file name (excluding the path) of the file (PDB on Windows, shared library or executable on other platforms) which the debug info was extracted from.', action='store')
    parser.add_argument('debug_id', type=str, help='The debug/build identifier for the version of the binary referenced in a minidump', action='store')
    opts = parser.parse_args()

    debug_file_name = opts.debug_file_name
    debug_id = opts.debug_id

    if debug_file_name.endswith('.pdb'):
        symfile_name = debug_file_name[0:-4] + '.sym'
    else:
        symfile_name = debug_file_name + '.sym'

    rel_path = '%s/%s/%s' % (debug_file_name, debug_id, symfile_name)
    cache_path = '%s/%s' % (CACHE_ROOT, rel_path)
    missing_entry_cache_path = '%s.failed' % cache_path

    if os.path.exists(cache_path):
        print('Found %s in cache' % rel_path, file=sys.stderr)
        cache_file = open(cache_path, 'r')
        print(cache_file.read())
        sys.exit(0)

    for server in opts.symbol_servers:
        symbol_url = '%s/%s' % (server, urllib2.quote(rel_path))

        if os.path.isfile(missing_entry_cache_path):
            cache_age = time.time() - os.path.getmtime(missing_entry_cache_path)
            if cache_age < MAX_MISSING_CACHE_AGE:
               print('Symbols for %s not found in cache but failed lookup cached %d seconds ago' % (debug_file_name, cache_age), file=sys.stderr)
               continue

        print('Symbols for %s not found in cache, fetching from %s' % (debug_file_name, symbol_url), file=sys.stderr)
        
        mkpath(os.path.dirname(cache_path))

        try:
            url_req = urllib2.Request(symbol_url)
            url_reply = urllib2.urlopen(url_req)
            data = url_reply.read()
            print('Successfully retrieved symbols', file=sys.stderr)
            print('Saving to cache as %s' % (cache_path), file=sys.stderr)
            print(data, file=sys.stdout)

            # save to local cache for future use
            cache_file = open(cache_path, 'w')
            cache_file.write(data)
            cache_file.close()

            if os.path.isfile(missing_entry_cache_path):
                os.remove(missing_entry_cache_path)

        except urllib2.HTTPError as err:
            print('Symbols not found for %s: %s' % (debug_file_name, err), file=sys.stderr)
            print('Caching miss in %s' % (missing_entry_cache_path), file=sys.stderr)

            symbols_missing_file = open(missing_entry_cache_path, 'w')
            symbols_missing_file.write(symbol_url)
            symbols_missing_file.close()

            sys.exit(1)

if __name__ == '__main__':
    main()

