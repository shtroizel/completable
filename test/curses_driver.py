#!/usr/bin/env python3

'''
Runs completable in a pseudo terminal, sends keys, and checks the rendered screen.

usage: curses_driver.py <completable> [completable args...] -- <step>...

steps:
    key:<name>            send a named key (see KEYS)
    text:<string>         send literal text
    wait:<regex>          wait until some screen line matches
    expect:<row>:<regex>  screen row <row> (* for any row) must match now
    remember:<row>        save screen row <row>
    same:<row>            screen row <row> must equal the saved row
    sh:<command>          run a shell command
    show                  print the screen

Address space randomization is off for completable. After the last step the driver waits for completable
to exit and fails unless it exits with 0.
Exits with 77 (skip) when the pyte module is missing.
'''

import ctypes
import fcntl
import os
import pty
import re
import select
import struct
import subprocess
import sys
import termios
import time

try:
    import pyte
except ImportError:
    print('pyte not found, skipping')
    sys.exit(77)


KEYS = {
    'RET': b'\r',
    'DEL': b'\x1b[3~',
    'UP': b'\x1bOA',
    'DOWN': b'\x1bOB',
    'RIGHT': b'\x1bOC',
    'LEFT': b'\x1bOD',
    'TAB': b'\t',
    'F1': b'\x1bOP',
    'ESC': b'\x1b',
    'SHIFT_LEFT': b'\x1b[1;2D',
    'SHIFT_RIGHT': b'\x1b[1;2C',
}
ROWS, COLS = 40, 140
ADDR_NO_RANDOMIZE = 0x0040000  # ThreadSanitizer builds fail with full ASLR on newer kernels
TIMEOUT = 120


class Session:
    def __init__(self, argv):
        self.screen = pyte.Screen(COLS, ROWS)
        self.stream = pyte.Stream(self.screen)
        self.open = True
        self.pid, self.fd = pty.fork()
        if self.pid == 0:
            ctypes.CDLL(None).personality(ADDR_NO_RANDOMIZE)
            os.environ.update(TERM='xterm', ESCDELAY='25')
            os.execv(argv[0], argv)
        fcntl.ioctl(self.fd, termios.TIOCSWINSZ, struct.pack('HHHH', ROWS, COLS, 0, 0))

    def read(self, secs):
        r, _, _ = select.select([self.fd], [], [], secs)
        if not r:
            return False
        try:
            data = os.read(self.fd, 65536)
        except OSError:
            data = b''
        if not data:
            self.open = False
            return False
        self.stream.feed(data.decode('utf-8', 'replace'))
        return True

    def settle(self, quiet=0.3):
        while self.open and self.read(quiet):
            pass

    def lines(self):
        return [line.rstrip() for line in self.screen.display]

    def wait(self, pattern):
        end = time.time() + TIMEOUT
        while time.time() < end:
            if any(re.search(pattern, line) for line in self.lines()):
                self.settle()
                return True
            if not self.open:
                return False
            self.read(0.1)
        return False

    def finish(self):
        end = time.time() + TIMEOUT
        while time.time() < end:
            if self.open:
                self.read(0.1)
            done, status = os.waitpid(self.pid, os.WNOHANG)
            if done:
                return os.waitstatus_to_exitcode(status)
            if not self.open:
                time.sleep(0.1)
        os.kill(self.pid, 9)
        os.waitpid(self.pid, 0)
        print('completable did not exit')
        return 1

    def show(self):
        print('\n'.join(self.lines()))


def main():
    split = sys.argv.index('--')
    session = Session(sys.argv[1:split])
    remembered = None

    for step in sys.argv[split + 1:]:
        kind, _, arg = step.partition(':')
        if kind == 'key':
            os.write(session.fd, KEYS[arg])
            session.settle()
        elif kind == 'text':
            os.write(session.fd, arg.encode())
            session.settle()
        elif kind == 'wait':
            if not session.wait(arg):
                session.show()
                print(f'timed out waiting for: {arg}')
                return 1
        elif kind == 'expect':
            row, _, pattern = arg.partition(':')
            lines = session.lines() if row == '*' else [session.lines()[int(row)]]
            if not any(re.search(pattern, line) for line in lines):
                session.show()
                print(f'row {row} does not match {pattern}')
                return 1
        elif kind == 'remember':
            remembered = session.lines()[int(arg)]
        elif kind == 'same':
            line = session.lines()[int(arg)]
            if line != remembered:
                session.show()
                print(f'row {arg} changed\n  was: {remembered}\n  now: {line}')
                return 1
        elif kind == 'sh':
            subprocess.run(arg, shell=True, check=True)
        elif kind == 'show':
            session.show()
        else:
            print(f'unknown step: {step}')
            return 1

    status = session.finish()
    if status != 0:
        session.show()
        print(f'completable exited with {status}')
    return status


sys.exit(main())
