# Description

```
Reverse a cellular automata - 97
Solves: 128 ▼

It's hard to reverse a step in a cellular automata, but solvable if done right.
https://cellularautomata.web.ctfcompetition.com/
Submit the flag for this task
CTF{...}

```
# Instructions

```
Reverse Cellular Automata
Challenge
We have built a cellular automata with 64 bit steps and obeys Wolfram rule 126, it's boundary condition wraps around so that the last bit is a neighbor of the first bit. Below you can find a special step we chose from the automata.

The flag is encrypted with AES256-CBC, the encryption key is the previous step that generates the given step. Your task is to reverse the given step and find the encryption key.

Example decryption with 32 bit steps:

echo "404c368b" > /tmp/plain.key; xxd -r -p /tmp/plain.key > /tmp/enc.key

echo "U2FsdGVkX18+Wl0awCH/gWgLGZC4NiCkrlpesuuX8E70tX8t/TAarSEHTnpY/C1D" | openssl enc -d -aes-256-cbc -pbkdf2 -md sha1 -base64 --pass file:/tmp/enc.key

Examples of 32 bit steps, reverse_rule126 in the example yields only one of the multiple values.
rule126('deadbeef') = 73ffe3b8 | reverse_rule126('73ffe3b8') = deadbeef

rule126('73ffe3b8') = de0036ec | reverse_rule126('de0036ec') = 73ffe3b8

rule126('de0036ec') = f3007fbf | reverse_rule126('f3007fbf') = de0036ec

Flag (base64)
U2FsdGVkX1/andRK+WVfKqJILMVdx/69xjAzW4KUqsjr98GqzFR793lfNHrw1Blc8UZHWOBrRhtLx3SM38R1MpRegLTHgHzf0EAa3oUeWcQ=

Obtained step (in hex)
66de3c1bf87fdfcf
```
