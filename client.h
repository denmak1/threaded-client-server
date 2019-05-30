//
// CMSC621 - Project 1
// Dennis Makmak
// 
// client.h - header for client.cpp
//

#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

#define MAXDATASIZE 256

// transaction typedef
typedef struct transaction_t {
  int ts;
  int id;
  char cmd;
  int amt;
} transaction_t;

// list of transactions
typedef std::vector<transaction_t> transaction_list_t;
transaction_list_t transaction_list;

// timestep between each transaction
float timestep;

double runtime;

#endif
