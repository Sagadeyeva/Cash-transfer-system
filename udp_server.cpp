#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <thread>
#include <sstream>
#include <iostream>
#include <thread>
#include <vector>
#include <fstream>
#include <algorithm>
#include "transmision.h"
#include <mutex>

#define BUFLEN 512
#define MAX_CLIENTS 10

using namespace std;

int mainSocket;
mutex hMutex;
mutex consoleMutex;

char SERVER_ADDR[15]="172.16.201.1";
int SERVER_PORT=64048;
bool finish=false;
bool working=true;
bool serverStart=false;


struct Arggs
{
    int sock;
    std::thread *thread;
    sockaddr_in info;
    int id;
    bool closure;
};

struct wallet
{
    string walletID;
    int value;
    string owner;
};

struct transfer_data
{
    string wallet_from;
    string login_from;
    string wallet_to;
    string login_to;
    int amount;
};

struct sockaddr_in serverAddress;
static vector <wallet> wallets;
vector <Arggs *> list_of_clients;


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

vector<string> split(string &s, char delim)
{
    stringstream ss(s);
    string item;
    vector<string> tokens;
    while(getline(ss,item,delim))
        tokens.push_back(item);
    return tokens;
}

string to_upper(string in)
{
    transform(in.begin(),in.end(), in.begin(), ::toupper);
    return in;
}

int money_addition (string user_input, string wallet_to_rewrite, int sum)
{
    for (int i=0; i<wallets.size(); i++)
    {
        if (wallets[i].walletID==wallet_to_rewrite)
        {

            wallets[i].value+=sum;
            std::cout<<"Found wallet "<<wallets[i].walletID<<" added"<<sum<<std::endl;
            return 1;
        }
    }
    return 0;
}

vector <wallet> get_wallets(string user_input)
{
    stringstream ss;
    ifstream fs;
    string str=ss.str();
    fs.open("/home/lana/communications/indiv/wallets.txt");
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
        cout<<"Can`t open the file"<<endl;
    }
    return wallets;
}

vector <transfer_data> get_receives(string user_input)
{
    vector<transfer_data>tr;
    ifstream ifs;
    stringstream ss;
    ss<<"/home/lana/communications/indiv/receives.txt";
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

            transmission.wallet_to=tempStrings[2];
            transmission.amount=atoi(tempStrings[3].c_str());
            tr.insert(tr.end(),transmission);
        }
    }
    else
    {
        cout<<"Error. Can not open the file"<<endl;
    }
    return tr;
}
vector<transfer_data> get_sends(string inputLogin)
{
    vector<transfer_data>sends;
    ifstream ifs;
    stringstream ss;
    ss<<"/home/lana/communications/indiv/sends.txt";
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
            transmission.login_from=tempStrings[1];
            transmission.wallet_to=tempStrings[2];
            transmission.amount=atoi(tempStrings[3].c_str());
            sends.insert(sends.end(),transmission);
        }
    }
    else
    {
        cout<<"Error. Can not open the file"<<endl;
    }
    return sends;
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
            return 1;
        }
    }
    return 0;
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
    //rewriteRecieves(user_input, updatedReceives);

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
    ofstream fs;
    fs.open("/home/lana/communications/indiv/users.txt", ofstream::app);
    if (fs.is_open())
    {
        fs<<login<<" "<<password<<endl;
        fs.close();
        return 1;
    }
    else
    {
        cout<<"Error, can not open the file!"<<endl;
        return -1;
    }
    return 0;
}

int login(string login, string password)
{
    ifstream ifs;
    ifs.open("/home/lana/communications/indiv/users.txt");
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
        cout<<"Error, can not open the file!"<<endl;
        return -1;
    }
    ifs.close();
    return 0;
}

