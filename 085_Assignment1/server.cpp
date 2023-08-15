#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <vector>
#include <ctime>
#include <algorithm>
#include <thread>

using namespace std;

const int SERVER_PORT = 8080;
const int MAX_CLIENTS = 3;

vector<pair<string, string>> jokes;
//loading jokes from file 
void LoadJokes(vector<pair<string, string>> &jokes)
{
    ifstream file("jokes.txt");
    if (!file)
    {
        cerr << "Error opening jokes.txt file" << endl;
        return;
    }

    string jokesText;
    while (getline(file, jokesText))
    {
        // separate jokes setup and the punch line and save it a data structure
        size_t jokesTypePos = jokesText.find(',');
        if (jokesTypePos != string::npos)
        {
            string jokesType = jokesText.substr(0, jokesTypePos);
            string jokesPunchline = jokesText.substr(jokesTypePos + 1);
            jokes.emplace_back(jokesType, jokesPunchline);
        }
    }

    file.close();
}

// sending and receving buffer
void SendAndReceive(int clientSocket, const string &sendMsg, string &receivedMsg)
{
    char buffer[1024] = {0};
    send(clientSocket, sendMsg.c_str(), sendMsg.size(), 0);
    recv(clientSocket, buffer, sizeof(buffer), 0);
    receivedMsg = buffer;
}

void HandleClient(int clientSocket)
{
    vector<int> jokeIndices(jokes.size());

    for (vector<pair<string, string>>::size_type i = 0; i < jokes.size(); ++i)
    {
        jokeIndices[i] = i;
    }

    //randomized the jokes sequences
    random_shuffle(jokeIndices.begin(), jokeIndices.end());

    //starting chatting with the message
        string knockMsg = "Knock knock!";
        send(clientSocket, knockMsg.c_str(), knockMsg.size(), 0);
        cout << "Server: " << knockMsg << endl;

        char buffer[1024] = {0};
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);
        string clientResponse = buffer;
        cout << "Client: " << clientResponse << endl;
        memset(buffer, 0, sizeof(buffer));

    for (vector<pair<string, string>>::size_type i = 0; i < jokes.size(); ++i)
    {
        //extract jokes type and punch line to show jokes to client
        int randomIndex = jokeIndices[i];
        string jokeSetup = jokes[randomIndex].first;
        string jokePunchline = jokes[randomIndex].second;

        

        //checking spelling
        if (clientResponse != "Who's there?" && i==0)
        {
            string response = "You are supposed to say, “Who’s there?”. Let’s try again.";
            send(clientSocket, response.c_str(), response.size(), 0);
            cout << "Server: " << response << endl;
            i--;
            // separating mismatch string
            recv(clientSocket, buffer, sizeof(buffer), 0);
            clientResponse = buffer;
            memset(buffer, 0, sizeof(buffer));

            continue;
        }

        i++;
        send(clientSocket, jokeSetup.c_str(), jokeSetup.size(), 0);
        cout << "Server: " << jokeSetup << endl;

        recv(clientSocket, buffer, sizeof(buffer), 0);
        clientResponse = buffer;
        cout << "Client: " << clientResponse << endl;
        memset(buffer, 0, sizeof(buffer));

        string serverResponse;
        if (clientResponse == (jokeSetup + " who?"))
        {
            serverResponse = jokePunchline;
            send(clientSocket, serverResponse.c_str(), serverResponse.size(), 0);
            cout << "Server: " << serverResponse << endl;

            recv(clientSocket, buffer, sizeof(buffer), 0);
            clientResponse = buffer;
            memset(buffer, 0, sizeof(buffer));

            if (i < jokes.size() - 1)
            {
                //continuing jokes if client want
                string anotherOne = "Would you like to listen to another? (Y/N)";
                send(clientSocket, anotherOne.c_str(), anotherOne.size(), 0);
                cout << "Server: " << anotherOne << endl;

                recv(clientSocket, buffer, sizeof(buffer), 0);
                clientResponse = buffer;
                cout << "Client: " << clientResponse << endl;
                memset(buffer, 0, sizeof(buffer));

                if (clientResponse == "N")
                {
                    break;
                }
            }
        }

        else if(clientResponse == "Y"){
            if (i < jokes.size() - 1)
            {
                //continuing jokes if client want
                string anotherOne = "Would you like to listen to another? (Y/N)";
                send(clientSocket, anotherOne.c_str(), anotherOne.size(), 0);
                cout << "Server: " << anotherOne << endl;

                recv(clientSocket, buffer, sizeof(buffer), 0);
                clientResponse = buffer;
                cout << "Client: " << clientResponse << endl;
                memset(buffer, 0, sizeof(buffer));

                if (clientResponse == "N")
                {
                    break;
                }
            }
        }
        
        else
        {
            //warn client spelling mistake and try again
            serverResponse = "You are supposed to say '" + jokeSetup + " who?'. Let’s try again.";
            send(clientSocket, serverResponse.c_str(), serverResponse.size(), 0);
            cout << "Server: " << serverResponse << endl;

            recv(clientSocket, buffer, sizeof(buffer), 0);
            clientResponse = buffer;
            memset(buffer, 0, sizeof(buffer));

            --i;
        }
    }
    // end connection with client if jokes complete or client don't want to another
    string noMoreJokes = "No more joke to show.";
    send(clientSocket, noMoreJokes.c_str(), noMoreJokes.size(), 0);
    cout << "Server: " << noMoreJokes << endl;

    close(clientSocket);
}

void AcceptClients(int serverSocket)
{
    socklen_t clientAddrLen;
    struct sockaddr_in clientAddr;

    while (true)
    {
        //accept client
        clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket < 0)
        {
            cerr << "Error to accept connection" << endl;
            continue;
        }

        thread(HandleClient, clientSocket).detach();
        cout << "Accepted connection from " << inet_ntoa(clientAddr.sin_addr) << endl;
    }
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddr;
    vector<thread> threadList;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        cerr << "Error socket" << endl;
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        cerr << "Error setting socket options" << endl;
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

//binding server file descriptor
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        cerr << "Error binding socket" << endl;
        return 1;
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0)
    {
        cerr << "Error listening" << endl;
        return 1;
    }

    cout << "Server listening on port " << SERVER_PORT << endl;

    LoadJokes(jokes);
    AcceptClients(serverSocket);

    close(serverSocket);

    return 0;
}
