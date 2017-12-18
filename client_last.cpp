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
#define SOCKET int
using namespace std;

int readn(int Sock, char *pBuffer, int Size)
{
//    int c=0;
//    memset(pBuffer,NULL,Size);
//    while (c<Size)
//    {
//        int nBytes=recv (Sock, pBuffer+c, Size-c, 0);
//        if (nBytes<0)
//        {
//            return -1;
//        }
//
//        c+=nBytes;
//
//    }
//    return 0;
    int sr = 0;
    while (Size)
    {

        sr = recv(Sock, pBuffer, Size, 0);
        if (sr < 0)
            return -1;

        pBuffer += sr;
        Size = Size - sr;

    }
    return sr;
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
    int res = 0;
    int s;
    int iFamily = AF_INET;
    int iType = SOCK_STREAM;
    int iProtocol = IPPROTO_TCP;
    char buffer[512];

    sockaddr_in server;

    s = socket(iFamily, iType, iProtocol);
    if (s<0)
    {
        printf("failed");
        return -1;
    }
    else
    {
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("192.168.136.1");
        server.sin_port = htons(27015);

        res = connect(s, (struct sockaddr *) &server, sizeof(server));
        if (res!=0)
            return 0;
        printf("successful connection\n");
        int option=0;
        char menu[512];
        bool authorized=false;
        res=readn(s,buffer,sizeof(buffer));
        strcpy(menu,buffer);


        do
        {
            std::cout<<"1. Authorize;\n";
            std::cout<<"2. Quit;\n";

            std::cout<<"Your action: ";
            std::string option_str;
            std::getline(std::cin, option_str);
            std::string::size_type sz;
            int option = std::stoi (option_str,&sz);

            switch (option)
            {
            case 1:
            {
                std::cout<<("Use the following form to authorize: LOGIN username password \n to register: REGISTER username password\n ");

                /*******old version**********
                  //scanf("%s", buffer);
                //	send(s, buffer, sizeof(buffer), 0);
                    // if (send (s, buffer, sizeof(buffer),0)>0) printf("Message sent!\n");
                //getting a "welcome-message from server"//
                //char buf[512];
                //readn(s,buf,512);

                //now I need to send authorization info to the server
                //using getline because scanf will read till separator, I need a whole line//
                */

                std:: string sendbuf;
                std::getline(std::cin, sendbuf);
                //  std::cout<<sendbuf<<std::endl;
                strncpy(buffer,sendbuf.c_str(),512);
                res=send (s, buffer, sizeof(buffer),0);

                res=readn(s,buffer,sizeof(buffer));
                if (strcmp(buffer,"S")==0)
                {
                    res=readn(s,buffer,sizeof(buffer));
                    std::cout<<buffer<<std::endl;
                    authorized=true;
                    break;
                }

                if (strcmp(buffer,"F")==0)
                {
                    res=readn(s,buffer,sizeof(buffer));
                    std::cout<<buffer<<std::endl;
                    break;
                }


            }
            case 2:

            {
                printf("\n");
                break;

            }

            }

        }
        while(!authorized);


        while(true)
        {
            printf("- true menu %s\n",menu);
            std:: string sendbuf;
            std::getline(std::cin, sendbuf);
            //std::cout<<sendbuf<<std::endl;
            strncpy(buffer,sendbuf.c_str(),512);

            res=send (s, buffer, sizeof(buffer),0);

            int sr = readn(s,buffer,512);

            std::cout<<buffer<<std::endl;

        }
        /* */

    }
    close(s);
    return 0;
}