int login_check (string login)
{
    ifstream ifs;
    ifs.open("/home/lana/communications/indiv/users.txt");
    string tempL, tempP;
    if (ifs.is_open())
    {
        while (!ifs.eof())
        {
            ifs>>tempL>>tempP;
            if (login==tempL)
            {
                return 1;
            }
        }
    }
    else
    {
        cout<<"Error, can not open the file!"<<endl;
        return -1;
    }
    return 0;
}
//TODO--------------------------------------------------------------------------------
int closeClient(int ind)
{
    hMutex.lock();
    int disc_index=-1;
    for(int i=0; i<list_of_clients.size(); i++)
    {
        if (list_of_clients[i]->id==ind)
        {
            disc_index=i;
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
            return -1;
        }
        else
        {
            printf("\n Client with ID %d has been disconnected \n",ind);
//            amount_of_clients--;
            // shutdown(*list_of_clients[disc_index]->sock, 2);
            close(list_of_clients[disc_index]->sock);

            list_of_clients[disc_index]->thread->join();
            list_of_clients.erase(list_of_clients.begin()+disc_index);

        }
        hMutex.unlock();
    }

    return 0;
}


int client_thread (Arggs * c)
{
    char buffer[512];
    //cout<<c->sock<<endl;
    Arggs *arg = (Arggs*) c;
    int s=c->sock;
    printf("%d\n",s);
    string username="";
    transmision tr;
    tr.mainSocket=s;
    memset(buffer,0,sizeof(buffer));
    string ires;
    cout<<"URRRAAA MI STROILI STROILI I NAKONEC DOS TROI LSDOASDKPASflkshdkfba\n";
    int state=0;

    vector <transfer_data> receives;
    vector <transfer_data> sends;
    vector <string> tempStr;

    strcpy(buffer,"You can use following commands:\n1) show_wallets\n2) create_wallet <walletName>\n3) put_money <wallet> <sum>\n4) send_money <your login> <your wallet> <toLogin> <toWallet> <sum>\n");

    //sendto(s, buffer, strlen(buffer), 0, (struct sockaddr *)&s, sizeof(s));
    send(s,buffer,sizeof(buffer),0);
    // tr.Send(arg->info,buffer);
    tr.lastCommandId=0;
    strcpy(buffer,"");
    int len = sizeof(s);

    while(true)
    {
        switch (state)
        {
        case 0:
        {
            //recvfrom(mainSocket,buff,10000,0,(sockaddr *) &cl, (socklen_t *)&len);
            //ires=tr.Receive(arg->info);
            int ires2=recv(s,buffer,sizeof(buffer),0);
            if(ires=="TIMEOUT")
            {
                cout<<ires<<endl;
                cout<<"SOCKET_ERROR"<<endl;
                closeClient(arg->sock);
                return 0;
            }
            else
            {
                std::string rez(buffer);
                if (count(rez.begin(), rez.end(), ' ')==2)
                {

                    tempStr=split(rez, ' ');
                    string com=to_upper(tempStr[0]);



                    cout<<com<<endl;

                    strncpy(buffer,"",512);
                    if (com.compare("LOGIN")==0)
                    {

                        cout<<"command //to login// has been read"<<endl;

                        if (login((string)tempStr[1], (string)tempStr[2])==1)
                        {

                            strcpy(buffer,"S");
                            tr.Send(arg->info,buffer);

                            strcpy(buffer,"Successful authorization! Welcome!");
                            tr.Send(arg->info,buffer);
                            username=tempStr[1];
                            state=1;
                        }
                        else
                        {
                            strcpy(buffer,"F");
                            tr.Send(arg->info,buffer);
                            strcpy(buffer,"");
                            strcat(buffer,"Incorrect login");
                            tr.Send(arg->info,buffer);
                        }
                    }
                    else if (com.compare("REGISTER")==0)
                    {


                        cout<<"command //to register// has been read";


                        if (login_check(tempStr[1])==1)
                        {
                            strcpy(buffer,"F");
                            tr.Send(arg->info,buffer);
                            strcpy(buffer,"");
                            strcat(buffer,"Sorry, login has been already taken");
                            tr.Send(arg->info,buffer);

                        }
                        else
                        {
                            addUser(tempStr[1], tempStr[2]);
                            strcpy(buffer,"S");
                            tr.Send(arg->info,buffer);
                            strcpy(buffer,"");
                            strcat(buffer,"Successful registration! Welcome:");
                            tr.Send(arg->info,buffer);
                            username=tempStr[1];
                            state=1;

                        }
                    }
                }
                else
                {
                    strcpy(buffer,"F");
                    tr.Send(arg->info,buffer);
                    strcpy(buffer,"");
                    strcat(buffer,"Sorry, but the form of your input is incorrect, try again...");
                    tr.Send(arg->info,buffer);
                }

            }
            break;
        }
        case 1:

            strncpy(buffer,"",512);
            char walletStr[512]="";
            ires=tr.Receive(arg->info);

            if(ires=="TIMEOUT")
            {
                shutdown(s,2);
                closeClient(arg->sock);

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
                        string wal="";
                        for(int i=0; i< wallets.size(); i++)
                        {
                            char balance[10]="";

                            snprintf(balance,sizeof(balance),"%d",wallets[i].value);
                            string balanceStr(balance);
                            wal += wallets[i].owner + " <" + wallets[i].walletID+ "> "+balanceStr+"y.e.\n";
                        }

                        char mes[512];
                        strcpy(mes,wal.c_str());
                        // const char *mes = walletStr.c_str();
                        cout << mes << endl;
                        tr.Send(arg->info, mes);
                    }

                    else if (to_upper(tempStr[0])=="SHOW_RECEIVES")
                    {

                        cout<<"command //show_receives// has been read"<<endl;

                        strcpy(walletStr,"");
                        receives=get_receives(username);

                        for(int i=0; i< receives.size() ; i++)
                        {
                            string tempVal;
                            stringstream ss;
                            ss<<receives[i].amount;
                            ss>>tempVal;
                            string msg="";
                            msg+=receives[i].wallet_from+" "+receives[i].login_to+" "+receives[i].wallet_to+" "+tempVal+"\n";

                            strcat(walletStr,msg.c_str());
                        }
                        tr.Send(arg->info, walletStr);
                    }
                }


                if(tempStr.size()==2)
                {
                    if(to_upper(tempStr[0])=="CREATE_WALLET")
                    {

                        cout<<"command //create_wallet// has been read"<<endl;

                        addWallet(username, tempStr[1]);

                        strcpy(buffer,"New wallet has been created");
                        tr.Send(arg->info,buffer);
                        strcpy(buffer,"");
                    }
                }

                if(tempStr.size()==3)
                {
                    if(to_upper(tempStr[0])=="PUT_MONEY")
                    {

                        cout<<"command //put_money// has been read"<<endl;

                        int tempSum=atoi(tempStr[2].c_str());
                        printf("%d\n",tempSum);
                        money_addition(username, tempStr[1], tempSum);

                        strcpy(buffer,"Money has been sent");
                        tr.Send(arg->info, buffer);
                    }
                }

                if(tempStr.size()==6)
                {
                    if(to_upper(tempStr[0])=="SEND_MONEY")
                    {

                        cout<<"command //send_money// has been read"<<endl;

                        int tempSum=atoi(tempStr[5].c_str());
                        int res=sendSum(username, tempStr[2], tempStr[3], tempStr[4], tempSum);
                        std::cout<<tempStr[1]<<" "<<tempStr[2]<<" "<<tempStr[3]<<" "<<tempStr[4]<<" "<<tempSum<<std::endl;
                        std::cout<<res<<std::endl;
                        if (res==1)
                        {
                            strcpy(buffer,"Your muuniy has been sent!");
                            tr.Send(arg->info, buffer);
                        }
                        else
                        {
                            strcpy(buffer,"Oops, sorry, some problems with your transmission!");
                            tr.Send(arg->info, buffer);
                        }
                    }
                }

            }
        }

    }

    return 0;
}

