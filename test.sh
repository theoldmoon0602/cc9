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
        echo "$input: $expected expected, but got $actual"
        exit 1
    fi
}

try 0 '0;'
try 42 '42;'
try 4 '1+10-7;'
try 7 '10 + 2 -5;'
try 3 '1 + 2 * 3 - 4;'
try 14 '1 * 2 + 3 * 4;'
try 2 '4/ 2;'
try 1 '4/ 3;'
try 0 '4/ 5;'
try 5 '(1 + 2) * 3 - 4;'
try 20 '1 * (2 + 3) * 4;'
try 1 '1 * -2 + 3;'
try 1 '4*4-3*5 < 2;'
try 0 '4*4-3*5 < 1;'
try 1 '4*4-3*5 <= 2;'
try 1 '4*4-3*5 <= 1;'
try 0 '4*4-3*5 > 2;'
try 0 '4*4-3*5 > 1;'
try 0 '4*4-3*5 >= 2;'
try 1 '4*4-3*5 >= 1;'
try 1 '1 == 1;'
try 0 '2 == 1;'
try 0 '1 != 1;'
try 1 '2 != 1;'

try 10 'a = 10; a;'
try 13 'a = 10; b = 1 + 2; a+b;'
try 11 'a = 10; b = a*2+1; b-a;'

echo OK
