package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionId;
    private ConnectionsImpl<String> connections;

    public StompMessagingProtocolImpl(){}

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = (ConnectionsImpl<String>)connections;
    }

    @Override
    public void process(String message) {
        Frame frame = new Frame(message, connectionId, null); //user.getConnectionHandler()

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
                handleError(frame, "Invalid command: " + frame.getCommand());
        }
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    private void handleConnect(Frame frame){

        // Extract required headers
        String version = frame.getHeaders().get("accept-version");
        String host = frame.getHeaders().get("host");
        String login = frame.getHeaders().get("login");
        String passcode = frame.getHeaders().get("passcode");

        // Validate headers
    if (version == null || !version.equals("1.2")) {
        sendErrorFrame("Unsupported STOMP version. Expected version 1.2.");
        return;
    }

    if (host == null || !host.equals("stomp.cs.bgu.ac.il")) {
        sendErrorFrame("Invalid host. Expected stomp.cs.bgu.ac.il.");
        return;
    }

    if (login == null || passcode == null) {
        sendErrorFrame("Missing login or passcode.");
        return;
    }

    // Validate user credentials (example logic)
    User user = connections.getUser(login); // Fetch the user object
    if (user == null) {
        // New user, create and store it
        user = new User(login, passcode);
        connections.addUser(user);
    } else {
        // Existing user, check credentials
        if (!user.getPassword().equals(passcode)) {
            sendErrorFrame("Invalid credentials for user: " + login);
            return;
        }

        // Check if the user is already connected
        if (user.isConnected()) {
            sendErrorFrame("User already connected: " + login);
            return;
        }
    }

    // Mark user as connected
    user.connect(frame.getConnectionID(), frame.getConnectionHandler());
    connections.

    // Send CONNECTED frame
    String connectedFrame = "CONNECTED\nversion:1.2\n\n\u0000";
    connections.send(frame.getConnectionID(), connectedFrame);
    

    }
    
    public void sendErrorFrame(String msg) {
        connections.send(connectionId, msg);
        connections.disconnect(connectionId);
    }
}