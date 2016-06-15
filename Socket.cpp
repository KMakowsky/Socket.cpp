//
//  Socket.cpp
//  SocketServer
//
//  Created by Kay Makowsky on 15.06.16.
//  Copyright © 2016 Kay Makowsky. All rights reserved.
//

#include "Socket.h"

Socket::Socket(){
    
}
Socket::Socket(int domain, int type, int protocol)
{
    memset(&address_info, 0, sizeof address_info);
    sock = socket(domain, type , protocol);
    if (sock < 0) {
        //exit(1);
        cerr << "opening socket error: " << gai_strerror(sock);
    }
    address_info.ai_family = domain;
    address_info.ai_socktype = type;
    address_info.ai_protocol = protocol;
    port = "";
    address = "";
}

int Socket::bind(string ip, string port){
    address = ip;
    this->port = port;
    int status;
    struct addrinfo *res;
    address_info.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(ip.c_str(), port.c_str(), &address_info, &res)) != 0) {
        cerr << "getaddrinfo error: " << gai_strerror(errno);
        //exit(1);
        return status;
    }
    address_info.ai_addrlen = res->ai_addrlen;
    address_info.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    status = ::bind(sock, address_info.ai_addr, address_info.ai_addrlen);
    if (status < 0) {
        //exit(1);
        cerr << "bind error: " << gai_strerror(errno);
    }
    return status;
}

int Socket::connect(string ip, string port){
    address = ip;
    this->port = port;
    struct addrinfo *res;
    int status;
    if ((status = getaddrinfo(ip.c_str(), port.c_str(), &address_info, &res)) != 0){
        cerr << "getaddrinfo error: " << gai_strerror(errno);
        //exit(1);
        return status;
    }
    address_info.ai_addrlen = res->ai_addrlen;
    address_info.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    status = ::connect(sock, address_info.ai_addr, address_info.ai_addrlen);
    if (status < 0) {
        //exit(1);
        cerr << "connect error: " << gai_strerror(errno);
    }
    return status;
}

int Socket::listen(int max_queue){
    int status;
    status = ::listen(sock,max_queue);
    if (status < 0) {
        //exit(1);
        cerr << "listen error: " << gai_strerror(errno);
    }
    return status;
}

Socket* Socket::accept(){
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    addr_size = sizeof their_addr;
    int newsock = ::accept(sock, (struct sockaddr *)&their_addr, &addr_size);
    if (newsock < 0) {
        //exit(1);
        cerr << "accept error: " << gai_strerror(newsock);
    }
    Socket *newSocket = new Socket(address_info.ai_family,address_info.ai_socktype,address_info.ai_protocol);
    newSocket->sock = newsock;
    newSocket->port = port;
    
    char host[NI_MAXHOST];
    int status = getnameinfo((struct sockaddr *)&their_addr, sizeof(their_addr), host, sizeof(host), NULL, 0, NI_NUMERICHOST);
    if (status < 0) {
        //exit(1);
        cerr << "getnameinfo error: " << gai_strerror(errno);
    }
    newSocket->address = host;
    newSocket->address_info.ai_family = their_addr.ss_family;
    newSocket->address_info.ai_addr = (struct sockaddr *)&their_addr;
    return newSocket;
}
int Socket::socket_write(string msg){
    const char * buf = msg.c_str();
    int len = strlen(buf);
    int status = send(sock, buf, len, 0);
    if (status < 0) {
        //exit(1);
        cerr << "write error: " << gai_strerror(errno);
    }
    return status;
}
int Socket::socket_read(string &buf,int len){
    char buffer[len];
    bzero(buffer,len);
    int status = recv(sock, buffer, len-1, 0);
    if (status < 0) {
        //exit(1);
        cerr << "read error: " << gai_strerror(errno);
    }
    buf = string(buffer);
    return status;
}
int Socket::socket_writeTo(string msg, string ip, string port){
    const char * buf = msg.c_str();
    int len = strlen(buf);
    address = ip;
    this->port = port;
    struct addrinfo *res;
    int status;
    if ((status = getaddrinfo(ip.c_str(), port.c_str(), &address_info, &res)) != 0){
        cerr << "getaddrinfo error: " << gai_strerror(errno);
        //exit(1);
        return status;
    }
    address_info.ai_addrlen = res->ai_addrlen;
    address_info.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    status = sendto(sock, buf, len, 0, address_info.ai_addr, address_info.ai_addrlen);
    if (status < 0) {
        //exit(1);
        cerr << "writeTo error: " << gai_strerror(errno);
    }
    return status;
}
int Socket::socket_readFrom(string &buf, int len, string ip, string port){
    char buffer[len];
    bzero(buffer,len);
    struct addrinfo *res;
    int status;
    if ((status = getaddrinfo(ip.c_str(), port.c_str(), &address_info, &res)) != 0){
        cerr << "getaddrinfo error: " << gai_strerror(errno);
        //exit(1);
        return status;
    }
    address_info.ai_addrlen = res->ai_addrlen;
    address_info.ai_addr = res->ai_addr;
    freeaddrinfo(res);
    status = recvfrom(sock, buffer, len-1, 0,address_info.ai_addr, &address_info.ai_addrlen);
    if (status < 0) {
        //exit(1);
        cerr << "readFrom error: " << gai_strerror(errno);
    }
    buf = string(buf);
    return status;
}

