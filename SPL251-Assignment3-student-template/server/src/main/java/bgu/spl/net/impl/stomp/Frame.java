package bgu.spl.net.impl.stomp;

import java.util.HashMap;
import java.util.Map;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.ConnectionsImpl;


public class Frame {

    private String command;
    private Map<String, String> headers; //HeaderType & Value
    private String body;
    
    private int connectionID;
    private ConnectionsImpl<String> connections = ConnectionsImpl.getInstance();     // Singleton ref
    private ConnectionHandler<String> connectionHandler; // Client's CH
    
    public Frame(String message, int connectionID, ConnectionHandler<String> connectionHandler) {
        this.headers = new HashMap<>();
        this.connectionID = connectionID;
        this.connectionHandler = connectionHandler;
    }

    public String getCommand() {
        return command;
    }

    public Map<String,String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

    public int getConnectionID() {
        return connectionID;
    }

    public ConnectionHandler<String> getConnectionHandler() { 
        return connectionHandler;
    }

}
