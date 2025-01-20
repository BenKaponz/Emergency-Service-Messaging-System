package bgu.spl.net.impl.stomp;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

import bgu.spl.net.srv.ConnectionHandler;
import bgu.spl.net.srv.Connections;

public class ConnectionsImpl<T> implements Connections<T> {

    private final ConcurrentHashMap<Integer, ConnectionHandler<T>> activeConnections;
    
    private final ConcurrentHashMap<String, List<Integer>> channelSubscriptions;

    public ConnectionsImpl() {
        this.activeConnections = new ConcurrentHashMap<>();
        this.channelSubscriptions = new ConcurrentHashMap<>();
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



}
