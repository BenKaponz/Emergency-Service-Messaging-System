package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;
import bgu.spl.net.srv.ConnectionsImpl;

public class StompMessagingProtocolImpl implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    private int connectionID;
    private Connections<String> connections;

    public StompMessagingProtocolImpl(){}

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionID = connectionId;
        this.connections = connections;
    }

    @Override
    public void process(String message) {
        Frame frame = new Frame(message, connectionID, null); //user.getConnectionHandler()

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
    
}
