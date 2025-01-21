package bgu.spl.net.impl.stomp;

import java.util.LinkedList;
import java.util.List;

public class Frame {

    private String command;
    private List<String> headers; //HeaderType & Value
    private String body;
    
    public Frame(String message) {
        this.headers = new LinkedList<>();

        // Spliting msg 
        String[] msgLines = message.split("\n");
    
        // First line is the command
        command = msgLines[0]; 
        int index;

        for (index = 1 ;index < msgLines.length; index++) {
            // Break when finished headers.
            if (msgLines[index].equals("")){
                break;
            }
            headers.add(msgLines[index]);
        }
        
        // After the empty line comes the body.
        index++;
        this.body = msgLines[index];
    }

    public String getCommand() {
        return command;
    }

    public List<String> getHeaders() {
        return headers;
    }

    public String getBody() {
        return body;
    }

}