int acceptNewThread(int s)
{


    printf ("accepting\n");

    int amount_of_clients;

    char buffer[512];
    char reciever[512];
    char sent_message[512];
    char buff[512]="";

    while (1)
    {
        int newConnectionS;
        struct sockaddr_in newClientAdress;
        socklen_t size=sizeof(newClientAdress);
        Arggs *new_client=new Arggs();
        /*<------*/
        int clientSocket=recvfrom(s, buffer, sizeof(buffer), 0, (sockaddr *) &newClientAdress, (socklen_t *)&size);
        cout<<buffer;

        if (strcmp(buffer, "REQUEST")==0)
        {
            cout<<"Request caught up!"<<endl;
            newConnectionS=socket(AF_INET, SOCK_DGRAM, 0);
            sockaddr_in server;
            server.sin_family=AF_INET;
            server.sin_addr.s_addr=htonl(INADDR_ANY);
            server.sin_port=htons(0);
            socklen_t len=sizeof(server);



            if (bind(newConnectionS, (struct sockaddr *)&server, len)<0)
            {
                perror("bind-failure");
                delete(new_client);
                continue;
            }
            cout<<"New connection socket created!"<<endl;

            socklen_t sizee=sizeof(server);
            getsockname(newConnectionS, (sockaddr *) &server, &sizee);


            char tempbuf[512]="Response";
//
            //snprintf(tempbuf, sizeof(tempbuf), "%d", htons(server.sin_port));
            //   sendto(mainSocket, tempbuf, sizeof(tempbuf), 0, (struct sockaddr *) &newClientAdress, size);
            //    cout<<"Send response failed!"<<endl;
            string porto="";
            porto=toString(ntohs(server.sin_port));
            strcpy(tempbuf,porto.c_str());

            if (sendto(s, tempbuf, sizeof(tempbuf), 0, (struct sockaddr *) &newClientAdress, size)<0)
            {
                perror("Port failed:");
                cout<<"Send port failed!"<<endl;
                delete(new_client);
                continue;
            }
            cout<<"Port sent!"<<endl;

            if (recvfrom(newConnectionS, tempbuf, sizeof(tempbuf),0, (struct sockaddr *) &newClientAdress, (socklen_t*)&size)<0)
            {
                cout<<"info recv failed!"<<endl;
                delete(new_client);
                continue;
            }
            cout<<"got info from new client:"<<tempbuf<<endl;

            string connStr(tempbuf);
            cout<<connStr<<endl;

            if (connStr.back()=='\n')
                connStr.pop_back();

            if (connStr=="#connect")
            {
                cout<<"new client has been connected"<<endl;
                if(connect(newConnectionS,(struct sockaddr *) &newClientAdress,size)<0)
                {
                    perror("connect failed");
                }

                cout<<"New socket pair channel established!"<<endl;
                amount_of_clients++;
                char *ip=inet_ntoa(newClientAdress.sin_addr);
                new_client->sock=newConnectionS;
                new_client->info=newClientAdress;
                new_client->id=htons(newClientAdress.sin_port);
                new_client->closure=false;

                std::thread *threadd = new std::thread(client_thread,new_client);
                new_client ->thread=threadd;
                hMutex.lock();
                list_of_clients.insert(list_of_clients.end(), new_client);
                hMutex.unlock();
            }
        }
    }
    for (Arggs *n: list_of_clients)
    {
        closeClient(n->sock);
        n->thread->join();
        //pthread_join(*list_of_clients[disc_index]->thread,NULL);
    }
    //return 0;

}





void showClients()
{
    for(Arggs * n : list_of_clients)
    {

        printf("ID: %d %s:%d\n", n->id, inet_ntoa(n->info.sin_addr),htons(n->info.sin_port));

    }
}

int main(int argc, char** argv)
{
    int ind;
    int res = 0;
    sockaddr_in server;

    int iFamily = AF_INET;
    int iType = SOCK_DGRAM;
    int iProtocol = IPPROTO_UDP;
    int s;
    s = socket(iFamily, iType, iProtocol);
    if (s<0)
    {
        printf("failed");
        exit(1);
    }
    else
    {
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(SERVER_ADDR);
        server.sin_port = htons(SERVER_PORT);

        printf("Socket has been created\n");

        if (bind(s, (struct sockaddr*) &server, sizeof(server))<0)
        {
            perror("failed to bind ");
            exit(2);
        }
        else
        {

            printf ("connecting...\n");

        }
        //   mainSocket=s;
        acceptNewThread(s);
    }
    /*int option=0;
    int ind;

    printf("1. Show all clients;\n");
    printf("2. Disconnect specific client;\n");
    printf("3. Show all wallets;\n");


    printf("Your action:  \n");


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
    */

    return 0;
}



