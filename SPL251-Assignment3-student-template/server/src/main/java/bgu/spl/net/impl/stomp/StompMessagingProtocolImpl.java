package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private ConnectionsImpl<String> connections;
    private User currentUser = null;

    public StompMessagingProtocolImpl() {
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = (ConnectionsImpl<String>) connections;
    }

    @Override
    public void process(String message) {
        Frame frame = new Frame(message); 

        switch (frame.getCommand()) {
            case "CONNECT":
                handleConnect(frame);
                break;
            case "SEND";
                handleSend(frame);
                break;
            case "SUBSCRIBE";
                handleSubscribe(frame);
                break;
            case "UNSUBSCRIBE";
                handleUnsubscribe(frame);
                break;
            case "DISCONNECT";
                handleDisconnect(frame);
                break;
            default:
                sendErrorFrame("Invalid command: " + frame.getCommand());
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    /************************************ CONNECT **************************************/
    private void handleConnect(Frame frame) {

        // Extract required headers
        String userName = splitHeaderValue(frame.getHeaders().get(2));
        String password = splitHeaderValue(frame.getHeaders().get(3));

        if (currentUser != null) {
            sendErrorFrame("The client is already logged in, log out before trying again.");
            return;
        }
        User user = connections.getUser(userName);
        // User never got created
        if (user != null) {
            if (!user.getPassword().equals(password)) {
                sendErrorFrame("Wrong password");
                return;
            }
            if (user.isConnected()) {
                sendErrorFrame("User already logged in");
                return;
            }
        }
        if (user == null) {
            user = new User(userName, password);
            connections.addUser(user);
        }
        user.setConnected(true);
        user.setConnectionId(connectionId);
        
        connections.send(connectionId, "CONNECTED\nversion:1.2\n\n\u0000");//ẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞẞ
    }

    /************************************ DISCONNECT **************************************/
    private void handleDisconnect (Frame frame) {

        // Extract required header and send rececipt.
        String receiptID = splitHeaderValue(frame.getHeaders().get(0));
        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n" + "\\u0000");

        connections.disconnect(connectionId);
    }

    /************************************ SUBSCRIBE ***************************************/
    private void handleSubscribe (Frame frame) {
        String destination = splitHeaderValue(frame.getHeaders().get(0));
        String subscriptionID = splitHeaderValue(frame.getHeaders().get(1));
        String receiptID = splitHeaderValue(frame.getHeaders().get(2));

        

    }


    public void sendErrorFrame(String msg) {
        connections.send(connectionId, msg);
        connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    public String splitHeaderValue(String header) {
        String[] headerParts = header.split(":");
        return headerParts[1];
    }
}