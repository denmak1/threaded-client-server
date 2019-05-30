CMSC621 - Project 1
Dennis Makmak

readme.txt


Compiling:
  make compile
    Will output 2 executables, "client" and "server".

  make clean
    Deletes executables.


Executing:
  server <port number> <records file>
    Starts a server with the provided records file and listens on the port.

  client <host> <port> <time step> <transactions file>
    Starts a client connected to the host on a port, using the transactions
    file provided. Host should be 127.0.0.1.

    Time step is seconds to sleep between each transaction:
      sleep time = (curr_time_stamp - last_time_stamp) * time_step


Generating files:
  ./gen_records.sh <num of records>
    Generates a number of records with random initial balances. The number of
    records will be the number of users in the system. Redirect the output to
    a file and use with the server.

  ./gen_transactions.sh <num of transactions> <num of users>
    Generates a number of random transactions of withdrawals and deposits up
    to a maximum for $5000 per withdrawal/deposit. Number of users should be
    less than or equal to the number of records generated with gen_records.sh.
    Time steps are random intervals of 1-5.


Scalability testing:
First, we generate a records file for the server. We use 5 accounts to stress
test the locking the happens since we have many transactions going through 5
accounts:
  ./gen_records.sh 5 > ./test2/Records.txt

And the the transaction file for our clients. We use the same transactions
file for all clients to avoid the randomness in accessing different accounts.
  ./gen_transactions.sh 10000 5 > ./test2/Transactions.txt
10000 transactions should be enough

Next we spawn our server and direct output:
  ./server 9999 ./test2/Records.txt > ./test2/server.out

For interest, we can monitor the server output:
  tail -f ./test2/server.out

Varying clients:
Next spawn our (5) clients and have them process transactions, and time their
runtime with this bash one-liner:
  for i in {1..5}; do eval "time (./client 127.0.0.1 9999 .00001 ./test2/Transactions.txt > ./test2/client'$i'.out) &"; done;

We take note only of "real" time:
0m1.169s, 0m1.285s, 0m1.305s, 0m1.344s, 0m1.563s, 0m1.584s

We don't care about which client finishes first for timing, we care about the
time it takes for all clients to finish, so we take note of our slowest time
only: 0m1.584s

So for 5 clients, our runtime will be 0m1.584s.

We run the test with 5 - 200 clients:
clients   real time
05        0m1.584s
10        0m2.991s
20        0m2.366s
30        0m2.808s
40        0m4.125s
50        0m4.060s
60        0m5.217s
70        0m5.819s
80        0m6.370s
90        0m7.274s
100       0m7.330s
110       0m8.227s
120       0m8.782s
130       0m10.651s
140       0m10.883s
150       0m11.617s
160       0m12.395s
170       0m12.284s
180       0m13.127s 
190       0m13.937s 
200       0m14.639s

Varying request rate:
We make some code modifications to keep track of average time per transaction
in the client (this means, send transaction -> let server process it -> get
response feedback from server).

Using the command similar to above, we keep the clients constant (say 20),
and test with various time step intervals. We will use 1000 records/users
here with 1000 transactions per client.

Again, we only look for slowest process, so we can use this bash command to
watch for runtimes and sort them, leaving the slowest on the bottom and
fastest at the top:
  grep "time per transaction" ./test3/client*.out | sort -t' '-n -k4

interval  avg transaction time (s)
0.0001    0.000022
0.0010    0.000023
0.0020    0.000025
0.0030    0.000025
0.0040    0.000027
0.0050    0.000023
0.0060    0.000026
0.0070    0.000025
0.0080    0.000025
0.0090    0.000021
0.0100    0.000021

Trying the same results with 5 records/users and 100 transactions per client.
Out of interest, I recorded the fastest time as well:
inteval   avg transaction time (s)
          slowest    fastest
0.0001    0.000032   0.000017
0.0010    0.000024   0.000017
0.0020    0.000025   0.000016
0.0030    0.000029   0.000017
0.0040    0.000025   0.000017
0.0050    0.000032   0.000015 
0.0060    0.000028   0.000017
0.0070    0.000028   0.000014
0.0080    0.000032   0.000018
0.0090    0.000032   0.000018
0.0100    0.000039   0.000017
