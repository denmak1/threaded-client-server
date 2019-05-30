#!/bin/bash

if [ $# -eq 0 ]
  then
    echo "usage: ./gen_transactions.sh <num of transactions> <num of users>"
    echo "and redirect output to a file"
    exit 1
fi

last_ts=0
for (( i = 1; i <= $1; i++)) do
  r="$(( ( RANDOM % 2 )  + 1 ))"

  if [ "$r" = "1" ]; then
      t='d';
    else
      t='w';
  fi;

  ts=$(($last_ts + $(( ( RANDOM % 5 ) + 1 ))))
  last_ts=$ts

  usr="$(( ( RANDOM % $2 ) + 1 ))"
  # usr="500"
  amt="$(( ( RANDOM % 5000 ) + 1 ))"

  echo "$ts $usr $t $amt";
done;
