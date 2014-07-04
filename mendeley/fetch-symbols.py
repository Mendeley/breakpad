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

def main():
    parser = argparse.ArgumentParser(
"""Fetch symbol files for a binary from Mendeley Desktop's symbol server and cache them locally.
""")
    parser.add_argument('-s', type=str,
      action='append',
      dest='symbol_servers',
      help='Add a symbol server to search for available symbols')
    parser.add_argument('binary_name', type=str, help='The file name (excluding the path) of the executable or shared library', action='store')
    parser.add_argument('debug_id', type=str, help='The debug/build identifier for the version of the binary referenced in a minidump', action='store')
    opts = parser.parse_args()

    binary_name = opts.binary_name
    debug_id = opts.debug_id
    binary_basename, binary_ext = os.path.splitext(binary_name)

    rel_path = '%s/%s/%s.sym' % (binary_name, debug_id, binary_basename)
    cache_path = '%s/%s' % (CACHE_ROOT, rel_path)

    if os.path.exists(cache_path):
        print('Found %s in cache' % rel_path, file=sys.stderr)
        cache_file = open(cache_path, 'r')
        print(cache_file.read())
        sys.exit(0)

    for server in opts.symbol_servers:
        symbol_url = '%s/%s' % (server, rel_path)

        print('Symbols for %s not found in cache, fetching from %s' % (binary_name, symbol_url), file=sys.stderr)

        try:
            url_req = urllib2.Request(symbol_url)
            url_reply = urllib2.urlopen(url_req)
            data = url_reply.read()
            print('Successfully retrieved symbols', file=sys.stderr)
            print(data, file=sys.stdout)

            # save to local cache for future use
            mkpath(os.path.dirname(cache_path))
            cache_file = open(cache_path, 'w')
            cache_file.write(data)
            cache_file.close()

        except urllib2.HTTPError as err:
            print('Symbols not found for %s: %s' % (binary_name, err), file=sys.stderr)
            sys.exit(1)

if __name__ == '__main__':
    main()

