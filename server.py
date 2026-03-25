import binascii
import threading
import uuid
from datetime import datetime
import socket
import struct
# from wsgiref.validate import header_re

from clients import Clients
from messages import Message
CLIENT_HEADER_FORMAT = '<16s B H I'
SERVER_HEADER_FORMAT = '<B H I'
CLIENT_HEADER_SIZE = struct.calcsize(CLIENT_HEADER_FORMAT)
SERVER_VERSION = 1
MESSAGE_ID_COUNTER = 0

def main():

    PORT = get_port()
    clients ={} #
    message ={} #

    storge_lock = threading.Lock()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("0.0.0.0", PORT) )
        s.listen()

        while True:
            conn, addr = s.accept()

            t = threading.Thread(target = client_handler, args = (conn, addr, clients, message, storge_lock))
            t.start()



def client_handler(conn, addr, clients, message, lock):

    with conn:
        print(f"Connected by {addr}")
        while True:
            header_data = conn.recv(CLIENT_HEADER_SIZE)
            client_header_stats =  open_heder(header_data)

            if client_header_stats["payload_size"] > 0:
                client_side_payload = conn.recv( client_header_stats["payload_size"] )
            else:
                client_side_payload  = b''

            match client_header_stats["code"]:

                case 700:  ## Register: **PayLoad** Name(255 bytes)

                    name = struct.unpack("255s", client_side_payload)[0]
                    name = name.decode('utf-8', errors='ignore')
                    name = name.split('\0')[0]


                    with lock:
                        name_exist = next((c for c in clients.values() if c.user_name() == name), None)

                        if name_exist:
                            name_exist.update_last_seen(datetime.now())
                            response_protocol(conn, SERVER_VERSION, 2100, name_exist.client_id())
                        else:
                            client_header_stats["client_id"] = uuid.uuid4().bytes  # The server need to generate an id
                            server_pay_load_2000 = request_700(name, client_header_stats, clients, message)
                            response_protocol(conn, SERVER_VERSION, 2100, server_pay_load_2000)




                case 701:  ## List of Clients: **NO client_side_payload**

                    with lock:
                        if client_header_stats["client_id"] not in clients:
                            response_protocol(conn, SERVER_VERSION, 9000, b"")
                        else:
                            server_payload_2101 = request_701(client_header_stats, clients)
                            response_protocol(conn, SERVER_VERSION, 2101, server_payload_2101)



                case 702: ## Pop public key of client: NOT IMPLEMENTED
                    raise NotImplemented


                case 703: ## Send message to client: **PayLoad** Client ID(16 bytes), Message Type(1 byte always 3)
                          ## Content Type(4 bytes), Message content(Interoperable)

                    send_to_client_id, message_type, content_size = struct.unpack("<16s B I", client_side_payload[:21])
                    message_content = client_side_payload[21:].decode().split("\0")[0]

                    with lock:
                        if client_header_stats["client_id"] not in clients or send_to_client_id not in clients:   ##   Check if client id of sender
                          response_protocol(conn, SERVER_VERSION, 9000, b"")                ##   And receiver is existing in client array


                        if content_size == 0: ## Check if message is empty
                            response_protocol(conn, SERVER_VERSION, 9000, b"")   ##   And receiver is existing in client array


                        server_pay_load_2103 = request_703(send_to_client_id, message_content, client_header_stats, message)
                        response_protocol(conn, SERVER_VERSION, 2103, server_pay_load_2103)


                case 704: ## Pop waiting messages list: **NO PayLoad**

                    with lock:
                        server_pay_load_2104 = request_704(client_header_stats["client_id"], message)
                        response_protocol(conn, SERVER_VERSION, 2104, server_pay_load_2104)


def request_700(name, client_header, clients, messages):

    now = datetime.now()


    clients[client_header["client_id"]] = Clients(client_header["client_id"], name, now)
    messages[client_header["client_id"]] = []

    print(f"deploy {client_header['code']} in server.")
    return client_header["client_id"]




def request_701(client_header, clients):

    pay_load_buffer = b""

    for values in clients.values():

        if client_header["client_id"] == values.client_id():
            continue

        name_in_bytes = values.user_name().encode()
        tamp_pay_load = struct.pack("<16s 255s", values.client_id(), name_in_bytes)
        pay_load_buffer += tamp_pay_load

    print(f"deploy {client_header['code']} in server.")
    return pay_load_buffer



def request_702():
    return NotImplemented

def answer_2102():
    return NotImplemented



def request_703(send_to_client_id, message_content, client_header, message):
    global MESSAGE_ID_COUNTER
    message_id = MESSAGE_ID_COUNTER
    MESSAGE_ID_COUNTER += 1

    message[send_to_client_id].append( Message(message_id, send_to_client_id, client_header["client_id"], 3, message_content) )

    print(f"deploy {client_header['code']} in server.")
    return struct.pack("<16s I", send_to_client_id, message_id)



def request_704(client_id, messages):

    answer_pay_load = b""
    for message in messages[client_id]:
        data =  struct.pack("<16s I B I ",  message.get_from_client(), message.get_message_id()
                            ,message.get_message_type(), len(message.get_content()))
        data = data + message.get_content().encode()
        answer_pay_load += data

    messages[client_id].clear()

    print(f"deploy {704} in server.")
    return answer_pay_load




def response_protocol(conn, server_version, code, payload):

    if isinstance(payload, str):
        payload = payload.encode()

    payload_size = len(payload)
    header = struct.pack(SERVER_HEADER_FORMAT, server_version, code, payload_size)
    conn.sendall(header + payload)
    print(f"Sent Response {code} to client.")



def get_port():

    try:
        with open("myport.info", "r") as f:
            port = int( f.read().strip() )

    except FileExistsError:
        print("The file is not exist || Default Port: 1357")
        port  = 1357

    return port


def open_heder(raw_bytes):
    header_format = '<16s B H I'

    id, ver, code, pay_size = struct.unpack(header_format, raw_bytes)

    return {"client_id" : id, "client_version": ver, "code": code, "payload_size": pay_size}




main()
#8469145-20937