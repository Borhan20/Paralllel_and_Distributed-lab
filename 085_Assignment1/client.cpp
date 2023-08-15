#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

const char *SERVER_IP = "127.0.0.1"; // Replace with the server IP address
const int SERVER_PORT = 8080;

void ConnectToServer(int &clientSocket)
{
    // Create a socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        cerr << "Error creating socket" << endl;
        exit(1);
    }

    // Initialize the server address 
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr);

    // Connect to the server using file descriptor
    if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        cerr << "Error connecting to the server" << endl;
        exit(1);
    }

    cout << "Connected to the server" << endl;
}

string ReceiveMessage(int clientSocket)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    return buffer;
}

void SendMessage(int clientSocket, const string &message)
{
    send(clientSocket, message.c_str(), message.size(), 0);
}

bool AskYesNoQuestion(const string &question)
{
    string userInput;
    cout << question << " (Y/N): ";
    getline(cin, userInput);

    if (userInput == "Y" || userInput == "y")
    {
        return true;
    }
    else if(userInput == "N" || userInput == "n")
    {
        return false;
    }
    
}

void InteractWithServer(int clientSocket)
{
    int count =0;
    while (true)
    {
        if(count ==0){
            count++;
            string serverResponse = ReceiveMessage(clientSocket);
            cout << serverResponse << endl;

            string userInput;
            getline(cin, userInput);
            SendMessage(clientSocket, userInput);

            if (userInput != "Who's there?")
            {   count--;
                serverResponse = ReceiveMessage(clientSocket);
                cout << serverResponse << endl;
                SendMessage(clientSocket, " "); // Send a dummy text for response separation
                continue;
            }
        }

        else{
            count++;
            string serverResponse = ReceiveMessage(clientSocket);
            cout << serverResponse << endl;

            string userInput;
            getline(cin, userInput);
            SendMessage(clientSocket, userInput);

            serverResponse = ReceiveMessage(clientSocket);
            cout << serverResponse << endl;
            SendMessage(clientSocket, " "); // Send a dummy text for response separation

            serverResponse = ReceiveMessage(clientSocket);
            cout << serverResponse << endl;

            bool check = AskYesNoQuestion("Would you like to listen to another?");

            if (!check)
                
                break;
            else{
                userInput = "Y";
            }

            SendMessage(clientSocket, userInput);

        }
        

        
    }
}

int main()
{
    int clientSocket;
    ConnectToServer(clientSocket);

    InteractWithServer(clientSocket);

    // Close the client socket
    close(clientSocket);

    return 0;
}
