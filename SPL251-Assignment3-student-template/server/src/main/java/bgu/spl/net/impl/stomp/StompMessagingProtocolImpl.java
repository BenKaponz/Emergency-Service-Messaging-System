package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private ConnectionsImpl<String> connections;
    private User currentUser = null;


    @Override
    public void start(int connectionId, Connections<String> connections) {        
        this.connections = (ConnectionsImpl<String>) connections;
        this.connectionId = connectionId;
        System.out.println("protocol has started!");
    }

    @Override
    public void process(String message) {
        // Spliting msg
        String[] msgLines = message.split("\n");
        String command = msgLines[0];

        if (command.equals("CONNECT")) {
            handleConnect(msgLines);
            return;
        }
        if (currentUser == null) {
            sendErrorFrame("Cannot preform the action because user is not connected");
            return;
        }
        switch (command) {
            case "SUBSCRIBE":
                handleSubscribe(msgLines);
                break;
            case "UNSUBSCRIBE":
                handleUnsubscribe(msgLines);
                break;
            case "DISCONNECT":
                handleDisconnect(msgLines);
                break;    
            case "SEND":
                handleSend(msgLines);
                break;    
            default:
                sendErrorFrame("Invalid command: " + command);
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    /************************************
     * CONNECT
     **************************************/
    private void handleConnect(String[] msgLines) {

        // Extract required headers
        String userName = splitHeaderValue(msgLines[3]);
        String password = splitHeaderValue(msgLines[4]);

        System.out.println(userName + " IS TRYING TO CONNECT TO THE SERVER");

        if (currentUser != null) {
            sendErrorFrame("The client is already logged in, log out before trying again.");
            return;
        }
        User user = connections.getUser(userName);
        // User never got created
        if (user != null) {
            System.out.println(userName + " EXISTS, CHECKING IF HE PUT THE CORRECT PASSWORD");
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
            System.out.println(userName + " NOT EXISTS, CREATING A NEW ONE");

            user = new User(userName, password);
            connections.addUser(user);
        }
        currentUser = user;
        user.setConnected(true);
        user.setConnectionId(connectionId);

        connections.send(connectionId, "CONNECTED\nversion:1.2\n\n");

        System.out.println(userName + " CONNECTED SUCCESSFULLY!!!!!!!!");
    }

    /************************************
     * DISCONNECT
     **************************************/
    private void handleDisconnect(String[] msgLines) {

        System.out.println(currentUser.getUserName() + " IS TRYING TO DISCONNECT FROM THE SERVER");

        System.out.println(" FIRST CONNECTION ID IS " + connectionId);
        // Extract required header and send rececipt.
        String receiptID = splitHeaderValue(msgLines[1]);
        currentUser.disconnect();
        currentUser = null;

        System.out.println(" SECOND CONNECTION ID IS " + connectionId);

        //connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");
        connections.send(connectionId, "bye");
        connections.disconnect(connectionId);

        System.out.println( " DISCONNECTED SUCCESSFULLY!!!!!!!!");

        
    }

    /************************************
     * SUBSCRIBE
     ***************************************/
    private void handleSubscribe(String[] msgLines) {
        String destination = splitHeaderValue(msgLines[1]);
        String subscriptionID = splitHeaderValue(msgLines[2]);
        String receiptID = splitHeaderValue(msgLines[3]);

        System.out.println(currentUser.getUserName() + " SUBSCRIBING TO " + destination);

        currentUser.addSub(destination, subscriptionID);
        connections.subscribe(destination, connectionId);
        
        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");

        System.out.println(currentUser.getUserName() + " SUCCESSFULLY SUBSCRIBED TO " + destination +"!!!!!!!!!!!!!!!!!!");

    }

    /************************************
     * UNSUBSCRIBE
     ***************************************/
    private void handleUnsubscribe(String[] msgLines) {
        String subscriptionID = splitHeaderValue(msgLines[1]);
        String receiptID = splitHeaderValue(msgLines[2]);

        String channel = currentUser.removeSub(subscriptionID);

        System.out.println(currentUser.getUserName() + " UNSUBSCRIBING FROM " + channel);

        connections.unsubscribe(channel, connectionId);
        connections.send(connectionId, "RECEIPT\nreceipt-id:" + receiptID + "\n\n");

        System.out.println(currentUser.getUserName() +" SUCCESSFULLY UNSUBSCRIBED FROM " + channel + "!!!!!!!!!!!!!!!!!!");
    }

    /************************************
     * SEND
     ***************************************/
    private void handleSend(String[] msgLines) {
        String destination = splitHeaderValue(msgLines[1]);
        
        if (currentUser.getSubscriptionID(destination) == null) {
            sendErrorFrame("User isn't subscribed to " + destination);
            return;
        }

        System.out.println(currentUser.getUserName() + " IS SENDING MESSAGES TO ALL THE CHICKS FROM " + destination + ". BE READY AND BRACE YOURSELF!");
        String body = getBodyMessage(msgLines);
        String message = "MESSAGE\nsubscription: " + currentUser.getSubscriptionID(destination) 
                    + "\nmessage-id: " + connections.getMessageID()
                    + "\ndestination: " + destination
                    + "\n\n"
                    + body
                    + "\n"
                    ;

        connections.send(destination, message);

        System.out.println("ALL THE CHICKS FROM " + destination +" HAVE RECIEVED SUCCESSFULLY THE MESSAGE!!!!!!!!!!!!!!!!!!!!!!!!");
    }
    
    public void sendErrorFrame(String msg) {
        System.out.println("ERROR DETECTED - CALL THE POWERPUFF GIRLS!!!!!!!!!!!!!!!!!!!!!");
        
        connections.send(connectionId, "ERROR\nmessage: " + msg + "\n\n");
        connections.disconnect(connectionId);
        shouldTerminate = true;
    }

    private String getBodyMessage(String[] msgLines) {
        String body = "";
        int index = 0;
        while (!msgLines[index].equals("")) {
            index++;
        }
        index++;
        while (index < msgLines.length) {
            body = body + msgLines[index] + "\n";
            index++;
        }

        return body;
    }

    public String splitHeaderValue(String header) {
        String[] headerParts = header.split(":");
        return headerParts[1];
    }
}