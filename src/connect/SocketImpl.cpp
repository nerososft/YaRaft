//
// Created by XingfengYang on 2020/1/7.
//

#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <sys/event.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "../../include/connect/SocketImpl.h"
#include "../../include/log/Log.h"
#include "../../include/raft/RaftMessageImpl.h"
#include "../../include/connect/ConnectionPool.h"
#include "../../include/common/Common.h"

namespace Connect {

    SocketImpl::~SocketImpl() noexcept = default;

    SocketImpl::SocketImpl(Connect::SocketImpl &&) noexcept = default;

    SocketImpl &SocketImpl::operator=(SocketImpl &&) noexcept = default;

    SocketImpl::SocketImpl() : fd(-1), kq(-1), addr({}), connectionPool(std::make_shared<ConnectionPool>()) {

    }

    bool SocketImpl::Configure(const SocketConfiguration &config) {
        this->configuration = config;
    }

    int SocketImpl::SetUp() {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(configuration.port);
        addr.sin_addr.s_addr = INADDR_ANY;
        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            LogError("[Socket] Unable to Open the Socket\n");
            exit(0);
        }

        kq = kqueue();
        if (kq == -1) {
            LogError("[Socket] Unable to init kqueue\n");
            exit(0);
        }

        EV_SET(&eventSet, fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
        if (kevent(kq, &eventSet, 1, NULL, 0, NULL) == -1) {
            LogError("[Socket] Unable to init kevent\n");
            exit(0);
        }

    }

    int SocketImpl::Bind() {
        if (bind(fd, (struct sockaddr *) (&addr), sizeof(addr)) != 0) {
            LogError("[Socket] Unable to bind the Socket\n");
            exit(0);
        }
    }

    int SocketImpl::Listen() {
        if (listen(fd, 50) == -1) {
            LogError("[Socket] Unable to listen the Socket\n");
            exit(0);
        }
    }

    void SocketImpl::SetSocketAcceptEventHandler(SocketAcceptEventHandler acceptEventHandler) {
        this->socketAcceptEventHandler = acceptEventHandler;
    }

    int SocketImpl::Accept(SocketAcceptEventHandler acceptEventHandler) {
        int nev = kevent(kq, NULL, 0, eventList, 32, NULL);
        struct sockaddr_storage sockaddrStorage{};
        socklen_t socklen = sizeof(sockaddrStorage);

        for (int i = 0; i < nev; i++) {
            int fd_ = (int) eventList[i].ident;
            if (eventList[i].flags & EV_EOF) {
                close(fd_);
            } else if (fd_ == fd) {
                int connectedFd = accept(fd, (struct sockaddr *) &sockaddrStorage, &socklen);
                if (connectedFd == -1) {
                    LogError("[Socket] Unable to Connect with the client")
                } else {
                    EV_SET(&eventSet, connectedFd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &eventSet, 1, NULL, 0, NULL);

                    int flags = fcntl(connectedFd, F_GETFL, 0);
                    assert(flags >= 0);
                    fcntl(connectedFd, F_SETFL, flags | O_NONBLOCK);

                    EV_SET(&eventSet, connectedFd, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
                    kevent(kq, &eventSet, 1, NULL, 0, NULL);
                }
            } else if (eventList[i].filter == EVFILT_READ) {
                char buf[1024];
                size_t bytes_read = recv(eventList[i].ident, buf, sizeof(buf), 0);
                acceptEventHandler(buf, eventList[i].ident);
            } else if (eventList[i].filter == EVFILT_WRITE) {

            }
        }
    }

    int SocketImpl::Connect(std::string host, int port) {
        LogInfo("[Socket] Start establish connection to Server [%s:%d].\n", host.c_str(), port)
        struct sockaddr_in sockaddrIn{};
        int sock = 0;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            LogError("[Socket] Socket creation error, host: %s, port:%d\n", host.c_str(), port)
            return -1;
        }

        sockaddrIn.sin_family = AF_INET;
        sockaddrIn.sin_port = htons(port);

        if (inet_pton(AF_INET, host.c_str(), &sockaddrIn.sin_addr) <= 0) {
            LogError("[Socket] Invalid address / Address not supported, host: %s, port:%d\n", host.c_str(), port)
            return -1;
        }

        if (connect(sock, (struct sockaddr *) &sockaddrIn, sizeof(sockaddrIn)) < 0) {
            LogError("[Socket] Connection establish Failed, host: %s, port:%d\n", host.c_str(), port)
            return -1;
        }
        LogInfo("[Socket] Connection established to Server [%s:%d].\n", host.c_str(), port)
        return sock;
    }

    int SocketImpl::Send(EndPoint endPoint, char *buf) {
        // get connect from connect pool
        const Connection *connection = this->connectionPool->GetConnection(endPoint);
        if (connection == nullptr) {
            LogWarnning("[Socket] Connection not found, host %s ,port %d\n", endPoint.host.c_str(), endPoint.port)
            // if not found connection in connection pool, create new connection and put to connection pool

            int sock = this->Connect(endPoint.host, endPoint.port);
            this->connectionPool->AddConnection(endPoint, sock);
            ssize_t size = send(sock, buf, strlen(buf), 0);
            if (size < 0) {
                LogError("[Socket] Send Message to node [%s:%d] Failed.\n", endPoint.host.c_str(), endPoint.port)
            }
        } else {
            LogInfo("[Socket] Start Send message to node [%s:%d]\n", endPoint.host.c_str(), endPoint.port)
            send(connection->socketFd, buf, strlen(buf), 0);
        }

    }

    void SocketImpl::getEndPointFromFdc(int fdc, EndPoint &endPoint) {
        socklen_t len;
        struct sockaddr_storage address{};
        char ipStr[INET6_ADDRSTRLEN];
        int port;

        len = sizeof address;
        getpeername(fdc, (struct sockaddr *) &address, &len);

        // deal with both IPv4 and IPv6:
        if (address.ss_family == AF_INET) {
            struct sockaddr_in *s = (struct sockaddr_in *) &address;
            port = ntohs(s->sin_port);
            inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof ipStr);
        } else { // AF_INET6
            struct sockaddr_in6 *s = (struct sockaddr_in6 *) &address;
            port = ntohs(s->sin6_port);
            inet_ntop(AF_INET6, &s->sin6_addr, ipStr, sizeof ipStr);
        }

        endPoint.port = port;
        endPoint.host = ipStr;
    }
}
