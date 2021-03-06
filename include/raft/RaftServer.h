//
// Created by XingfengYang on 2020/1/1.
//

#ifndef RAFT_RAFTSERVER_H
#define RAFT_RAFTSERVER_H

#include <memory>

#include "IRaftServer.h"
#include "../timer/TimeKeeper.h"
#include "RaftServerImpl.h"
#include "../connect/SocketImpl.h"
#include "../connect/SocketOps.h"

namespace Raft {
    class RaftServer : public IRaftServer {
    public:
        ~RaftServer() noexcept;

        RaftServer(const RaftServer &) = delete;

        RaftServer(RaftServer &&) noexcept;

        RaftServer &operator=(const RaftServer &) = delete;

        RaftServer &operator=(RaftServer &&) noexcept;

    public:
        RaftServer();

        void SetTimeKeeper(std::shared_ptr<Timer::TimeKeeper> timeKeeper);

        void Mobilize();

        void Demobilize();

        void WaitForAtLeastOneWorkerLoop();

        void SetRunning(){this->isRunning = true;}

    public:
        virtual bool Configure(const Configuration &configuration) override;

        virtual void SetSendMessageDelegate(SendMessageDelegate sendMessageDelegate) override;

        virtual void ReceiveMessage(std::shared_ptr<RaftMessage> message,
                                    unsigned int senderInstanceNumber) override;

    private:
        bool isRunning = false;
        std::shared_ptr<Connect::SocketImpl> socketOps;
        std::shared_ptr<RaftServerImpl> raftServer;
    };
}
#endif //RAFT_RAFTSERVER_H
