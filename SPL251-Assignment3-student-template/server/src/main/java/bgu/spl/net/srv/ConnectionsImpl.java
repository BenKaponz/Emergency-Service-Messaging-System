package bgu.spl.net.srv;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import bgu.spl.net.impl.stomp.User;

public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> activeConnections; // ConnectionID &
                                                                                      // ConnectionHandler
    private final ConcurrentHashMap<String, CopyOnWriteArrayList<Integer>> channelSubscriptions; // Topic & Subscribers
    private final ConcurrentHashMap<String, User> allUsers; // Username & User
    private AtomicInteger messageIDGen;

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
        this.messageIDGen = new AtomicInteger(1);
    }

    @Override
    public boolean send(int connectionId, T msg) {
        synchronized (activeConnections) { /************************************** SYNCHRONIZED ****************/
            ConnectionHandler<T> handler = activeConnections.get(connectionId);
            if (handler != null) {
                handler.send(msg);
                return true;
            }
            return false;
        }
    }


    @Override
    public void send(String channel, T msg) {
        synchronized (channelSubscriptions) { /************************************** SYNCHRONIZED ****************/
            List<Integer> subscribers = channelSubscriptions.get(channel);
            if (subscribers != null) {
                for (int connectionId : subscribers) {
                    send(connectionId, msg);
                }
            }
        }
    }

    @Override
    public void disconnect(int connectionId) {
        // Deleting from connectID-CH map
        activeConnections.remove(connectionId);
        // Delete from all subscribed channels
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

    public void subscribe(String channel, int connectionID) {
        synchronized (channelSubscriptions) {  /************************************** SYNCHRONIZED ****************/
            if (!channelSubscriptions.containsKey(channel)) {
                channelSubscriptions.put(channel, new CopyOnWriteArrayList<>());
            }
            channelSubscriptions.get(channel).add(connectionID);
        }
    }

    public void unsubscribe(String channel, int connectionID) {
        synchronized (channelSubscriptions) {  /************************************** SYNCHRONIZED ****************/
            channelSubscriptions.get(channel).remove(Integer.valueOf(connectionID));
        }
    }

    public int getMessageID() {
        return this.messageIDGen.incrementAndGet();
    }

}