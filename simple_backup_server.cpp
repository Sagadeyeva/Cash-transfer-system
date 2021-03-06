#include <iostream>
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#define MAX_CLIENTS 10
using namespace std;
std::mutex hMutex;
std::mutex consoleMutex;
long int threadID;
int amount_of_clients;

struct Arggs {
    SOCKET sock;
    std::thread *Th;
    sockaddr_in info;
    int id;
    boolean closure;
};

std::vector <Arggs *>list_of_clients;

int readn(SOCKET Sock, char *pBuffer, int Size) {
    int c=Size;
    while (Size)
    {
        int const nBytes=recv (Sock, pBuffer, Size, 0);
                if (nBytes==SOCKET_ERROR) {
                    return SOCKET_ERROR;
                }
            cout<<nBytes;
            cout<<pBuffer;
              if ((0==nBytes)||(-1==nBytes))
                {
                    return -1;
                }
                pBuffer +=nBytes;
                Size -= nBytes;
    }

    return c;
}

void closeClient(int  ind) {
    int disc_index=-1;
    hMutex.lock();
    for (int i=0; i<list_of_clients.size(); i++)
    {
        if (list_of_clients[i]->id==ind)
            {disc_index=i; break;}
    }
    if (disc_index>=0) {
        list_of_clients[disc_index]->closure=true;
        int disconnecter=ind;
        if (!disconnecter) {
            printf("Socket has not been closed!\n");
        }
        else {
            printf("Client with ID %d has been disconnected \n",ind);
            amount_of_clients--;
        }
        delete(list_of_clients[disc_index]);
        list_of_clients.erase(list_of_clients.begin()+disc_index);
    }
    hMutex.unlock();
}



int client_thread (Arggs * c) {
    char buffer[512];
    SOCKET s=c->sock;
    memset(buffer,0,sizeof(buffer));
    int ires;
    while(true)
    {
        ires=readn(s,buffer,sizeof(buffer));
        cout<<ires;
        cout<<buffer;
        if(ires==SOCKET_ERROR)
        {
                shutdown(s,2);
                closesocket(s);
                closeClient(c->id);
                break;
        }
        consoleMutex.lock();
        printf("message----------> %s \n", buffer);
        consoleMutex.unlock();
        ires=send(s, buffer, sizeof(buffer), 0);
        consoleMutex.lock();
        printf("message recieved \n");
        consoleMutex.unlock();
        if (ires==SOCKET_ERROR)
            {
                shutdown(s,2);
                closesocket(s);
                break;
            }
    }
    return 0;
}

int acceptNewThread(SOCKET &s) {

    consoleMutex.lock();
    printf ("accepting\n");
    consoleMutex.unlock();
    int amount_of_clients;

    char buffer[512];
    char reciever[512];
    char sent_message[512];

while (1) {

   SOCKET newConnection;
    int iFamily = AF_INET;
   int iType = SOCK_STREAM;
   int iProtocol = IPPROTO_TCP;
   sockaddr_in clientAddress={0};
   int clientAddressLength=sizeof(clientAddress);
   newConnection = accept(s, (SOCKADDR *) & clientAddress, & clientAddressLength);
   amount_of_clients++;

   if (newConnection==INVALID_SOCKET) {
    break;
   }
  Arggs *new_client=new Arggs();
   new_client ->sock=newConnection;
   new_client ->id=newConnection;
   new_client->info=clientAddress;
   std::thread *threadd = new std::thread(client_thread,std::ref(new_client));
   new_client ->Th=threadd;
   hMutex.lock();
   list_of_clients.push_back(new_client);
   hMutex.unlock();

   printf("Client connected %s:%d \n",inet_ntoa(clientAddress.sin_addr),htons(clientAddress.sin_port));

}
for(Arggs * n : list_of_clients)
{
   shutdown(n->sock,2);
   closesocket(n->sock);
   n->Th->join();
   delete(n->Th);
   delete(n);
}
return 0;
}

void showClients()
{
    for(Arggs * n : list_of_clients)
{
     consoleMutex.lock();
     printf("ID: %d %s:%d\n", n->id, inet_ntoa(n->info.sin_addr),htons(n->info.sin_port));
     consoleMutex.unlock();
}
}

int main(void)
{
   WSADATA wsaData = {0};
   int ind;
   int res = 0;
   sockaddr_in server;
   res = WSAStartup(MAKEWORD(2, 2), &wsaData);
   SOCKET s = INVALID_SOCKET;
   int iFamily = AF_INET;
   int iType = SOCK_STREAM;
   int iProtocol = IPPROTO_TCP;
   s = socket(iFamily, iType, iProtocol);
   if (s==INVALID_SOCKET) {
        printf("failed");
        return -1;
   } else {
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("192.168.136.1");
    server.sin_port = htons(27015);
    consoleMutex.lock();
    printf("Socket created\n");
    consoleMutex.unlock();
    if ((res = bind(s, (SOCKADDR *) &server, sizeof(server)))<0) {
        printf ("failed to bind \n");
        return -1;
    } else {
        consoleMutex.lock();
    printf ("connecting...\n");
    consoleMutex.unlock();
    }
    int client_to_delete=-1;

    res = listen(s,10);
    consoleMutex.lock();
    std::thread acceptThr(acceptNewThread, std::ref(s));
    consoleMutex.unlock();
    boolean closure=false;
    int comand;
    while (s!=INVALID_SOCKET) {
        int option=0;
        int ind;
        consoleMutex.lock();
            printf("1. Show all clients;\n");
            printf("2. Disconnect specific client;\n");
            printf("3. Add a new client;\n");
            printf("4. Show the state of a specific wallet;\n");
            printf("5. Make a cash transfer from client;\n");
            printf("6. Close server;\n");
consoleMutex.unlock();
consoleMutex.lock();
		printf("Your action:  \n");
consoleMutex.unlock();
		//scanf("%i \n", &option);
		std::cin>>option;

		switch (option) {
		case 1:{
                showClients();
                break;
                }
        case 2:{
            showClients();
            printf("Enter the ID of client to disconnect>>>>>>>> \n");
            std::cin>>ind;
            closeClient(ind);
            break;
        }


        case 6:
            {

   printf("Server is closing\n");
    shutdown(s,2);
    res = closesocket(s);
    acceptThr.join();
    printf("Server has been closed \n");
    return 0;
            }
    }
   }

}
return 0;
}
