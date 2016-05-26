#!/usr/bin/env python

from __future__ import print_function

import argparse
import glob
import os
import subprocess
import sys

IS_MAC = sys.platform == 'darwin'

def main():
    parser = argparse.ArgumentParser('Stacktrace dump test')
    parser.add_argument('--symbols', help='Dump symbols for binary', action='store_true')
    parser.add_argument('binary_name', type=str, nargs='?', action='store', default=None)
    parser.add_argument('debug_id', type=str, nargs='?', action='store', default=None)
    opts = parser.parse_args()

    buggy_app_path = './buggy_app'
    dump_syms_path = '../dump_syms'
    stackwalk_path = '../minidump_stackwalk'

    if opts.symbols:
        sym_file = opts.binary_name + '.sym'
        if os.path.exists(sym_file):
            print(open(sym_file, 'r').read())
            sys.exit(0)
        else:
            print('No symbols found for %s' % (opts.binary_name))
            sys.exit(1)
    else:
        dump_files = glob.glob('*.dmp')
        for existing_dump in dump_files:
            os.remove(existing_dump)

        symfile = open('buggy_app.sym', 'w')
        stackfile = open('stacktrace.txt', 'w+')
        minidump_log = open('stackwalk-log.txt', 'w')

        dump_syms_platform_options = []
        if IS_MAC:
            # See utilities/ci/debug_symbols.py and/or buildtool/debug_symbols.py:
            # on OS-X we only dump x86_64 symbols.
            dump_syms_platform_options = ['-a', 'x86_64']

        subprocess.call([dump_syms_path] + dump_syms_platform_options + [buggy_app_path], stdout=symfile)
        subprocess.call([buggy_app_path])

        dump_files = glob.glob('*.dmp')
        if len(dump_files) < 1:
            print('No minidump file created', file=sys.stderr)
            sys.exit(1)

        subprocess.call([stackwalk_path, '-m', '-e', '%s --symbols ' % os.path.realpath(__file__), dump_files[0]], stdout=stackfile, stderr=minidump_log)
        stackfile.seek(0)
        stack_content = stackfile.read()

        if len(stack_content) == 0:
            print('Failed to generate a stack trace', file=sys.stderr)
            sys.exit(1)

        stack_trace = []

        # read stack trace output. Each line is a pipe-delimited list
        # where the first entry specifies the type of info in that line
        # and the remaining elements are type-specific attributes.
        #
        # Stack trace lines have the form [$THREAD_ID, $FRAME_ID, $MODULE_NAME, $FUNCTION_NAME, ... $OFFSET]
        for line in stack_content.splitlines():
            info = line.split('|')
            if info[0] == '0':
                stack_trace += [info[3]]

        if len(stack_trace) == 0:
            print('No stack trace found in output', file=sys.stderr)
            sys.exit(1)

        buggy_function_name = 'aBuggyFunction'
        if not (buggy_function_name in stack_trace[0]):
            print('Crashing function was %s, expected %s' % (stack_trace[0], buggy_function_name))
            sys.exit(1)

        print('Stack trace OK')

if __name__ == '__main__':
    main()
