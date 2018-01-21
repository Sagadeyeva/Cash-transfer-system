#pragma comment(lib, "WS2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <iostream>
#include <map>
#include <algorithm>
#include <ctype.h>
#include <fstream>
#include <mutex>
#include <conio.h>
#include <string.h>
#include <cstdlib>
#include <thread>

using namespace std;

#define SERVER "172.16.201.1"
#define BUFLEN 512
#define PORT 64048

bool exitFlag=false;
bool notLogined=true;
bool busy=false;
sockaddr_in server, si_other;
int slen=sizeof(si_other);

char welcomeMsg[]="Welcome to the server!\nUse one of the following commands to login or register\nLOGIN USER PASSWORD\nREGISTER USER PASSWORD\n";
SOCKET clientSocket;

void iAmAlive()
{
    while(!exitFlag)
    {
        Sleep(2000);

        int len = sizeof(si_other);
        string checker="#check";
        send(clientSocket, checker.c_str(), strlen(checker.c_str()),0);
    }
}

int main(int argc, char** argv)
{
    int s;
    char buf[BUFLEN];
    char message[BUFLEN];
    char loginbuf[BUFLEN];
    int packetNumber=0;
    int counter=0;

    WSADATA wsa= {0};
    int ires=WSAStartup(MAKEWORD(2,2), &wsa);
    if (ires!=0)
    {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Initializing winsock...\n");
    if(WSAStartup(MAKEWORD(2,2),&wsa)!=0)
    {
        printf("Failed. Error code :%d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("Initialized\n");

    if((clientSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==SOCKET_ERROR)
    {
        printf("Socket() failed with error code: %d",WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    memset((char *)&si_other,0,sizeof(si_other));
    si_other.sin_family=AF_INET;
    si_other.sin_port=htons(PORT);
    si_other.sin_addr.s_addr=inet_addr(SERVER);
    sockaddr_in client;
    client.sin_family=AF_INET;
    client.sin_port=htons(0);
    client.sin_addr.s_addr=htonl(INADDR_ANY);
    ires=bind(clientSocket,(sockaddr*)&client,sizeof(client));
    if (ires==SOCKET_ERROR)
    {
        printf("Bind failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    char sendbuf[512]="REQUEST";
    sockaddr_in socketInfo;
    int infolen;
    getsockname(clientSocket,(SOCKADDR *)&socketInfo,&infolen);
    printf("started %s:%d \n",inet_ntoa(socketInfo.sin_addr),ntohs(socketInfo.sin_port));
    if(sendto(clientSocket, sendbuf, sizeof(sendbuf), 0,(struct sockaddr *)&si_other, sizeof(si_other))<0)  //, (struct sockaddr *)&si_other, sizeof(si_other));
    {
        printf("Failed to send to the server REQUEST: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    else
    {
        cout<<"request sent"<<endl;
    }
     int si_len=sizeof(si_other);
    ires=recvfrom(clientSocket, buf, sizeof(buf), 0,(struct sockaddr *)&si_other,&si_len);
    cout<<"recieved port and ip"<<endl;
    if (ires<0)
    {
        printf("Failed to receive port: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    int newport=atoi(buf);
    closesocket(clientSocket);

    clientSocket=socket(AF_INET, SOCK_DGRAM, 0);
    ires=bind(clientSocket, (SOCKADDR *) &client, sizeof(client));
    if (ires==SOCKET_ERROR)
    {
        printf("Failed to bind: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_port=htons(newport);
    address.sin_addr.s_addr=inet_addr(SERVER);
    int addressSize=sizeof(address);
    ires=connect(clientSocket, (struct sockaddr *)&address, addressSize);
    if (ires<0)
    {

        printf("Connect failed with error code: %d",WSAGetLastError());
        WSACleanup();
        return 1;
    }
    string rez="#connect";
    send(clientSocket,rez.c_str(),strlen(rez.c_str()), 0);
    int res;
    char menu[512];
    res=recv(clientSocket,buf, sizeof(buf),0);
    strcpy(menu, buf);
    bool authorized=false;

    do
    {

        std::cout<<"1. Authorize;\n";
        std::cout<<"2. Quit;\n";
        std::cout<<"Your action: ";
        std::string option_str;
        std::getline(cin, option_str);

        int option = atoi(option_str.c_str());

        switch (option)
        {
        case 1:
        {
            std::cout<<"Use the following form to authorize: LOGIN username password \n to register: REGISTER username password\n";
            std::string sendbuf;
            std::getline(cin, sendbuf);
            std::strncpy(loginbuf, sendbuf.c_str(),512);
            res=send(clientSocket,loginbuf,strlen(loginbuf), 0);
            cout<<"{ \n"<<loginbuf<<"\n}"<<endl;
            cout<<"authorization info sent"<<endl;

            res=recv(clientSocket, buf, sizeof(buf),0);
            cout<<buf;
            if (strcmp(buf, "S")==0)
            {
                res=recv(clientSocket, buf, sizeof(buf),0);
                std::cout<<buf<<std::endl;
                authorized=true;
                break;
            }
            if (strcmp(buf, "F")==0)
            {
                res=recv(clientSocket, buf, sizeof(buf),0);
                std::cout<<buf<<std::endl;
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
    while (!authorized);

    while (true)
    {

        printf("Welcome to the cash-transfer system! %s\n", menu);
        std::string sendbuffer;
        std::getline(std::cin, sendbuffer);
        strcpy(buf,"");
        strncpy(buf, sendbuffer.c_str(),512);
        res=send(clientSocket, buf, sizeof(buf),0);
        int sr=recv(clientSocket, buf,512,0);
        std::cout<<buf<<std::endl;


    }

closesocket(clientSocket);

return 0;
}

