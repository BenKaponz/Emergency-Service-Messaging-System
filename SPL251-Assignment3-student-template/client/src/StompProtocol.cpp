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
    tempUsername = username;
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

       // Check if the channelName exists in the map
    if (channelToSubscriptionId.find(channelName) == channelToSubscriptionId.end()) {
        cout << "User cannot report because he is not subscribed to the channel: " << channelName << endl;
        return "";
    }

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

void StompProtocol::createSummary(const string &channelName, const string &user, const string &file) {
    lock_guard<mutex> lock(summarizeMapMutex);
    if (summarizeMap.find(channelName) == summarizeMap.end()) {
        cout << "Can't summarize, the given channel isn't exist" << endl;
        return;
    }
    if (summarizeMap[channelName].find(user) == summarizeMap[channelName].end()) {
        cout << "Can't summarize, the given user isn't didn't send any message relative to the given channel" << endl;
        return;
    }

    vector<Event> &events = summarizeMap[channelName][user];

    // Sorting first by date time then by name.
    sort(events.begin(), events.end(), [](const Event &a, const Event &b) {
        if (a.get_date_time() != b.get_date_time()) {
            return a.get_date_time() < b.get_date_time();
        }
        return a.get_name() < b.get_name();
    });

    // Calculating stats.
    int activeCount = 0;
    int forcesArrivalCount = 0;
    for (const auto &ev : events) {
        if (ev.get_general_information().at("active") == "true") {
            activeCount++;
        }
        if (ev.get_general_information().at("forces_arrival_at_scene") == "true") {
            forcesArrivalCount++;
        }
    }

    // Creating file if isn't existing.
    ofstream outFile(file, ios::trunc);
    if (!outFile.is_open()) {
        cout << "Error: Could not open or create the file: " << file << endl;
        return;
    }
    
    // Writing the summary to the file
    outFile << "Channel: " << channelName << endl;
    outFile << "Stats:" << endl;
    outFile << "Total: " << events.size() << endl;
    outFile << "active: " << activeCount << endl;
    outFile << "forces arrival at scene: " << forcesArrivalCount << "\n" << endl;
    outFile << "Event Reports:" << endl;

    // Writing all reports.
    int reportNumber = 1;
    for (const auto &ev : events) {
        outFile << "Report_" << reportNumber << ":" << endl;
        outFile << "   city: " << ev.get_city() << endl;
        // המרת זמן מ-epoch לפורמט קריא
        time_t epochTime = static_cast<time_t>(ev.get_date_time());
        outFile << "   date time: " << epochToDate(epochTime) << endl;
        outFile << "   event name: " << ev.get_name() << endl;
        // יצירת תקציר לתיאור
        string descriptionSummary = ev.get_description();
        if (descriptionSummary.length() > 27) {
            descriptionSummary = descriptionSummary.substr(0, 27) + "...";
        }
        outFile << "   summary: " << descriptionSummary << endl;
        reportNumber++;
    }
    outFile.close();
    cout << "Summary created successfully in file: " << file << endl;
}

string StompProtocol:: epochToDate(time_t epochTime) {
    struct tm *timeInfo = localtime(&epochTime);

    // יצירת מחרוזת בפורמט הנדרש
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%d/%m/%y %H:%M", timeInfo);

    return string(buffer);
}

/************************************************ITAY*****************************************************/


void StompProtocol::initiate() {
    // הפעלת לולאות הקלט והפלטs
    thread clientThread = thread([this]() {clientThreadLoop(); });
    thread serverThread = thread([this]() {serverThreadLoop(); });

    // המתנה לסיום הלולאות
    clientThread.join();
    serverThread.join();
}

void StompProtocol::clientThreadLoop() {
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


void StompProtocol::serverThreadLoop() {
    
    while (!shouldTerminate) {
        string responseFromServer;
        if (connectionHandler != nullptr){
            if (connectionHandler->getFrameAscii(responseFromServer, '\0')) {
                vector<string> tokens = splitString(responseFromServer, '\n');
                string command = tokens[0];

                if (command == "CONNECTED") {
                    isConnected = true;
                    currentUser = tempUsername;
                    cout << "Successfuly logged in:" << endl;
                } else if (command == "RECEIPT") {
                    int receiptId = extractReceiptId(responseFromServer);
                    if (receiptId == disconnectReceipt) {
                        disconnect();
                    }
                } else if (command == "ERROR") {
                    cout << "Error recieved from server:" << endl;
                } else if (command == "MESSAGE") {
                    Event event = createEvent(responseFromServer);
                    saveEventForSummarize(event.get_channel_name(),event);
                } else {
                    cout << "Unexpected frame received: " << command << endl;
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
    tempUsername = "";
    currentUser = "";
    isConnected = false;
    subscriptionIDGenerator = 1;
    receiptIDGenerator = 1;
    disconnectReceipt = -2;

    channelToSubscriptionId.clear();
    summarizeMap.clear();
    
    cout << "Logged out successfully." << endl;
}

Event StompProtocol::createEvent(const string &frame) {
    string channel_name, city, name, description;
    int date_time = 0;
    map<string, string> general_information;

    // Split the frame into lines
    vector<string> lines = splitString(frame, '\n');
    bool isDescription = false; // To identify if we're processing the description

    for (const string &line : lines) {
        if (line.empty()) {
            // Skip empty lines
            continue;
        }

        if (line.find("destination:") == 0) {
            // Extract channel name from destination
            channel_name = line.substr(12); // Skip "destination:"
        } else if (line.find("city:") == 0) {
            city = line.substr(5); // Skip "city:"
        } else if (line.find("event name:") == 0) {
            name = line.substr(11); // Skip "event name:"
        } else if (line.find("date time:") == 0) {
            date_time = stoi(line.substr(10)); // Convert "date time:" value to integer
        } else if (line.find("general information:") == 0) {
            // Start parsing general information
            isDescription = false; // Reset description flag
        } else if (line.find("description:") == 0) {
            isDescription = true; // Start parsing the description
            description = line.substr(12); // Initialize description with the first line
        } else if (isDescription) {
            // Append to description
            if (!description.empty()) {
                description += "\n"; // Add newline for subsequent lines
            }
            description += line;
        } else if (line.find(':') != string::npos) {
            // Parse key-value pairs in general information
            size_t delimiterPos = line.find(':');
            string key = line.substr(0, delimiterPos);
            string value = line.substr(delimiterPos + 1);
            general_information[key] = value;
        }
    }

    // Create and return the Event object
    return Event(channel_name, city, name, date_time, description, general_information);
}