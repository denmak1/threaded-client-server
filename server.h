//
// CMSC621 - Project 1
// Dennis Makmak
//
// server.h - header for server.cpp
//

#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iterator>

#define MAXDATASIZE 256
#define MAX_CONS    200

// account typedef
typedef struct account_t {
  int id;                   // acc id
  std::string name;         // acc name
  int balance;              // current balance
  pthread_mutex_t lock;     // lock descriptor
  pthread_cond_t cond;      // cond descriptor 
} account_t;

// client typedef
typedef struct client_t {
  int con_id;               // connection id
  int newsockfd;            // socket
  pthread_t thread;         // thread descriptor
} client_t;

// file containing account ids, names and initial balances
std::string init_f_name;

// map of clients
typedef std::map<int, client_t> client_map_t;
client_map_t client_map;

// map of accounts
typedef std::map<int, account_t> account_map_t;
account_map_t account_map;

#endif
