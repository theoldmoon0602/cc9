#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./cc9 "$input" > tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
        echo "$input => $actual"
    else
        echo "$expected expected, but got $actual"
        exit 1
    fi
}

try 0 0
try 42 42
try 4 '1+10-7'
try 7 '10 + 2 -5'

echo OK
