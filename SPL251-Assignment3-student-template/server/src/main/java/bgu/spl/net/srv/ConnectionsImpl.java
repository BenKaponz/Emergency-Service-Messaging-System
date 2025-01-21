package bgu.spl.net.srv;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> activeConnections; // ConnectionID & ConnectionHandler
    private final ConcurrentHashMap<String, List<Integer>> channelSubscriptions;      // Topic & Subscribers
    private final ConcurrentHashMap<String, User> allUsers;                           // Username & User

    //private final ConcurrentHashMap<Integer, String> activeUsers;                     // connectionID & username
    

    private static class connectionsImplHolder {
        private static final ConnectionsImpl instance = new ConnectionsImpl<>();
    }

    public static ConnectionsImpl getInstance() {
        return connectionsImplHolder.instance;
    }

    // Constructor
    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscriptions = new ConcurrentHashMap<>();
        this.allUsers = new ConcurrentHashMap<>();
        //this.activeUsers = new ConcurrentHashMap<>();
    }

    @Override
    public boolean send(int connectionId, T msg) {
        ConnectionHandler<T> handler = activeConnections.get(connectionId);
        if (handler != null) {
            handler.send(msg);
            return true;
        }
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        List<Integer> subscribers = channelSubscriptions.get(channel);
        if (subscribers != null) {
            for (int connectionId : subscribers) {
                send(connectionId, msg);
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        activeConnections.remove(connectionId);
        for (List<Integer> subscribers : channelSubscriptions.values()) {
            subscribers.remove(Integer.valueOf(connectionId));
        }
    }

    public void connect(int connectionId, ConnectionHandler<T> handler) {
        activeConnections.put(connectionId, handler);
    }

    public User getUser(String userName) {
        return allUsers.get(userName);
    }

    public void addUser(User user) {
        allUsers.put(user.getUserName(), user);
    }

}