package bgu.spl.net.impl.stomp;

import bgu.spl.net.srv.ConnectionHandler;

public class User {
    private int connectionId;
    private String userName;
    private String password;
    private boolean isConnected;
    

    public User(String userName, String password) {
        this.connectionId = -1;
        this.userName = userName;
        this.password = password;        
        this.isConnected = false;
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

    public boolean isConnected() {
        return isConnected;
    }

    // SETTERS
    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

}
