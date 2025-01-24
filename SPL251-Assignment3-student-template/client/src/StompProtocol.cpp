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

