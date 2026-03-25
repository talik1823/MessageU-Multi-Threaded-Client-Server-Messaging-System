//
// Created by Tal on 05/03/2026.
//

#include "Client.h"
using boost::asio::ip::tcp;

int main()
{
    try
    {
        boost::asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::socket socket(io_context);

        std::vector<std::string> connection_parm = get_connection("server.info");

        if(connection_parm.size() < 2 ) {
            std::cerr << "Invalid server.info format. Expected IP:PORT" << std::endl;
            throw std::runtime_error("Error: Not valid server.info file format OR file not open| Line 20");
        }

        std::string ip = connection_parm[0];
        std::string port = connection_parm[1];
        std::vector<Client> phone_book;

        boost::asio::connect(socket, resolver.resolve(ip, port));
        std::cout << "Connected to " << ip << ":" << port << " successfully!" << std::endl;

        bool still_run = true;
        while(still_run)
        {
            std::cout << "\nMessageU client at your service." << std::endl;
            std::cout << "110) Register" << std::endl;
            std::cout << "120) Request for clients list" << std::endl;
            std::cout << "130) Request for public key || Not implemented" << std::endl;
            std::cout << "140) Request for waiting messages" << std::endl;
            std::cout << "150) Send a text message" << std::endl;
            std::cout << "151) Send a request for symmetric key || Not implemented" << std::endl;
            std::cout << "152) Send your symmetric key || Not implemented" << std::endl;
            std::cout << "0) Exit client" << std::endl;

            int code;

            std::cin >> code;

            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // clean the buffer from \n that make lots of bugs
            switch (code) {

                case 110: // Register
                    request_100(socket);

                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;


                case 120: // Request for clients list
                    request_120(socket, phone_book, true);

                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;


                case 140: // Request for waiting messages
                    request_120(socket, phone_book, false);
                    request_140(socket, phone_book);

                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    break;


                case 150: // Send a text message
                    request_120(socket, phone_book, false);
                    request_150(socket, phone_book);
                    break;


                case 0:
                    still_run = false;
                    break;

            }
        }
    }
    catch(std::exception &e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        std::cout << "server responded with an error" << std::endl;
        std::cout << "server responded with an error" << std::endl;
    }
    return 0;
}


std::vector<std::string> get_connection(std::string file_name) {
//    std::cout <<  "Current path is: " << std::filesystem::current_path() << std::endl;
    std::ifstream file(file_name);
    std::vector<std::string> result;

    if (!file) {
        std::cerr << "Failed to open file\n" << std::endl;

        return result;
    }

    std::string line;
    std::getline(file, line);
    size_t pos = line.find(':' );

    if( pos == std::string::npos) {
        std::cerr << "There is no \':\' in the line\n";
        return result;

    }

    result.push_back( line.substr(0, pos) );
    result.push_back( line.substr(pos + 1) );

    return result;
}


void request_100(tcp::socket& socket)
{
    std::cout << "Enter the User name." << std::endl;

    char name[255];
    std::cin.getline(name, 255);

    RequestHeader request = {0};
    request.version = 1;
    request.code = 700;
    request.payloadSize = 255;

    boost::asio::write(socket, boost::asio::buffer(&request, sizeof(request)) );
    boost::asio::write(socket, boost::asio::buffer(&name, 255) );

    ResponseHeader response;
    boost::asio::read(socket,  boost::asio::buffer(&response, sizeof(response)) );

    if (response.code == 2100) {
        uint8_t id[16];
        boost::asio::read(socket, boost::asio::buffer(id, 16));
        store_data_my_info(name, id);
        std::cout << "\tYou are register to User name: <" << name <<"> with id: " << id << std::endl;
    }
    else
    {
        std::cout << "Error responses code: 9000." << std::endl;
    }
}



void request_120(tcp::socket& socket , std::vector<Client> &phone_book, bool show_list) {

    Client longed_in_client = {0};

    if( !load_data_my_info(longed_in_client) ) {
        std::cout << "There is an error with \"my.info\" " << std::endl;
    }

    RequestHeader request = {0};
    memcpy(request.id, longed_in_client.id, 16);
    request.payloadSize = 0;
    request.version = 1;
    request.code = 701;

    boost::asio::write(socket, boost::asio::buffer(&request, sizeof(request)) );

    ResponseHeader response;
    boost::asio::read(socket, boost::asio::buffer(&response, sizeof(response)));

    if(response.code == 2101) {

        std::vector<uint8_t> buffer(response.payloadSize);
        boost::asio::read(socket, boost::asio::buffer(buffer.data(),buffer.size() ));

        phone_book.clear();

        size_t offset = 0;
        size_t client_index = 1;


        if(show_list)
            std::cout << "\tContact List of client " << longed_in_client.name << std::endl;

        while ( offset + 271 <= buffer.size()) {
            Client tamp  = {0};

            memcpy(tamp.id, &buffer[offset], 16);

            offset += 16;

            char * name_ptr = reinterpret_cast<char*>(&buffer[offset]);
            memcpy(tamp.name, name_ptr, 255);

            offset += 255;

            phone_book.push_back(tamp);


                                // if used in request 140(send message) there is no need in printing the list in
            if(show_list) {     // if used in request 120(show list of client) there is a need in displaying the list
                std::cout << client_index << ") " << " User: " << tamp.name << std::endl;
                client_index += 1;
            }
        }

    } else{
        std::cout << "Error responses code: 9000." << std::endl;
    }

}

