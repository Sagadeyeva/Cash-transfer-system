#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sstream>
#include <iostream>
#include <thread>

using namespace std;

int readn(int Sock, char *pBuffer, int Size) {
    int c=Size;
    while (Size)
    {

        int const nBytes=recv (Sock, pBuffer, Size, 0);
                if (nBytes<=0) {
                    return -1;
                }
                if ((0==nBytes)||(-1==nBytes))
                {
                    return -1;
                }
                pBuffer +=nBytes;
                Size -= nBytes;
    }
    return c;
}

void get_word(char* str, int p)
{
    int i=0;
    while(str[p+i]!='\n')
    {
        int C = str[p+i]-'0';
        printf("%d ",C);
        i++;
    }
    printf("\nFINE");
}

int main(void)
{
//   WSADATA wsaData = {0};
   int res = 0;
   int s;
   int iFamily = AF_INET;
   int iType = SOCK_STREAM;
   int iProtocol = IPPROTO_TCP;
    char buffer[512];
  // char recieve_buf[512]="";


   sockaddr_in server;

   //res = WSAStartup(MAKEWORD(2, 2), &wsaData);

   s = socket(iFamily, iType, iProtocol);
   if (s<0)
       {
        printf("failed");
        return -1;
        }
   else {
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.136.1");
    server.sin_port = htons(27015);

    res = connect(s, (struct sockaddr *) &server, sizeof(server));
    if (res!=0)
        return 0;
    printf("successful connection\n");
    while(s>0) {
    int option=0;

    do {
		std::cout<<"1. Authorize;\n";
		std::cout<<"2. Quit;\n";

		std::cout<<"Your action: ";
		//scanf("%i", &option);
		std::string option_str;
        std::getline(std::cin, option_str);
        std::string::size_type sz;
        int option = std::stoi (option_str,&sz);

		switch (option) {
		case 1: {
			std::cout<<("Use the following form to authorize: LOGIN username password \n to register: REGISTER username password\n ");
      /*******old version**********
        //scanf("%s", buffer);
		//	send(s, buffer, sizeof(buffer), 0);
          // if (send (s, buffer, sizeof(buffer),0)>0) printf("Message sent!\n");
       */


       //getting a "welcome-message from server"//
       //char buf[512];
       //readn(s,buf,512);

       //now I need to send authorization info to the server
       //using getline because scanf will read till separator, I need a whole line//
       std:: string sendbuf;
       std::getline(std::cin, sendbuf);
       std::cout<<sendbuf<<std::endl;
        //if (send (s, sendbuf.c_str(), sizeof(sendbuf),0)>0)
        res=send (s, sendbuf.c_str(), sizeof(sendbuf),0);
        std::cout<<res<<std::endl;
           // printf ("info sent for authorization\n");
             //   else {
              //      printf ("Info for authorization not sent\n");
              //      return 0;
           // }

}
		case 2:

		{printf("\n");

		}

	}
}
	while (option!=2);


	// Закрытие сокета и окончание работы с сервером

	//int id=0;
	//getID of disconnected client
	//char anouncement1[512]="Client with ID+'\n'";
	//char anouncement2[512]=" has been disconnected+'\n'";
	//int id=5;

	//char* id_to_string;
	//get_word(id_to_string, id);
	//char buff[512];
	//buff=anouncement1+id_to_string+anouncement2;
	//send(s,anouncement1, sizeof(anouncement1),0);
	//send (s, id_to_string, sizeof(id_to_string),0);
	//send(s,anouncement2,sizeof(anouncement2),0);
	//if ((send(s,anouncement1,sizeof(anouncement1),0)!=SOCKET_ERROR) || (send (s, id_to_string, sizeof(id_to_string),0)!=SOCKET_ERROR) || (send(s,anouncement2,sizeof(anouncement2),0)!=SOCKET_ERROR))

	//{
	    //close(s);
	   // return 0;
	//   	}
    close(s);
    return 0;

}

    }
}


