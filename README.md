# 🚨 Emergency Service Messaging System

A cross-platform emergency event messaging system implemented as part of the *SPL251 - Advanced Systems Programming* course at Ben-Gurion University.  
The system enables users to connect, subscribe to topics, send and receive messages, and manage state in real-time using the STOMP protocol.

## 🔧 Technologies Used

- **C++** — Client-side implementation
- **Java** — Server-side implementation (supporting both TPC and Reactor architectures)
- **STOMP Protocol** — For structured messaging
- **Git** — Version control (bonus points received)

## 💡 Project Structure

- **Client (C++)**  
  - Handles user input, message parsing, STOMP framing, connection lifecycle, and multithreaded I/O.
  - Supported commands: `login`, `subscribe`, `send`, `report`, `logout`, `summary`.

- **Server (Java)**  
  - Based on `bgu.spl.net.srv.Server`.
  - Supports two threading models: **Thread Per Client (TPC)** and **Reactor** using Java NIO.
  - Manages client sessions, topic subscriptions, and frame routing.

## ✨ Features

- Full support for the STOMP 1.2 protocol.
- Real-time event broadcasting to subscribers.
- Login & session tracking.
- Multithreaded client handling (mutex-safe).
- Graceful disconnection & logout.
- Custom event summary generation.

## 🗃️ Sample Workflow

```bash
> login host:port username password
> join team-a
> send /event team-a {"general_description":"fire", "location":"Building A"}
> logout
```

## 🚀 How to Run

### Server (Java)
```bash
cd Server
mvn package
java -jar target/Server.jar tpc 7777
```

### Client (C++)
```bash
cd Client
make
./client
```

## 📁 Folder Structure

```
.
├── Client/             # C++ Client code
├── Server/             # Java Server code
├── include/            # Shared headers
├── README.md
├── Makefile
└── ...
```

## 🧪 Tests & Debugging

- All features were manually tested with simulated users.
- Memory safety ensured via `valgrind` and code reviews.
- Server supports debugging printouts under verbose mode.

## 🎓 Course Information

- **Course**: SPL - Systems Programming
- **Institution**: Ben-Gurion University of the Negev
- **Year**: 2025

## 🧑‍💻 Authors

- **Ben Kapon**  
  Student at BGU
  [LinkedIn](https://www.linkedin.com/in/ben-kapon-523882331)

- **Itay Shaul**  
  Student at BGU
  [LinkedIn](https://www.linkedin.com/in/itay-shaul/)
