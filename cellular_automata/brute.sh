#!/bin/bash

python2 automata126.py | tee possible_keys.txt

grep '0x' possible_keys.txt | tr -d 'L' | cut -b3- | while read key
do
    echo "$key" > /dev/shm/plain.key; xxd -r -p /dev/shm/plain.key > /dev/shm/enc.key
    echo "U2FsdGVkX1/andRK+WVfKqJILMVdx/69xjAzW4KUqsjr98GqzFR793lfNHrw1Blc8UZHWOBrRhtLx3SM38R1MpRegLTHgHzf0EAa3oUeWcQ=" | openssl enc -d -aes-256-cbc -pbkdf2 -md sha1 -base64 --pass file:/dev/shm/enc.key 2>/dev/null | grep 'CTF{' > /dev/null
    if [ $? -eq 0 ]
    then
        echo "Key: "
        cat /dev/shm/plain.key

        echo "FLAG:"
        echo "U2FsdGVkX1/andRK+WVfKqJILMVdx/69xjAzW4KUqsjr98GqzFR793lfNHrw1Blc8UZHWOBrRhtLx3SM38R1MpRegLTHgHzf0EAa3oUeWcQ=" | openssl enc -d -aes-256-cbc -pbkdf2 -md sha1 -base64 --pass file:/dev/shm/enc.key 2>/dev/null   
        exit 0
    fi
done