package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;

public class User {
    private int connectionId;
    private String userName;
    private String password;
    private boolean isConnected;
    private ConnectionHandler<String> connectionHandler;
    // private HashMap<String, List<Frame>> userLog; // MA ZE

    public User(String userName, String password) {
        this.connectionId = -1;
        this.userName = userName;
        this.password = password;        
        this.isConnected = false;
        this.connectionHandler = null;  
        // this.userLog = new HashMap(); // MA ZE
    }

    // GETTERS
    public int getConnectionId() {
        return connectionId;
    }

    public String getUserName() {
        return userName;
    }

    public String getPassword() {
        return password;
    }

    // public ConcurrentHashMap<Integer, String> getChannelSubscriptions() {
    //     return channelSubscriptions;
    // }

    public boolean isConnected() {
        return isConnected;
    }

    public ConnectionHandler<String> getConnectionHandler() {
        return connectionHandler;
    }

    // SETTERS
    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

    public void setConnectionHandler(ConnectionHandler<String> connectionHandler) {
        this.connectionHandler = connectionHandler;
    }

    public void connect(int connectionId, ConnectionHandler<String> connectionHandler) {
        setConnectionId(connectionId);
        setConnectionHandler(connectionHandler);
        setConnected(true);
    }

    public void disconect() {
        setConnectionHandler(null);
        setConnectionId(-1);
        setConnected(false);
    }

}
