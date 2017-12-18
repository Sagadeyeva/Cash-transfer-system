#include <iostream>
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <string.h>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <locale>
#include <ctype.h>

#define MAX_CLIENTS 10

using namespace std;
HANDLE hMutex;
std::mutex consoleMutex;
long int threadID;
int amount_of_clients;



struct Arggs
{
    SOCKET *sock;
    std::thread *Th;
    sockaddr_in info;
    int id;
    boolean closure;
};

struct wallet
{
    string walletID;
    int value;
    string owner;
};

static vector <wallet> wallets;

struct transfer_data
{
    string wallet_from;
    string login;
    string wallet_to;
    int amount;
};
std::vector <Arggs *>list_of_clients;

void startWSA()
{
    WSADATA wsaDATA;
    int iResult=WSAStartup(MAKEWORD(2,2), &wsaDATA);
    if(iResult!=0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        exit(1);
    }
    else
        puts("WSAStartup success");
}


int readn(SOCKET Sock, char *pBuffer, int Size)
{
    int c=0;
    memset(pBuffer,NULL,Size);
    while (c<Size)
    {
        int nBytes=recv (Sock, pBuffer+c, Size-c, 0);
        if (nBytes<=0)
        {
            return SOCKET_ERROR;
            shutdown(Sock,2);
            closesocket(Sock);
            return 0;
            break;
        }
        c+=nBytes;

    }
    return 0;
}

string to_upper(string input)
{
    transform(input.begin(), input.end(), input.begin(), (int (*)(int))std::toupper);
    return input;
}

vector <string> split (string &s, char symbol)
{
    stringstream ss(s);
    string elem;
    vector<string> letters;
    while (getline(ss, elem, symbol))
        letters.push_back(elem);
    return letters;
}

