package bgu.spl.net.impl.stomp;

import java.util.Map;
import java.util.HashMap;

public class User {
    private int connectionId;
    private String userName;
    private String password;
    private boolean isConnected;
    private Map<String, String> subscribeIdToChannels; 

    public User(String userName, String password) {
        this.connectionId = -1;
        this.userName = userName;
        this.password = password;        
        this.isConnected = false;
        this.subscribeIdToChannels = new HashMap<>();
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

    public void addSub(String channel, String subscriptionID) {
        this.subscribeIdToChannels.put(channel, subscriptionID);
    }

    public String removeSub(String subscriptionID){
        return this.subscribeIdToChannels.remove(subscriptionID);
    }

    public String getSubscriptionID (String channel) {
        return subscribeIdToChannels.get(channel);
    }

    // SETTERS
    public void setConnectionId(int connectionId) {
        this.connectionId = connectionId;
    }

    public void setConnected(boolean connected) {
        isConnected = connected;
    }

}
