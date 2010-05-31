/* UDP client in the internet domain */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fstream>

#include "func.h"

void error(char *);

class SmilUdpClient
{
public:
    SmilUdpClient();

    int readPage ();
private:
    std::string         hostName_;
    int                 port_;
   int                  sock, length, n;
   struct               sockaddr_in server, from;
   struct               hostent *hp;
   char                 buffer[256];
 
};


