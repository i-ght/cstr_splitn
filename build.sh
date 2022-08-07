#!/bin/bash
if [ $# -eq 0 ]
  then
    COMPILER="gcc"
  else
    COMPILER=$1
fi

mkdir -p bin
mkdir -p bin/.tmp

INCLUDES="-I./include/ -I./src/"
LIBRARIES=""
BIN="bin/program"

CFILES=""
for file in ./src/*.c
do
    CFILES="$CFILES $file"
done

echo $COMPILER -g -DDEBUG $INCLUDES $CFILES -o bin/program
$COMPILER -g -DDEBUG $INCLUDES $CFILES -o bin/program
