#include "StompProtocol.h"
#include "event.h"
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <fstream>

/********************************* HELPER METHODS ********************************************************/
string StompProtocol::createFrameString(const string &command, const string &headers, const string &body)
{
    string frame = command + "\n" + headers + "\n";
    //Checks if there's body.
    if (!body.empty()){
        frame += body + "\n"; 
    }
    frame += "\0";
    return frame;
}

string addHeader(const string& key, const string& value) {
    return key + ":" + value + "\n";
}

/************************************* FRAME MAKERS *******************************************************/

// CONNNECT FRAME
string StompProtocol::makeConnectFrame(const string& login, const string& passcode) {
    string headers = addHeader("accept-version", "1.2");
    headers += addHeader("host", "stomp.cs.bgu.ac.il");
    headers += addHeader("login", login);
    headers += addHeader("passcode", passcode);

    return createFrameString("CONNECT", headers);
}

// DISCONNECT FRAME
string StompProtocol::makeDisconnectFrame(const string& receiptID) {
    string headers = addHeader("receipt", receiptID);

    return createFrameString("DISCONNECT", headers);
}

// SUBSCRIBE FRAME
string StompProtocol::makeSubscribeFrame(const string& destination, const string& subscriptionID, const string& receiptID) {
    string headers = addHeader("destination", destination);
    headers += addHeader("id", subscriptionID);
    headers += addHeader("receipt", receiptID);

    return createFrameString("SUBSCRIBE", headers);
}

// UNSUBSCRIBE FRAME
string StompProtocol::makeUnsubscribeFrame(const string& subscriptionID, const string& receiptID) {
    string headers = addHeader("id", subscriptionID);
    headers += addHeader("receipt", receiptID);

    return createFrameString("UNSUBSCRIBE", headers);
}

// SEND FRAME
string StompProtocol::makeSendFrame(const string& destination, Event eventToSend) {
    string headers = addHeader("destination", destination);
    
    string body = addHeader("user", eventToSend.getEventOwnerUser());
    body += addHeader("city", eventToSend.get_city());
    body += addHeader("event name", eventToSend.get_name());
    body += addHeader("date time", to_string(eventToSend.get_date_time()));
    body += "general information:\n";

    for (const auto& [key, value] : eventToSend.get_general_information()) {
        body += "    " + key + ": " + value + "\n";
    }

    body = "description:\n" + eventToSend.get_description();

    return createFrameString("SEND", headers, body);
}

/************************************* HANDLER FUNCTIONS *******************************************************/

// HANDLE LOGIN
string StompProtocol::handleLogin(const string &hostPort, const string &username, const string &password) {

    if (isConnected) {
        cout << "You are already logged in, log out before trying again." << endl;
        return "";
    }

    size_t colonPos = hostPort.find(':');
    if (colonPos == string::npos) {
        cout << "Invalid host:port format" << endl;
        return "";
    }

    string host = hostPort.substr(0, colonPos);
    string port = hostPort.substr(colonPos + 1);

    if (host.empty() || username.empty() || password.empty()) {
        cout << "Host, username, or password cannot be empty" << endl;
        return "";
    }

    connectionHandler = new ConnectionHandler(host, static_cast<short>(stoi(port)));
    if(!connectionHandler->connect()){
        cout << "Couldn't connect to server" << endl;
        return "";
    }

    string connectFrame =  makeConnectFrame(username, password);
    currentUser = username;
    cout << connectFrame << endl;
    return connectFrame;
}

// HANDLE LOGOUT 
string StompProtocol::handleLogout() {

    if(!isConnected) {
        cout << "User isn't logged in" << endl;
        return "";
    }

    disconnectReceipt = receiptIDGenerator;
    receiptIDGenerator++;

    string disconnectFrame = makeDisconnectFrame(to_string(disconnectReceipt));
    return disconnectFrame;
}

// HANDLE JOIN 
string StompProtocol::handleJoin(const string &topic) {

    if (!isConnected)  {
        cout << "User is not logged in, log in before trying to join " + topic << endl;
        return "";
    }

    channelToSubscriptionId[topic] = subscriptionIDGenerator;
    string subscribeFrame = makeSubscribeFrame(topic, to_string(subscriptionIDGenerator), to_string(receiptIDGenerator));
    receiptIDGenerator++;
    subscriptionIDGenerator++;

    return subscribeFrame;
}                                                    

// HANDLE EXIT
string StompProtocol::handleExit(const string &topic) {

    if (!isConnected) {
        cout << "User is not logged in, log in before trying to exit " + topic << endl;
        return "";
    }

    int subscriptionIDforTopic = channelToSubscriptionId[topic];
    string unsubscribeFrame = makeUnsubscribeFrame(to_string(subscriptionIDforTopic), to_string(receiptIDGenerator));
    receiptIDGenerator++;
    subscriptionIDGenerator++;

    return unsubscribeFrame;
}             

// HANDLE REPORT
string StompProtocol::handleReport(const string& file) {

    if (!isConnected)  {
        cout << "User is not logged in, log in before trying to report" << endl;
        return "";
    }

    names_and_events jsonData = parseEventsFile(file);
    const string &channelName = jsonData.channel_name;
    vector<Event> &events = jsonData.events;
    
    if (events.empty()) {
        return "";
    }

    for (auto &eventToSend : events) {
        eventToSend.setEventOwnerUser(currentUser);
        saveEventForSummarize(channelName, eventToSend);
        string sendFrame = makeSendFrame(channelName, eventToSend);
        connectionHandler->sendFrameAscii(sendFrame, '\0');
    }
    return "NotEmpty";
}