int Socket::socket_set_opt(int level, int optname, void* optval){
    unsigned int len = sizeof(optval);
    int status = ::setsockopt(sock,level,optname,optval,len);
    if (status < 0) {
        //exit(1);
        cerr << "socket_set_opt error: " << gai_strerror(errno);
    }
    return status;
}

int Socket::socket_get_opt(int level, int optname, void* optval){
    unsigned int len = sizeof(optval);
    int status = ::getsockopt(sock,level,optname,optval,&len);
    if (status < 0) {
        //exit(1);
        cerr << "socket_get_opt error: " << gai_strerror(errno);
    }
    return status;
}

int Socket::socket_shutdown(int how){
    int status = ::shutdown(sock, how);
    if (status < 0) {
        //exit(1);
        cerr << "shutdown error: " << gai_strerror(errno);
    }
    return status;
}

void Socket::close(){
    ::close(sock);
}

int Socket::select(vector<Socket> *reads, vector<Socket> *writes, vector<Socket> *exceptions,int seconds){
    struct timeval tv;
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    
    int maxSock = 0;
    
    if(reads != NULL){
        FD_ZERO(&readfds);
        for (int i = 0; i < reads->size(); i++) {
            int sockInt = reads->at(i).sock;
            if (sockInt > maxSock) {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &readfds);
        }
    }
    if(writes != NULL){
        FD_ZERO(&writefds);
        for (int i = 0; i < writes->size(); i++) {
            int sockInt = writes->at(i).sock;
            if (sockInt > maxSock) {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &writefds);
        }
    }
    if(exceptions != NULL){
        FD_ZERO(&exceptfds);
        for (int i = 0; i < exceptions->size(); i++) {
            int sockInt = exceptions->at(i).sock;
            if (sockInt > maxSock) {
                maxSock = sockInt;
            }
            FD_SET(sockInt, &exceptfds);
        }
    }
    
    // Um writefds und exceptfds kümmern wir uns nicht:
    int result = ::select(maxSock+1, &readfds, &writefds, &exceptfds, &tv);
    
    if (result < 0) {
        //exit(1);
        cerr << "select error: " << gai_strerror(errno);
    }
    
    if (reads != NULL) {
        for (int i = reads->size()-1; i >= 0; i--) {
            if (!FD_ISSET(reads->at(i).sock, &readfds)) {
                reads->push_back(reads->at(i));
            }
        }
    }
    if (writes != NULL) {
        for (int i = writes->size()-1; i >= 0; i--) {
            if (!FD_ISSET(writes->at(i).sock, &writefds)) {
                writes->push_back(writes->at(i));
            }
        }
    }
    if (exceptions != NULL) {
        for (int i = exceptions->size()-1; i >= 0; i--) {
            if (!FD_ISSET(exceptions->at(i).sock, &exceptfds)) {
                exceptions->push_back(exceptions->at(i));
            }
        }
    }
    return result;
}
