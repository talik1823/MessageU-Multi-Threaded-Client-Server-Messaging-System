# MessageU: Multi-Threaded Client-Server Messaging System

A cross-platform messaging application featuring a **Python-based asynchronous server** and a **C++ Boost.Asio client**. This project implements a custom binary protocol to facilitate secure and efficient communication between users.

---

## 📁 Project Structure

### Server (Python)
* **`server.py`**: The main entry point. Handles socket binding, multi-threaded connection management, and request routing.
* **`clients.py`**: Manages the `Clients` class, storing user metadata, unique IDs, and "last seen" timestamps.
* **`messages.py`**: Handles the `Message` class for structuring and buffering pending messages.
* **`myport.info`**: Configuration file for the server's listening port.

### Client (C++)
* **`main.cpp`**: The interactive CLI and network logic using `boost::asio`.
* **`Client.h`**: Defines the binary structures (`#pragma pack`) and shared data types.
* **`server.info`**: Configuration file containing the server's IP and Port (e.g., `127.0.0.1:1357`).
* **`my.info`**: Local storage for the client's registered name and unique 16-byte ID.

---

## ⚙️ How It Works

### 1. Communication Protocol
The system uses a **binary header-payload protocol** to ensure low overhead. Data is packed into bytes to maintain compatibility between the Python server and C++ client.

**Request Header (Client to Server):**
| Field | Size | Type |
| :--- | :--- | :--- |
| Client ID | 16 Bytes | Byte Array |
| Version | 1 Byte | Unsigned Int |
| Request Code | 2 Bytes | Unsigned Int (Little Endian) |
| Payload Size | 4 Bytes | Unsigned Int (Little Endian) |

**Response Header (Server to Client):**
| Field | Size | Type |
| :--- | :--- | :--- |
| Version | 1 Byte | Unsigned Int |
| Response Code | 2 Bytes | Unsigned Int (Little Endian) |
| Payload Size | 4 Bytes | Unsigned Int (Little Endian) |

### 2. Supported Operations
| Code | Action | Description |
| :--- | :--- | :--- |
| **700** | Register | Sends a name; Server returns a unique 16-byte UUID. |
| **701** | List Clients | Retrieves names and IDs of all other registered users. |
| **703** | Send Message | Sends a text message to a specific Client ID. |
| **704** | Pull Messages | Downloads waiting messages and clears them from the server. |

### 3. Concurrency & Integrity
The server utilizes the `threading` module to handle each client in a separate thread, allowing for simultaneous active connections. A `threading.Lock` mechanism is used to ensure thread-safe access to the shared client database and message buffers.

---

## 🚀 Getting Started

### Prerequisites
* **Server**: Python 3.10 or higher.
* **Client**: C++17 compliant compiler and **Boost Libraries** (Asio).

### Setup & Execution
1. **Server Configuration**: Create a file named `myport.info` containing only the port number (e.g., `1357`).
2. **Start Server**:
   ```bash
   python server.py
