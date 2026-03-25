//
// Created by Tal on 13/03/2026.
//

#ifndef MAMAN15_DEFNSIVE_PROGRAMING_CLIENT_H
#define MAMAN15_DEFNSIVE_PROGRAMING_CLIENT_H

#include <iostream>
#include <cstdlib>
#include <boost/asio.hpp>
#include <cstring>
#include <fstream>
#include <vector>
#include <iomanip>

using boost::asio::ip::tcp;

//
#pragma pack(push,1)
struct RequestHeader {
    uint8_t  id[16];      // Your Client ID
    uint8_t  version;     // Server version
    uint16_t code;        // Request code (e.g., 701)
    uint32_t payloadSize; // Size of payload (0 for 701)
};
#pragma pack(pop)

#pragma pack(push,1)

struct ResponseHeader {
    uint8_t  version;
    uint16_t code;
    uint32_t payloadSize;
};
#pragma pack(pop)


struct Client {
    uint8_t  id[16];
    char name[255];
};


struct Message {
    uint8_t from_client_id[16];
    uint32_t message_id;
    uint8_t message_type;
    uint32_t content_size;
    std::vector<uint8_t> content;
};


std::vector<std::string> get_connection(std::string file_name);
void store_data_my_info(const std::string& name, const uint8_t id[16]);
void request_100(tcp::socket& socket);
void request_120(tcp::socket& socket, std::vector<Client> &phone_book, bool show_list);
void request_140(tcp::socket& socket, std::vector<Client>& phone_book);
void request_150(tcp::socket& socket, std::vector<Client>& client_book );
bool load_data_my_info(Client &client_return);
void print_message(Message &msg , std::vector<Client>& phone_book);
#endif //MAMAN15_DEFNSIVE_PROGRAMING_CLIENT_H