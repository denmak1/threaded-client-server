#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "usage: ./gen_records.sh <num of records>"
    echo "and redirect output to a file"
    exit 1
fi

for (( i = 1; i <= $1; i++)) do
  echo "$i user_$i $RANDOM";
done;
