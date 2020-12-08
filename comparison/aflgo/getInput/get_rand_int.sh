#! /bin/bash
# returns random integer between MIN and MAX

_MIN=$1
_MAX=$2
awk -v "seed=$[(RANDOM & 32767) + 32768 * (RANDOM & 32767)]" \
       "BEGIN { srand(seed); printf(\"%d\n\", $_MIN + rand() * ($_MAX - $_MIN) ) }"
