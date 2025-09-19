#!/bin/bash

# diff the output with the expected result
# the diff output is piped into cat -A to highlight trailing whitespace 

TESTS=tests
EXPECTED=expected
OUTPUT=output

TEST_EXT=c
OUTPUT_EXT=txt

BINARY=build/tokenize

if [ -d "$TESTS" ] && [ -d "$EXPECTED" ]; then
    mkdir -p "$OUTPUT"
else
    echo -e "\e[1;33mDirectory \"$TESTS\" or \"$EXPECTED\" not found\e[0m"
    exit 1
fi

if ! make; then
    echo -e "\e[1;31mError running 'make'\e[0m"
    exit 1
fi

if [ ! -x "$BINARY" ]; then
    echo -e "\e[1;31mBinary \"$BINARY\" not found or not executable\e[0m"
    exit 1
fi

shopt -s nullglob
for i in "$TESTS"/*.$TEST_EXT; do
    echo "Testing file $i:";
    BN=$(basename "$i" .$TEST_EXT)
    "$BINARY" "$i" > "$OUTPUT/o$BN.$OUTPUT_EXT"
    if [ ! -f "$OUTPUT/o$BN.$OUTPUT_EXT" ]; then
        echo -e "\e[1;31mFailed to create file \"$OUTPUT/o$BN.$OUTPUT_EXT\"\e[0m"
        exit 1
    fi
    if [ ! -f "$EXPECTED/e$BN.$OUTPUT_EXT" ]; then
        echo -e "\e[1;31mNo matching reference file \"$EXPECTED/e$BN.$OUTPUT_EXT\"\e[0m"
        exit 1
    fi
    DIFF=$(diff "$OUTPUT/o$BN.$OUTPUT_EXT" "$EXPECTED/e$BN.$OUTPUT_EXT" | cat -A)
    if [ -n "$DIFF" ]; then
        echo -e "\e[0;33mDifferences between $OUTPUT/o$BN.$OUTPUT_EXT and $EXPECTED/e$BN.$OUTPUT_EXT:\e[0m"
        echo "$DIFF"
        echo -e "\e[1;31mSome test(s) failed\e[0m"
        exit 1
    fi
done

if test -n "$(find "$TESTS" -maxdepth 1 -name "*.$TEST_EXT" -print -quit)"; then
    echo -e "\e[1;32mAll tests passed\e[0m"
else
    echo -e "\e[1;33mNo tests found in $TESTS\e[0m"
fi

shopt -u nullglob
