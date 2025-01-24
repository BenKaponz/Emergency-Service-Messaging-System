#pragma once

#include "ConnectionHandler.h"
#include <string>
#include <map>
#include <vector>
#include <mutex>

using namespace std;

class StompProtocol {

private:

    ConnectionHandler *connectionHandler;   // ניהול חיבור לשרת
    string username;                        // שם המשתמש המחובר
    bool isConnected;                         // סטטוס החיבור
    map<string, int> channelToSubscriptionId; // ניהול ערוצים (Channel --> SubscriptionID)
    int subscriptionIDGenerator;                 // מזהה ההרשמה הבא לערוץ
    int receiptIDGenerator;                      // מזהה ה-receipt הבא
    map<int, bool> receipts;                // מעקב אחר receipts שהתקבלו

    mutex protocolMutex; // מנעול לסינכרון בין תהליכים

    // פונקציית עזר ליצירת מחרוזת של Frame
    string createFrameString(const string &command, const string &headers, const string &body = "");
    string addHeader(const string& key, const string& value);

public:

    // Rule of 5.
    StompProtocol(ConnectionHandler *connectionHandler);
    ~StompProtocol();
    StompProtocol(const StompProtocol&) = delete;
    StompProtocol& operator= (const StompProtocol&) = delete;

    // פונקציות עיבוד פקודות משתמש
    string handleLogin(const string &hostPort, const string &username, const string &password); // מחזירה Frame של CONNECT
    string handleLogout();                                                                      // מחזירה Frame של DISCONNECT
    string handleJoin(const string &topic);                                                     // מחזירה Frame של SUBSCRIBE
    string handleExit(const string &topic);                                                     // מחזירה Frame של UNSUBSCRIBE
    void handleReport(const string &filepath);
    // יצירת סיכום לפי אירועים
    void createSummary(const string &channelName, const string &user, const string &file);

    string makeConnectFrame(const string& login, const string& passcode);
    string makeDisconnectFrame(const string& recieptID);
    string makeSubscribeFrame(const string& destination, const string& subscriptionID, const string& receiptID);
    string makeUnsubscribeFrame(const string& subscriptionID, const string& receiptID);
    string makeSendFrame(const string& destination, Event eventToSend);

    // טיפול בתגובות מהשרת
    void processServerMessage(const string &message); // עיבוד הודעת טקסט מהשרת

    // שליחת הודעה לשרת
    bool sendFrame(const string &frame); // שליחת מחרוזת Frame לשרת

    // לולאות קלט וקריאה
    void keyboardLoop(); // לולאת קלט של המשתמש
    void readLoop();     // לולאת קריאה של תשובות מהשרת

    // פונקציות עזר
    vector<string> splitString(const string &str, char delimiter); // פיצול מחרוזת
};