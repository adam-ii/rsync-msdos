#!/usr/bin/env python3

import argparse
import enum
import logging
import os
import subprocess
import sys


class WatcomTool(enum.Enum):
    WCL = enum.auto()
    WCC = enum.auto()
    WLINK = enum.auto()


def select_tool(do_compile, do_link) -> WatcomTool:
    if do_compile:
        if do_link:
            return WatcomTool.WCL
        return WatcomTool.WCC
    elif do_link:
        return WatcomTool.WLINK
    return None


def run_cmd(cmd):
    print(' '.join(cmd))
    return subprocess.call(cmd)


def do_main():
    parser = argparse.ArgumentParser(description='OpenWatcom owcc replacement')
    parser.add_argument('-o', dest='output', metavar='<name>', type=str, help='set output file name')
    parser.add_argument('-c', dest='compile_only', action='store_true', help='compile only, no link')
    parser.add_argument('-b', dest='target', metavar='<target>', default='dos', help='compile and link for target')
    parser.add_argument('-v', dest='verbose', action='count', default=0, help='show sub program invocations')
    parser.add_argument('-g', dest='enable_debug', action='store_true', help='enable debugging information')

    parser.add_argument('-Wc,<option>', dest='wcc_option', metavar=' ', type=str, help='pass any option to WCC')

    parser.add_argument('-Wl,<option>', dest='wlink_option', metavar=' ', type=str, help='pass any directive to WLINK')
    parser.add_argument('-mstack-size', dest='stack_size', type=str, metavar='<stack_size>', default='12k', help='set stack size')

    parser.add_argument('--version', dest='version', action='store_true')
    args, unknown_args = parser.parse_known_args()

    if args.verbose > 1:
        logging.basicConfig(
            handlers=[logging.StreamHandler(sys.stdout)],
            level=max(0, logging.ERROR - 10 * args.verbose)
        )

    logging.debug('Unknown args: %s', unknown_args)

    if args.version:
        print('Open Watcom Driver for rsync')
        exit(0)

    # Extract filename arguments
    unknown_without_filenames = []
    src_files = []
    obj_files = []
    lib_files = []

    for unknown_arg in unknown_args:
        filtered = False
        if not unknown_arg.startswith('-'):
            lowercase_arg = unknown_arg.lower()
            if lowercase_arg.endswith('.c'):
                src_files += [unknown_arg]
                filtered = True
            elif lowercase_arg.endswith('.o') or lowercase_arg.endswith('.obj'):
                obj_files += [unknown_arg]
                filtered = True
            elif lowercase_arg.endswith('.lib'):
                lib_files += [unknown_arg]
                filtered = True

        if not filtered:
            unknown_without_filenames += [unknown_arg]

    logging.debug('Filtered src_files: %s', src_files)
    logging.debug('Filtered obj_files: %s', obj_files)
    logging.debug('Filtered lib_files: %s', lib_files)
    logging.debug('Remaining unknown args: %s', unknown_without_filenames)

    do_compile = args.compile_only or len(src_files)
    do_link = (not args.compile_only and len(src_files)) or len(obj_files)
    invoke_tool = select_tool(do_compile, do_link)

    if not invoke_tool:
        parser.print_help()
        exit(1)

    logging.debug('invoke_tool: %s, do_compile: %s, do_link: %s', invoke_tool, 'yes' if do_compile else 'no', 'yes' if do_link else 'no')

    # Could extract the tool names from OpenWatcom's specs.owc
    target_system = args.target
    cmd = None

    # Settings for DOS rsync build
    # TODO: add --with-watt32=dir to configure
    WATCOM_DIR = os.environ.get('WATCOM', None)
    WATT32_DIR = os.environ.get('WATT32_DIR', None)

    missing_env = False
    if not WATCOM_DIR:
        print('Environment variable "WATCOM" is not set')
        missing_env = True
    if not WATT32_DIR:
        print('Environment variable "WATT32_DIR" is not set')
        missing_env = True
    if missing_env:
        exit(1)

    WATCOM_INC = os.path.join(WATCOM_DIR, 'h')
    WATT32_INC = os.path.join(WATT32_DIR, 'inc')
    WATT32_LIB = os.path.join(WATT32_DIR, 'lib', 'wattcpwl.lib')

    # Build the command line for each tool
    if invoke_tool == WatcomTool.WCL:
        # For the configure step "checking whether the C compiler works"
        cmd = [
            'wcl',
            f'-bt={target_system}',     # compile for target OS
        ]

        if args.output:
            cmd += [
                f'-fe={args.output}'    # name executable file
            ]

        if args.stack_size:
            cmd += [
                f'-k{args.stack_size}'  # set stack size
            ]
    elif invoke_tool == WatcomTool.WCC:
        cmd = [
            'wcc', 
            '-q',                   # operate quietly
            f'-bt={target_system}', # build target for operating system
            '-wx',                  # set warning level to maximum setting
        ]

        if args.output:
            cmd += [
                f'-fo={args.output}'    # set object or preprocessor output file name
            ]

        if target_system == 'dos':
            cmd += [
                '-0',   # 8086 instructions
                '-ml',  # large memory model (large code/large data)
                '-fpc', # calls to floating-point library
                '-zu',  # SS != DGROUP
                '-zm',  # place functions in separate segments
            ]

        if args.enable_debug:
            cmd += [
                '-od',  # disable optimizations
                '-d2',  # full symbolic debugging info.
            ]
        else:
            cmd += [
                '-d0',  # no debugging information
                '-os',  # optimize for space
                '-s',   # remove stack overflow checks
                '-zc',  # place strings in CODE segment
                '-zl',  # remove default library information
                '-zld', # remove file dependency information
            ]
    elif invoke_tool == WatcomTool.WLINK:
        cmd = [
            'wlink',
            'system', target_system,
            'option', 'caseexact',
            'option', 'map',
            'option', 'eliminate',
            'option', 'quiet',
            'option', 'noextension',
        ]

        if args.output:
            cmd += [
                'name', args.output
            ]

        if target_system == 'dos':
            cmd += [
                'option', 'dosseg'
            ]

        if args.stack_size:
            cmd += [
                'option', f'stack={args.stack_size}'
            ]

        if args.enable_debug:
            cmd += [
                'debug', 'dwarf'
            ]

    # Append pass-through directives
    for unknown_arg in unknown_without_filenames:
        filtered = False

        if unknown_arg.startswith('-Wc,'):
            if invoke_tool == WatcomTool.WCC:
                split = unknown_arg.split(',')
                cmd += split[1:]
                logging.debug('WCC directives: %s]', split[1:])
            filtered = True
        elif unknown_arg.startswith('-Wl,'):
            if invoke_tool == WatcomTool.WLINK:
                split = unknown_arg.split(',')
                cmd += split[1:]
                logging.debug('WLINK directives: %s]', split[1:])
            filtered = True
        else:
            if unknown_arg.startswith('-') and invoke_tool == WatcomTool.WLINK:
                # Do not pass compile flags to wlink
                filtered = True

        if not filtered:
            cmd += [unknown_arg]

    # Append DOS rsync build settings
    if invoke_tool == WatcomTool.WCL or invoke_tool == WatcomTool.WCC:
        cmd += [
            '-DNOSHELLORSERVER',
            f'-I{WATT32_INC}',
            f'-I{WATCOM_INC}',
        ]
    if invoke_tool == WatcomTool.WCL or invoke_tool == WatcomTool.WLINK:
        lib_files += [WATT32_LIB]

    # Append filenames
    if invoke_tool == WatcomTool.WCL:
        cmd += src_files + obj_files + lib_files
    elif invoke_tool == WatcomTool.WCC:
        cmd += src_files
    elif invoke_tool == WatcomTool.WLINK:
        for i in obj_files:
            cmd += ['file', i]
        for i in lib_files:
            cmd += ['library', i]

    if args.verbose == 1:
        print(cmd)
    else:
        logging.debug('%s', cmd)

    exit(run_cmd(cmd))


if __name__ == '__main__':
    do_main()
