//
// CMSC621 - Project 1
// Dennis Makmak
// 
// client.cpp - implements a client in the multi-user concurrent bank account
// manager
//

#include "client.h"


// load transactions from a file
void load_transactions(char* fname) {
  printf("client: loading transactions from %s\n", fname);

  // open file
  FILE *fp;
  fp = fopen(fname, "r");
  
  if (fp == NULL)
    exit(EXIT_FAILURE);

  // read by line
  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  // add transactions to a vector
  int i;
  while ((read = getline(&line, &len, fp)) != -1) {
    // printf("client: line of length %zu :\n", read);
    // printf("%s", line);

    // process transaction by deposting/withdrawing
    char* tokenized;
    tokenized = strtok(line, " ");

    // parse the buffer string
    i = 0;
    transaction_t transaction;
    while (tokenized != NULL) {
      switch (i++) {
        case 0:
          transaction.ts = atoi(tokenized);
          break;
        case 1:
          transaction.id = atoi(tokenized);
          break;
        case 2:
          transaction.cmd = tokenized[0];
          break;
        case 3:
          transaction.amt = atoi(tokenized);
          break;
        default:
          break;
      }
      tokenized = strtok(NULL, " ");
    }

    transaction_list.push_back(transaction);
  }
  fclose(fp);

  printf("client: done\n");
}


// send a transaction to the server and return response
int send_transaction(int socketfd, transaction_t transaction) {
  char buffer[MAXDATASIZE];

  // populate buffer with transaction and send
  bzero(buffer, MAXDATASIZE);
  sprintf(buffer, "%d %d %c %d", transaction.ts,
                                 transaction.id,
                                 transaction.cmd,
                                 transaction.amt);

  printf("client: send %s\n", buffer);
  int n = write(socketfd, buffer, MAXDATASIZE);
  if (n < 0) {
    fprintf(stderr, "client: error writing to socket\n");
    exit(1);
  }

  // wait for response from server
  bzero(buffer, MAXDATASIZE);
  n = read(socketfd, buffer, MAXDATASIZE);

  return atoi(buffer);
}


// process transaction vector
int process_transactions(int socketfd) {
  // iterate over transactions and send each to server
  int res, i;
  int last_ts = 0;
  int curr_ts = 0;
  transaction_list_t::iterator iter;
  for (iter = transaction_list.begin();
       iter != transaction_list.end();
       ++iter) {
    // sleep
    curr_ts = (*iter).ts;
    usleep(1000000 * (curr_ts - last_ts) * timestep);

    clock_t c_s = clock();
    res = send_transaction(socketfd, *iter);
    clock_t c_e = clock();
    runtime += c_e - c_s;

    if (res == -1)
      printf("client: transaction failed, balance unchanged\n");
    else if (res == -2)
      printf("client: transaction failed, account not found\n");
    else {
      printf("client: transaction success, new balance = %d\n", res);
      i++;
    }

    last_ts = curr_ts;
  }

  // done sending transactions, send DONE signal to server
  char buffer[MAXDATASIZE];
  strcpy(buffer, "DONE");
  int n = write(socketfd, buffer, MAXDATASIZE);

  return i;
}


int main(int argc, char **argv) {
  if (argc < 5) {
    fprintf(stderr, "usage: client <host> <port> <timestep> <transactions file>\n");
    exit(1);
  }

  // find server
  struct hostent *server;
  server = gethostbyname(argv[1]);
  if (server == NULL) {
    fprintf(stderr, "client: no such host exists\n");
    exit(1);
  }

  // create socket
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    fprintf(stderr, "client: error creating a socket\n");
    exit(0);
  }

  // populate struct
  struct sockaddr_in server_addr;
  int portno = atoi(argv[2]);
  server_addr.sin_family = AF_INET;
  bcopy((char*)server->h_addr,
        (char*)&server_addr.sin_addr.s_addr,
        server->h_length);
  server_addr.sin_port = htons(portno);

  // connect
  int res =
    connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (res < 0) {
    fprintf(stderr, "client: error connecting to server\n");
    exit(1);
  }

  // read transactions file by line and store into vector
  load_transactions(argv[4]);

  // process transactions
  timestep = atof(argv[3]);
  res = process_transactions(socketfd);

  printf("client: done, %d/%lu successful transactions\n",
         res, transaction_list.size());

  printf("time per transaction: %f\n",
         (runtime/transaction_list.size()) / CLOCKS_PER_SEC);

  close(socketfd);
  exit(0);
}
