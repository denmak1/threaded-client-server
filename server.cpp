//
// CMSC621 - Project 1
// Dennis Makmak
//
// server.cpp - implements a server in the multi-user concurrent bank account
// manager
//

#include "server.h"


// load initial accounts from a file
void load_accounts() {
  // read file by line
  FILE *fp;
  fp = fopen(init_f_name.c_str(), "r");
  
  char* line = NULL;
  size_t len = 0;
  ssize_t read;
  
  if (fp == NULL)
    exit(EXIT_FAILURE);

  int id, balance;
  std::string name;

  char* tokenized;
  while ((read = getline(&line, &len, fp)) != -1) {
    // printf("line of length %zu :\n", read);
    // printf("%s", line);

    char* tokenized;
    tokenized = strtok(line, " ");

    int i = 0;
    while (tokenized != NULL) {
      if (i == 0)
        id = atoi(tokenized);
      else if (i == 1)
        name = std::string(tokenized);
      else if (i == 2)
        balance = atoi(tokenized);

      tokenized = strtok(NULL, " ");
      i++;
    }
    i = 0;
    printf("%d - %s - %d\n", id, name.c_str(), balance);

    // initialize locks for this account
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);

    // create account
    account_t account = {
      .id = id,
      .name = name,
      .balance = balance,
      .lock = lock,
      .cond = cond
    };

    account_map[id] = account;
  } 

  fclose(fp);
}


// print accounts
void print_accounts() {
  account_map_t::iterator iter;
  for (iter = account_map.begin(); iter != account_map.end(); ++iter) {
    printf("%d, %s, %d\n", (iter->second).id,
                           (iter->second).name.c_str(),
                           (iter->second).balance);
  }
}


// client handler thread
void* client_thread(void* _client) {
  client_t client = *((client_t *) _client);

  printf("server: client %d connected\n", client.con_id);

  // get sockfd for this client
  int newsockfd = client.newsockfd;

  // buffer for data
  char buffer[MAXDATASIZE];

  // variables for transaction
  int ts;       // timestamp
  int id;       // account id
  char cmd;     // 'w' or 'd'
  int amt;      // amount

  int n, i, ret_val;
  while (true) {
    bzero(buffer, MAXDATASIZE);

    // wait for a transaction
    n = read(newsockfd, buffer, MAXDATASIZE);

    // check for disconnect
    if (n == 0) {
      printf("server: client %d closed connection\n", client.con_id);
      break;
    }

    // check for recv problem
    if (n < 0) {
      fprintf(stderr, "server: error in receiving data from client %d\n",
              client.con_id);
      break;
    }

    printf("server: from client %d - get %s\n", client.con_id, buffer);

    // if we get DONE signal, close connection with client
    if (strcmp(buffer, "DONE") == 0) {
      printf("server: client %d sent DONE, closing connection\n",
             client.con_id);
      break;
    }

    // process transaction by deposting/withdrawing
    char* tokenized;
    tokenized = strtok(buffer, " ");

    // parse the buffer string
    i = 0;
    while (tokenized != NULL) {
      switch (i++) {
        case 0:
          ts = atoi(tokenized);
          break;
        case 1:
          id = atoi(tokenized);
          break;
        case 2:
          cmd = tokenized[0];
          break;
        case 3:
          amt = atoi(tokenized);
          break;
        default:
          break;
      }
      tokenized = strtok(NULL, " ");
    }
    i = 0;

    printf("server: parsed %d %d %c %d\n", ts, id, cmd, amt);

    // update account balance
    if (account_map.find(id) != account_map.end()) {  // found acc

      // lock account
      pthread_mutex_lock(&(account_map[id].lock));

      if (cmd == 'd') {                               // deposit
        account_map[id].balance += amt;
        ret_val = account_map[id].balance;
      }
      else if (cmd == 'w') {                          // withdraw
        if (account_map[id].balance > amt) {          // ok balance
          account_map[id].balance -= amt;
          ret_val = account_map[id].balance;
        }
        else                                          // low balance
          ret_val = -1;
      }
      else                                            // invalid cmd
        ret_val = -1;

      // unlock account
      pthread_mutex_unlock(&(account_map[id].lock));
    }
    else                                              // acc not found
      ret_val = -2;
     
    // send response
    bzero(buffer, MAXDATASIZE);
    sprintf(buffer, "%d", ret_val);
    n = write(newsockfd, buffer, MAXDATASIZE);
  }

  free(_client);
}


// main routine
int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "usage: server <port number> <records file>\n");
    exit(1);
  }

  // avoid server dying on client exit
  signal(SIGPIPE, SIG_IGN);

  // load initial accounts
  init_f_name = std::string(argv[2]);
  load_accounts();
  print_accounts();

  // create socket
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    fprintf(stderr, "server: error creating a socket\n");
    exit(1);
  }

  // populate structs
  struct sockaddr_in serveradd, clientaddr; 
  bzero((char *)&serveradd, sizeof(serveradd));
  int portno = atoi(argv[1]);

  serveradd.sin_family = AF_INET;
  serveradd.sin_addr.s_addr = INADDR_ANY;
  serveradd.sin_port = htons(portno);

  // bind socket for listening
  int res = bind(socketfd, (struct sockaddr *)&serveradd, sizeof(serveradd));
  if (res < 0) {
    fprintf(stderr, "server: error binding socket\n");
    exit(1);
  }

  listen(socketfd, MAX_CONS);
  printf("server: listening on %d\n", portno);

  // listen for client connections
  int cc = 0;
  int newsockfd;
  while (true) {
    // wait for a new client connection
    socklen_t clientlen = sizeof(clientaddr);
    newsockfd = accept(socketfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (newsockfd < 0)
      fprintf(stderr, "server: error accepting client %d\n", cc);

    else {
      // create thread for the client connection and add it to client list
      pthread_t thread;

      // populate client struct
      client_t client = {
        .con_id = cc,
        .newsockfd = newsockfd,      
        .thread = thread
      };

      // make client pointer for pthread function parameter
      client_t *clientp = (client_t*)malloc(sizeof(*clientp));
      *clientp = client;

      // add client to map
      client_map[cc] = client;

      // spawn the thread
      pthread_create(&client.thread, 0, &client_thread, clientp);
    }

    cc++;
  }

  // join all threads
  client_map_t::iterator iter;
  for (iter = client_map.begin(); iter != client_map.end(); ++iter) {
    pthread_join((iter->second).thread, NULL);
  }

  close(newsockfd);
  close(socketfd);

  return 0;
}