void request_140(tcp::socket& socket, std::vector<Client>& phone_book) {  // getting waiting list

    Client longed_in_client = {0};

    if( !load_data_my_info(longed_in_client) ) {
        std::cout << "There is an error with \"my.info\" " << std::endl;
    }

    RequestHeader request = {0};
    request.code = 704;
    request.payloadSize = 0;
    request.version = 1;
    memcpy(request.id, longed_in_client.id, 16);

    boost::asio::write(socket, boost::asio::buffer(&request, sizeof(request)) );

    ResponseHeader response = {0};

    boost::asio::read(socket, boost::asio::buffer(&response, sizeof(response)));

    if(response.code == 2104){

        std::vector<uint8_t> buffer(response.payloadSize);
        boost::asio::read(socket, boost::asio::buffer(buffer.data(), buffer.size()) );

        size_t offset = 0;
        while( offset < buffer.size() ) {

            Message msg = {0};

            std::memcpy(msg.from_client_id, &buffer[offset], 16);
            offset += 16;

            std::memcpy(&msg.message_id, &buffer[offset], 4);
            offset += 4;

            std::memcpy(&msg.message_type, &buffer[offset], 1);
            offset += 1;

            std::memcpy(&msg.content_size, &buffer[offset], 4);
            offset += 4;

            if (offset + msg.content_size <= buffer.size()) {
                msg.content.assign(offset + buffer.begin(), buffer.begin() + offset + msg.content_size);
                offset += msg.content_size;

            } else {
                std::cerr << "There is a buffer overflow in the buffer of the waiting message." << std::endl;
                throw std::runtime_error("Error: buffer overflow in waiting messages| Line 254");

            }

            print_message( msg, phone_book);
        }

    } else {
        std::cerr << "Error responses code: 9000." << std::endl;
        throw std::runtime_error("Error: Code 9000 response| Line 260");

    }
}

void request_150(tcp::socket& socket, std::vector<Client>& phone_book ) {

    Client longed_in_client = {0};

    if( !load_data_my_info(longed_in_client) ) {
        std::cout << "There is an error with \"my.info\" " << std::endl;
    }

    RequestHeader request = {0};
    request.code = 703;
    request.version = 1;
    std::memcpy(request.id, longed_in_client.id, 16);

    std::string name;
    std::cout << "Enter the name of the recipient:" << std::endl;
    std::getline(std::cin, name);

    uint8_t target_id [16]  = {0};
    bool found = false;

    for( Client client : phone_book) {
        if ( std::strcmp(client.name, name.c_str()) == 0) {
            std::memcpy(target_id, client.id, 16);
            found = true;
            break;
        }
    }

    if(!found) {
        std::cerr << "There is no user name: " << name << std::endl;
        throw std::runtime_error("There is no user name" + name);
    }

    std::string text_message;
    std::cout << "Enter the message text" << std::endl;
    std::getline(std::cin, text_message);

    uint32_t message_size = static_cast<uint32_t>(text_message.length());
    uint32_t payload_size = 16 + 4 + 1 + message_size;

    std::vector<uint8_t> payload;
    payload.reserve((payload_size));

    request.payloadSize = payload_size;

    payload.insert(payload.end(), target_id, target_id + 16);

    payload.push_back(3);

    uint8_t size_bytes[4];
    std::memcpy(size_bytes, &message_size, 4);
    payload.insert(payload.end(), size_bytes, size_bytes + 4);

    payload.insert(payload.end(), text_message.begin(), text_message.end());

    boost::asio::write(socket, boost::asio::buffer(&request, sizeof(request)) );
    boost::asio::write(socket, boost::asio::buffer(payload));


    ResponseHeader response = {0};
    boost::asio::read(socket, boost::asio::buffer(&response, sizeof(response)) );


    if(response.code == 2103) {

        uint8_t response_payload[20];
        boost::asio::read(socket, boost::asio::buffer(&response_payload, 20));

        std::cout << "The message was successfully send." << std::endl;

    } else {
        std::cerr << "Error responses code: 9000." << std::endl;
        throw std::runtime_error("Error responses code: 9000| Line 341");

    }
}


void store_data_my_info(const std::string& name, const uint8_t id[16]) {
    std::ofstream file("my.info");

    if (!file) {
        std::cerr << "Failed to open file\n" << std::endl;
        return ;
    }

    file << name << "\n";
    for( int i = 0; i < 16; i++ )
        file << std::hex << std::setw(2) << std::setfill('0') << (int)id[i];

    file << "\n";
    std::cout << "\n\tThe client id and name are stored in my.info" << std::endl;
    file.close();
}



bool load_data_my_info(Client &client_return) {

    std::ifstream file("my.info");

    if (!file) {
        std::cerr << "Failed to open file\n" << std::endl;
        return false;
    }
    std::string name_line;
    std::string id_line;

    if ( !std::getline(file, name_line) || name_line.length() > 255)
        return false;

    std::strncpy(client_return.name, name_line.c_str(), sizeof(client_return.name)-1);


    if( !std::getline(file, id_line) || id_line.length() < 32)
        return false;

    for(int i = 0; i < 16; i++) {

        std::string b = id_line.substr(i*2, 2);
        client_return.id[i] = static_cast<uint8_t>( std::stoul(b, nullptr, 16) );
    }

    file.close();
    return true;
}

void print_message(Message &msg , std::vector<Client>& phone_book) {

    std::string name;
    for( Client client : phone_book) {
        if ( std::memcmp(client.id, msg.from_client_id, 16) == 0 ) {
            name  = client.name;
            break;
        }
    }

    std::cout << "\nFrom:" << "<" <<  name << ">" << "\n";
    std::string content(msg.content.begin(), msg.content.end());

    std::cout << "Content:" << "<" <<  content << ">"<< "\n";
    std::cout << "\n------<EOF>>------" << std::endl;

}