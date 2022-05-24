#include <cell.h>
#include <iostream>
#include <chrono>
#include <unistd.h>
#include <string.h>


using namespace std::chrono_literals;


namespace socket_project{

Cell::Cell(){
    this->safety = new std::mutex();
}

Cell::~Cell(){
    this->thread_connectivity->join();
    this->thread_loop->join();
    this->thread_read->join();

    delete this->thread_connectivity;
    delete this->thread_loop;
    delete this->thread_read;
}

void Cell::server(int __port){
    this->mode = SERVER;
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(this->sock, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt));

    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = INADDR_ANY;
    this->addr.sin_port = htons(__port);
    this->add_size = sizeof(this->addr);

    if(bind(this->sock, (struct sockaddr*) &this->addr, add_size) == -1){
        std::cout << "Cannot bind to address or port" << std::endl;
    }
    if(listen(this->sock, 5) == -1){
        std::cout << "Cannot listen to port" << std::endl;
    }

    this->listening = true;
    this->thread_loop = new std::thread(&Cell::connection_loop, this);
    this->thread_connectivity = new std::thread(&Cell::connectivity_loop, this);

}

void Cell::connectivity_loop(){
    while(this->listening){
        for(int index = this->connections.size() - 1 ; index >= 0 ; index--){
            int sockfd = this->connections.at(index);
            bool remove = false;

            char const *buf = "ACK";
            int resp = send(sockfd, buf, sizeof(buf), MSG_NOSIGNAL);
            remove = resp == -1 && errno == EPIPE;

            if(remove){
                this->safety->lock();
                this->connections.erase(this->connections.begin() + index);
                this->safety->unlock();
            }
        }
        std::this_thread::sleep_for(50ms);
    }
}

void Cell::connection_loop(){
    while(this->listening){
        int csock = accept(this->sock, (struct sockaddr*) &this->addr, (socklen_t*) &this->add_size);
        if(csock!=-1){
            this->safety->lock();
            this->connections.push_back(csock);
            this->safety->unlock();
        }

        std::this_thread::sleep_for(50ms);
    }
}

void Cell::stop(){
    this->listening=false;

    this->thread_loop->join();
    this->thread_loop = nullptr;

    this->thread_connectivity->join();
    this->thread_connectivity = nullptr;

    this->thread_read->join();
    this->thread_read = nullptr;

    if(this->mode==CLIENT){
        close(this->sock);
    }
}

void Cell::join(int __port){
    this->mode = CLIENT;
    this->sock = socket(AF_INET, SOCK_STREAM, 0);
    this->addr.sin_family = AF_INET;
    this->addr.sin_addr.s_addr = INADDR_ANY;
    this->addr.sin_port = htons(__port);
    this->add_size = sizeof(this->addr);

    int client_fd = connect(this->sock, (struct sockaddr*) &this->addr, this->add_size);
    if(client_fd == -1){
        std::cout << "Cannot connect to server" << std::endl;
    }

    this->listening = true;
    this->thread_read = new std::thread(&Cell::read_socket, this);
}

void Cell::read_socket(){
    while(this->listening){
        char buff[1024];
        int res = read(this->sock, buff, 1024);
        if(this->mode == CLIENT && std::string(buff) == "ACK"){
            continue;
        }
        if(res!=-1){
            std::cout << "Message: " << buff << std::endl;
        }
        std::this_thread::sleep_for(50ms);
    }
}

void Cell::message(char const *__msg, int __fd){
    if(send(__fd, __msg, strlen(__msg), 0)<0){
        std::cout << "Unable to send message" << std::endl;
    }
}

void Cell::broadcast(char const *__msg){
    if(this->mode == SERVER){
        this->safety->lock();
        for(auto __fd: this->connections){
            this->message(__msg, __fd);
        }
        this->safety->unlock();
    }
}

}