void StompProtocol::saveEventForSummarize(const string& channelName, const Event& event) {

    if (summarizeMap.find(currentUser) == summarizeMap.end()) {
        summarizeMap[currentUser] = map<string, vector<Event>>();
    }
    if (summarizeMap[currentUser].find(channelName) == summarizeMap[currentUser].end()){
        summarizeMap[currentUser][channelName] = vector<Event>();
    }
    summarizeMap[currentUser][channelName].push_back(event);
}

/************************************************ITAY*****************************************************/


void StompProtocol::initiate() {
    // הפעלת לולאות הקלט והפלט
    thread clientThread = thread([this]() { inputFromClientThreadLoop(); });
    thread serverThread = thread([this]() { inputFromServerThreadLoop(); });

    // המתנה לסיום הלולאות
    clientThread.join();
    serverThread.join();
}

void StompProtocol::inputFromClientThreadLoop() {
    while (!shouldTerminate) {
        string line;
        getline(cin, line);
        vector<string> tokens = splitString(line, ' ');
        string frameToSend;

        if (tokens.empty()) continue;

        try {
            if (tokens[0] == "login") {
                if (tokens.size() != 4) {
                    cout << "Expecting: login <host:port> <username> <password>" << endl;
                    continue;
                }
                frameToSend = handleLogin(tokens[1], tokens[2], tokens[3]);

            } else if (tokens[0] == "exit") {
                if (tokens.size() != 2) {
                    cout << "Expecting: exit <topic>" << endl;
                    continue;
                }
                frameToSend = handleExit(tokens[1]);
            } 
            else if (tokens[0] == "join") {
                if (tokens.size() != 2) {
                    cout << "Expecting: join <topic>" << endl;
                    continue;
                }
                frameToSend = handleJoin(tokens[1]);
            }
            else if (tokens[0] == "report") {
                if (tokens.size() != 2) {
                    cout << "Expecting: report <file>" << endl;
                    continue;
                }
                frameToSend = handleReport(tokens[1]);
            } else if (tokens[0] == "logout") {
                frameToSend = handleLogout();
            } else if (tokens[0] == "summary") {
                if (tokens.size() != 4) {
                    cout << "Expecting: summary <channel_name> <user> <file>" << endl;
                    continue;
                }
                createSummary(tokens[1], tokens[2], tokens[3]);
            } else {
                cout << "Unknown command" << endl;
                continue;
            }

            if (!frameToSend.empty()) {
                if (!connectionHandler->sendFrameAscii(frameToSend, '\0')) {
                    connectionHandler->close();
                    cerr << "Failed to send frame: " << endl;
                }
            }

        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
        }
    }
}


void StompProtocol::inputFromServerThreadLoop() {
    
    while (!shouldTerminate) {
        string responseFromServer;
        if (connectionHandler != nullptr){
            if (connectionHandler->getFrameAscii(responseFromServer, '\0')) {
                vector<string> tokens = splitString(responseFromServer, '\n');
                string title = tokens[0];

                if (title == "CONNECTED") {
                    isConnected = true;
                    cout << "Successfuly logged in:" << endl;
                } else if (title == "RECEIPT") {
                    int receiptId = extractReceiptId(responseFromServer);
                    if (receiptId == -1) {
                        cout << "RECEIPT frame received, but no valid receipt-id found:" << endl;
                    }
                    else if (receiptId == disconnectReceipt) {
                        disconnect();
                    }
                } else if (title == "ERROR") {
                    cout << "Error recieved from server:" << endl;
                } else {
                    cout << "Unexpected frame received: " << title << endl;
                }
                cout << responseFromServer << endl;
            } else {
                cout << "Failed to receive response from server." << endl;
            }
        }
        
    }
}

int StompProtocol::extractReceiptId(const string &frame) {
    // חפש את receipt-id בתוך ה-frame
    size_t receiptPos = frame.find("receipt-id:");
    if (receiptPos != string::npos) {
        // חשב את תחילת הערך של ה-receipt-id
        size_t start = receiptPos + string("receipt-id:").length();
        size_t end = frame.find('\n', start); // חפש את סוף השורה

        // אם הצלחנו למצוא את הסוף, נחזיר את ה-receipt-id כ-integer
        if (end != string::npos) {
            try {
                return stoi(frame.substr(start, end - start));
            } catch (const invalid_argument &e) {
                cerr << "Invalid receipt-id format in frame: " << e.what() << endl;
            } catch (const out_of_range &e) {
                cerr << "Receipt-id out of range in frame: " << e.what() << endl;
            }
        }
    }
    // במקרה שלא מצאנו או היה שגיאה, נחזיר -1
    return -1;
}

void StompProtocol::disconnect() {
    connectionHandler->close();
    connectionHandler = nullptr;
    currentUser = "";
    channelToSubscriptionId.clear();
    isConnected = false;
    subscriptionIDGenerator = 1;
    receiptIDGenerator = 1;
    cout << "Logged out successfully." << endl;
}