vector <transfer_data> get_receives(string user_input)
{
    WaitForSingleObject(hMutex,INFINITE);
    vector<transfer_data>tr;
    ifstream ifs;
    stringstream ss;
    ss<<"C:/receives.txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path,str.c_str());
    ifs.open(path);
    string line;
    if(ifs.is_open())
    {
        while(getline(ifs,line))
        {
            vector<string> tempStrings=split(line,' ');
            transfer_data transmission;
            transmission.wallet_from=tempStrings[0];
            transmission.login=tempStrings[1];
            transmission.wallet_to=tempStrings[2];
            transmission.amount=atoi(tempStrings[3].c_str());
            tr.insert(tr.end(),transmission);
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error. Can not open the file"<<endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    return tr;
}
vector<transfer_data> get_sends(string inputLogin)
{
    WaitForSingleObject(hMutex,INFINITE);
    vector<transfer_data>sends;
    ifstream ifs;
    stringstream ss;
    ss<<"C:/sends.txt";
    string str=ss.str();

    char *path=new char[str.length()+1];
    strcpy(path,str.c_str());
    ifs.open(path);
    string line;
    if(ifs.is_open())
    {
        while(getline(ifs,line))
        {
            vector<string> tempStrings=split(line,' ');
            transfer_data transmission;
            transmission.wallet_from=tempStrings[0];
            transmission.login=tempStrings[1];
            transmission.wallet_to=tempStrings[2];
            transmission.amount=atoi(tempStrings[3].c_str());
            sends.insert(sends.end(),transmission);
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error. Can not open the file"<<endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    return sends;
}

vector <wallet> get_wallets(string user_input)
{
    WaitForSingleObject(hMutex, INFINITE);
    vector <wallet> wallets;
    stringstream ss;
    ifstream fs;
    // ss<<user_input;
    string str=ss.str();
    fs.open("C:/wallets.txt");
    string line;
    if (fs.is_open())
    {
        while (getline(fs,line))
        {
            vector <string> tempStrings=split(line, ' ');
            wallet wally;
            wally.walletID=tempStrings[0];
            wally.value=atoi(tempStrings[1].c_str());
            wallets.insert(wallets.end(), wally);
            cout << wally.value << endl;
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Can`t open the file"<<endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    return wallets;
}

void rewriteData (string user_input, vector <wallet> wallets_to_rewrite)
{
    WaitForSingleObject(hMutex,INFINITE);
    ofstream fs;
    stringstream ss;
    ss<<user_input;
    string str=ss.str();
    fs.open("C:/wallets.txt");
    if (fs.is_open())
    {
        for (int i=0; i<wallets_to_rewrite.size(); i++)
        {
            fs<<wallets_to_rewrite[i].walletID<<" "<<wallets_to_rewrite[i].value<<endl;
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Can`t open the file"<<endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    fs.close();
}

int money_addition (string user_input, string wallet_to_rewrite, int sum)
{
    for (int i=0; i<wallets.size(); i++)
    {
        if (wallets[i].walletID==wallet_to_rewrite)
        {

            wallets[i].value+=sum;
            rewriteData(user_input, wallets);
            std::cout<<"Found wallet "<<wallets[i].walletID<<" added"<<sum<<std::endl;
            return 1;
        }
    }
    return 0;
}

int receive_money (string user_input, string wallet_to_rewrite, int sum)
{
    vector <wallet> wallet=get_wallets(user_input);
    for (int i=0; i<wallet.size(); i++)
    {
        if (wallet[i].walletID==wallet_to_rewrite)
        {
            if (wallet[i].value<sum)
            {
                return -1;
            }
            wallet[i].value-=sum;
            rewriteData(user_input, wallet);
            return 1;
        }
    }
    return 0;
}

void newSend (string fromUser, string sendWallet, string toUser, string recieveWallet, int sum)
{
    WaitForSingleObject(hMutex, INFINITE);
    ofstream fs;
    stringstream ss;
    ss<<"C:/sends.txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path, str.c_str());
    fs.open(path, ofstream::app);
    if (fs.is_open())
    {
        fs<<sendWallet<<" "<<toUser<<" "<<recieveWallet<<" "<<sum<<endl;
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error. Can not open the file"<<endl;
        consoleMutex.unlock();
    }
    fs.close();
    ReleaseMutex(hMutex);
}

void newReceive(string toUser, string receiveWallet, string fromUser, string sendWallet, int sum)
{
    WaitForSingleObject(hMutex, INFINITE);
    ofstream fs;
    stringstream ss;
    ss<<"C:/receives.txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path, str.c_str());
    fs.open(path, ofstream::app);

    if (fs.is_open())
    {
        fs<<receiveWallet<<" "<<fromUser<<" "<<sendWallet<<" "<<sum<<endl;
    }
    else
    {
        consoleMutex.unlock();
        cout<<"Can not open the file"<<endl;
    }
    fs.close();
    ReleaseMutex(hMutex);
}

void rewriteSends (string user_input, vector <transfer_data> tr)
{
    WaitForSingleObject(hMutex, INFINITE);
    ofstream fs;
    stringstream ss;
    ss<<"C:/sends.txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path, str.c_str());
    fs.open(path, ofstream::app);
    if (fs.is_open())
    {
        for (int i=0; i<tr.size(); i++)
        {
            fs<<tr[i].wallet_from<<" "<<tr[i].wallet_to<<" "<<tr[i].amount<<endl;
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error. Can not open the file!"<< endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    fs.close();
}

void rewriteRecieves (string user_input, vector <transfer_data> tr)
{

    WaitForSingleObject(hMutex, INFINITE);
    ofstream fs;
    stringstream ss;
    ss<<"C:/receive.txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path, str.c_str());
    fs.open(path, ofstream::app);
    if (fs.is_open())
    {
        for (int i=0; i<tr.size(); i++)
        {
            fs<<tr[i].wallet_from<<" "<<tr[i].wallet_to<<" "<<tr[i].amount<<endl;
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error. Can not open the file!"<< endl;
        consoleMutex.unlock();
    }
    ReleaseMutex(hMutex);
    fs.close();
}

int sendSum (string fromUser, string fromWallet, string toUser, string toWallet, int sum)
{
    for (int i=0; i<wallets.size(); i++)
    {
        for (int j=0; j<wallets.size(); j++)
        {
            if ((wallets[i].walletID==fromWallet) && (wallets[j].walletID==toWallet))
            {
                if(wallets[i].value>=sum)
                {
                    wallets[i].value-=sum;
                    wallets[j].value+=sum;
                    return 1;
                }
                else
                {
                    return 3;
                }
            }
        }

    }
    return 0;

}

void receiveSum (string user_input, string userWallet, string toWallet, int sum)
{
    vector <transfer_data> tr=get_receives(user_input);
    vector <transfer_data> updatedReceives;

    for (int i=0; i<tr.size(); i++)
    {
        if ((tr[i].wallet_from==userWallet) &&
                (tr[i].wallet_to==toWallet) &&
                (tr[i].amount==sum)
           )
        {
            money_addition(user_input, userWallet, sum);
        }
        else
        {
            updatedReceives.insert(updatedReceives.end(), tr[i]);
        }
    }
    rewriteRecieves(user_input, updatedReceives);

    tr=get_sends(user_input);
    vector <transfer_data> updatedSends;
    for (int i=0; i<tr.size(); i++)
    {
        if  ((tr[i].wallet_from==userWallet) &&
                (tr[i].wallet_to==toWallet) &&
                (tr[i].amount==sum)
            )
        {
            receive_money(user_input, toWallet, sum);
        }
        else
        {
            updatedSends.insert(updatedSends.end(), tr[i]);
        }
    }
}

int addWallet (string user_input, string walletID)
{
    wallet w;
    w.owner = user_input;
    w.walletID = walletID;
    w.value = 0;
    wallets.push_back(w);
    return 0;

}

int addUser(string login, string password)
{
    WaitForSingleObject(hMutex, INFINITE);
    ofstream fs;
    fs.open("C:/users.txt", ofstream::app);
    if (fs.is_open())
    {
        fs<<login<<" "<<password<<endl;
        fs.close();
        return 1;
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error, can not open the file!"<<endl;
        consoleMutex.unlock();
        return -1;
    }
    ReleaseMutex(hMutex);
    return 0;
}

int login(string login, string password)
{
    WaitForSingleObject(hMutex, INFINITE);
    ifstream ifs;
    ifs.open("C:/users.txt");
    string tempL, tempP;
    if (ifs.is_open())
    {
        while (!ifs.eof())
        {
            ifs>>tempL>>tempP;
            if ((login==tempL) && (password==tempP))
            {
                return 1;
            }
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error, can not open the file!"<<endl;
        consoleMutex.unlock();
        return -1;
    }
    ifs.close();
    ReleaseMutex(hMutex);
    return 0;
}

int login_check (string login)
{
    WaitForSingleObject(hMutex, INFINITE);
    ifstream ifs;
    ifs.open("C:/users.txt");
    string tempL, tempP;
    if (ifs.is_open())
    {
        while (!ifs.eof())
        {
            ifs>>tempL>>tempP;
            if (login==tempL)
            {
                ReleaseMutex(hMutex);
                return 1;
            }
        }
    }
    else
    {
        consoleMutex.lock();
        cout<<"Error, can not open the file!"<<endl;
        consoleMutex.unlock();
        ReleaseMutex(hMutex);
        return -1;
    }
    return 0;
}

int closeClient(int ind)
{
    WaitForSingleObject(hMutex, INFINITE);
    int disc_index=-1;
    for(int i=0; i<list_of_clients.size(); i++)
    {
        if (list_of_clients[i]->id==ind)
        {
            disc_index=i;
            ReleaseMutex(hMutex);
            break;
        }
    }
    if (disc_index>=0)
    {
        list_of_clients[disc_index]->closure=true;
        int disconnecter=ind;
        if (!disconnecter)
        {
            printf("Socket has not been closed!\n");
            ReleaseMutex(hMutex);
            return -1;
        }
        else
        {
            consoleMutex.lock();
            printf("\n Client with ID %d has been disconnected \n",ind);
            consoleMutex.unlock();
            amount_of_clients--;
            shutdown(*list_of_clients[disc_index]->sock, 2);
            closesocket(*list_of_clients[disc_index]->sock);
            delete(list_of_clients[disc_index]);
            list_of_clients.erase(list_of_clients.begin()+disc_index);
            ReleaseMutex(hMutex);
        }

    }
    ReleaseMutex(hMutex);
    return 0;
}

int UserInfo (string in_login)
{
    WaitForSingleObject(hMutex, INFINITE);
    ofstream ofs;
    stringstream ss;
    ss<<"C:\\tcpServer\\"<<in_login<<".txt";
    string str=ss.str();
    char *path=new char[str.length()+1];
    strcpy(path, str.c_str());
    ofs.open(path, ofstream::app);
    if (ofs.is_open())
    {
        ofs<<"start_online_wallets"<<endl<<"end_online_wallets"<<endl<<"start_requests"<<endl<<
           "end_requests"<<endl<<"start_sends"<<endl<<"end_sends"<<endl;
        ofs.close();
    }
    else
    {
        cout<<"Error. Can not open file"<<endl;
        return -1;
    }
    ReleaseMutex(hMutex);
}

int sender(SOCKET s, char* buffer, int buflen)
{
    int res=send(s,buffer,buflen,0);
    if(res<=0)
        return 0;
    return 1;
}

int client_thread (Arggs * c)
{
    char buffer[512];
    //cout<<c->sock<<endl;
    SOCKET s=*c->sock;
    string username="";

    memset(buffer,0,sizeof(buffer));
    int ires;

    int state=0;

    vector <transfer_data> receives;
    vector <transfer_data> sends;
    std::vector <string> tempStr;

    strcpy(buffer,"You can use following commands:\n1) show_wallets\n2) create_wallet <walletName>\n3) put_money <wallet> <sum>\n4) send_money <your login> <your wallet> <toLogin> <toWallet> <sum>\n");
    send(s,buffer,sizeof(buffer),0);
    strcpy(buffer,"");
    while(true)
    {
        switch (state)
        {
        case 0:

            ires=readn(s,buffer,sizeof(buffer));

            if(ires==SOCKET_ERROR)
            {
                shutdown(s,2);
                closesocket(s);
                cout<<"SOCKET_ERROR"<<endl;
                shutdown(s,2);
                closesocket(s);
                closeClient(c->id);
                return 0;
            }
            else
            {
                std::string rez(buffer);
                if (count(rez.begin(), rez.end(), ' ')==2)
                {

                    tempStr=split(rez, ' ');
                    string com=to_upper(tempStr[0]);


                    consoleMutex.lock();
                    cout<<com<<endl;
                    consoleMutex.unlock();
                    strncpy(buffer,"",512);
                    if (com.compare("LOGIN")==0)
                    {
                        consoleMutex.lock();
                        cout<<"command //to login// has been read"<<endl;
                        consoleMutex.unlock();
                        if (login((string)tempStr[1], (string)tempStr[2])==1)
                        {

                            strcpy(buffer,"S");
                            send(s,buffer,sizeof(buffer),0);
                            strcpy(buffer,"Successful authorization! Welcome!");
                            send(s,buffer,sizeof(buffer),0);
                            username=tempStr[1];
                            state=1;
                        }
                        else
                        {
                            strcpy(buffer,"F");
                            send(s,buffer,sizeof(buffer),0);
                            strcpy(buffer,"");
                            strcat(buffer,"Incorrect login");
                            send(s,buffer,sizeof(buffer),0);
                        }
                    }
                    else if (com.compare("REGISTER")==0)
                    {

                        consoleMutex.lock();
                        cout<<"command //to register// has been read";
                        consoleMutex.unlock();

                        if (login_check(tempStr[1])==1)
                        {
                            strcpy(buffer,"F");
                            send(s,buffer,sizeof(buffer),0);
                            strcpy(buffer,"");
                            strcat(buffer,"Sorry, login has been already taken");
                            send(s,buffer,sizeof(buffer),0);

                        }
                        else
                        {
                            addUser(tempStr[1], tempStr[2]);
                            strcpy(buffer,"S");
                            send(s,buffer,sizeof(buffer),0);
                            strcpy(buffer,"");
                            strcat(buffer,"Successful registration! Welcome:");
                            send(s,buffer,sizeof(buffer),0);
                            username=tempStr[1];
                            state=1;

                        }
                    }
                }
                else
                {
                    strcpy(buffer,"F");
                    send(s,buffer,sizeof(buffer),0);
                    strcpy(buffer,"");
                    strcat(buffer,"Sorry, but the form of your input is incorrect, try again...");
                    send(s,buffer,sizeof(buffer),0);
                }
                break;
            }

        case 1:
            strncpy(buffer,"",512);
            char walletsStr[512]="";
            ires=readn(s,buffer,sizeof(buffer));

            if(ires==SOCKET_ERROR)
            {
                shutdown(s,2);
                closesocket(s);

                closeClient(c->id);
                return 0;
                break;
            }
            else
            {

                std::string rez(buffer);
                tempStr=split(rez, ' ');
                if(tempStr.size()==1)
                {
                    if(to_upper(tempStr[0])=="SHOW_WALLETS")
                    {
                        string walletsStr;
                        for(int i=0; i< wallets.size(); i++)
                        {
                            char balance[10]="";

                            snprintf(balance,sizeof(balance),"%d",wallets[i].value);
                            string balanceStr(balance);
                            walletsStr += (wallets[i].owner + " <" + wallets[i].walletID+ "> "+balanceStr+"y.e.\n");
                        }

                        const char *mes = walletsStr.c_str();
                        cout << mes << endl;
                        send(s, mes, 512,0);
                    }

                    else if (to_upper(tempStr[0])=="SHOW_RECEIVES")
                    {
                        consoleMutex.lock();
                        cout<<"command //show_receives// has been read"<<endl;
                        consoleMutex.unlock();
                        strcpy(walletsStr,"");
                        receives=get_receives(username);
                        for(int i=0; i< receives.size() ; i++)
                        {
                            string tempVal;
                            stringstream ss;
                            ss<<receives[i].amount;
                            ss>>tempVal;
                            char* msg=new char[(receives[i].wallet_from+" "+
                                                receives[i].login+" "+receives[i].wallet_to+" "+tempVal+"\n").length()+1];
                            strcpy(msg,(receives[i].wallet_from+" "+
                                        receives[i].login+" "+receives[i].wallet_to+" "+tempVal+"\n").c_str());
                            strcat(walletsStr,msg);
                        }
                        send(s, walletsStr, sizeof(walletsStr),0);
                    }
                }


                if(tempStr.size()==2)
                {
                    if(to_upper(tempStr[0])=="CREATE_WALLET")
                    {
                        consoleMutex.lock();
                        cout<<"command //create_wallet// has been read"<<endl;
                        consoleMutex.unlock();
                        addWallet(username, tempStr[1]);

                        strcpy(buffer,"New wallet has been created");
                        send(s,buffer,sizeof(buffer),0);
                        strcpy(buffer,"");
                    }
                }

                if(tempStr.size()==3)
                {
                    if(to_upper(tempStr[0])=="PUT_MONEY")
                    {
                        consoleMutex.lock();
                        cout<<"command //put_money// has been read"<<endl;
                        consoleMutex.unlock();
                        int tempSum=atoi(tempStr[2].c_str());
                        printf("%d\n",tempSum);
                        money_addition(username, tempStr[1], tempSum);

                        strcpy(buffer,"Money has been sent");
                        send(s,buffer,sizeof(buffer),0);
                    }
                }

                if(tempStr.size()==6)
                {
                    if(to_upper(tempStr[0])=="SEND_MONEY")
                    {
                        consoleMutex.lock();
                        cout<<"command //send_money// has been read"<<endl;
                        consoleMutex.unlock();
                        int tempSum=atoi(tempStr[5].c_str());
                        int res=sendSum(username, tempStr[2], tempStr[3], tempStr[4], tempSum);
                        std::cout<<tempStr[1]<<" "<<tempStr[2]<<" "<<tempStr[3]<<" "<<tempStr[4]<<" "<<tempSum<<std::endl;
                        std::cout<<res<<std::endl;
                        if (res==1)
                        {
                            strcpy(buffer,"Your muuniy has been sent!");
                            send(s,buffer,sizeof(buffer),0);
                        }
                        else
                        {
                            strcpy(buffer,"Oops, sorry, some problems with your transmission!");
                            send(s,buffer,sizeof(buffer),0);
                        }
                    }
                }

            }
        }

    }

    return 0;
}

int acceptNewThread(SOCKET &s)
{

    consoleMutex.lock();
    printf ("accepting\n");
    consoleMutex.unlock();
    int amount_of_clients;

    char buffer[512];
    char reciever[512];
    char sent_message[512];

    while (1)
    {

        SOCKET newConnection;
        int iFamily = AF_INET;
        int iType = SOCK_STREAM;
        int iProtocol = IPPROTO_TCP;
        sockaddr_in clientAddress= {};
        int clientAddressLength=sizeof(clientAddress);
        newConnection = accept(s, (SOCKADDR *) & clientAddress, & clientAddressLength);
        amount_of_clients++;

        if (newConnection==INVALID_SOCKET)
        {
            break;
        }
        Arggs *new_client=new Arggs();
        new_client ->sock=&newConnection;
        new_client->info=clientAddress;
        new_client->id=htons(clientAddress.sin_port);
        std::thread *threadd = new std::thread(client_thread,std::ref(new_client));
        new_client ->Th=threadd;
        WaitForSingleObject(hMutex, INFINITE);
        list_of_clients.push_back(new_client);
        ReleaseMutex(hMutex);
        consoleMutex.lock();
        printf("Client connected %s:%d \n",inet_ntoa(clientAddress.sin_addr),htons(clientAddress.sin_port));
        consoleMutex.unlock();
    }
    for(Arggs * n : list_of_clients)
    {
        shutdown(*n->sock,2);
        closesocket(*n->sock);
        n->Th->join();
        delete(n->Th); //ожидание завершени€ всех потоков с учетом отдельного отключени€ в цикле перебора каждого
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
    if (s==INVALID_SOCKET)
    {
        printf("failed");
        return -1;
    }
    else
    {
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr("192.168.136.1");
        server.sin_port = htons(27015);
        consoleMutex.lock();
        printf("Socket created\n");
        consoleMutex.unlock();
        if ((res = bind(s, (SOCKADDR *) &server, sizeof(server)))<0)
        {
            printf ("failed to bind \n");
            return -1;
        }
        else
        {
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
        while (s!=INVALID_SOCKET)
        {
            int option=0;
            int ind;
            consoleMutex.lock();
            printf("1. Show all clients;\n");
            printf("2. Disconnect specific client;\n");
            printf("3. Show all wallets;\n");

            consoleMutex.unlock();
            consoleMutex.lock();
            printf("Your action:  \n");
            consoleMutex.unlock();

            std::cin>>option;

            switch (option)
            {
            case 1:
            {
                showClients();
                break;
            }
            case 2:
            {
                showClients();
                printf("Enter the ID of client to disconnect>>>>>>>> \n");
                std::cin>>ind;
                closeClient(ind);
                break;
            }

            case 3:
            {
                string walletsStr;
                for(int i=0; i< wallets.size(); i++)
                {
                    char balance[10]="";

                    snprintf(balance,sizeof(balance),"%d",wallets[i].value);
                    string balanceStr(balance);
                    walletsStr += (wallets[i].owner + " <" + wallets[i].walletID+ "> "+balanceStr+"y.e.\n");
                }

                const char *mes = walletsStr.c_str();
                cout << mes << endl;

                break;
            }
            }
        }

    }
    return 0;
}
