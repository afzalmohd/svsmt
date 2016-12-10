#!/bin/bash

RESULT_DIR="./results"
CHECK_WP="../src/check_wp/check_wp"

if [ $# -ne 0 ] && [ $1 = "clean" ]; then
    rm -r $RESULT_DIR
    exit 0
fi

if [ ! -d $RESULT_DIR ]; then
    mkdir $RESULT_DIR
fi

if [ ! -f $CHECK_WP ]; then
    echo "$CHECK_WP not found. Have you built it?"
    exit 1
fi

for tests in *.c; do
    result_file=$RESULT_DIR"/"${tests%.*}
    echo $result_file
    $CHECK_WP $tests > $result_file
done

