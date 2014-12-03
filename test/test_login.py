#!/usr/bin/python

import pexpect

HANDLE  = 'CBUser1'
MESSAGE = 'Hello, world!'

child = pexpect.spawn('cb/cbinit', timeout=5)
child.expect('Unix CB version 1.0')
child.expect('Allocating \d+ slots')
child.expect('Shared memory size is \d+')

child = pexpect.spawn('cb/CB', timeout=5)
child.expect('Unix CB version 1.0')
child.expect('Use /\\? for help')

i = child.expect(['-- #\d+ Logged on: \w+/.*', 'Handle to use: '])
if i == 1:
  child.sendline(HANDLE)
  child.expect('-- #\d+ Logged on: \w+/%s' % HANDLE)
child.sendline(MESSAGE)
child.expect('#\d+ \w+/.*: %s' % MESSAGE)
child.sendline('/q')
child.expect(pexpect.EOF)
