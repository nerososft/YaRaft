//
// Created by XingfengYang on 2020/1/2.
//
#include "../include/RaftCore.h"
#include "../include/RaftMessage.h"

namespace Raft {

    RaftCore::~RaftCore() noexcept = default;

    RaftCore::RaftCore() {
    }

    void RaftCore::ResetElectionTimer() {
        std::lock_guard<decltype(sharedProperties->mutex)> lock(sharedProperties->mutex);
        sharedProperties->timeOfLastLeaderMessage = timeKeeper->GetCurrentTime();
        sharedProperties->currentElectionTimeout = std::uniform_real_distribution<>(
                sharedProperties->configuration.minimumElectionTimeout,
                sharedProperties->configuration.maximumElectionTimeout
        )(sharedProperties->randomGenerator);
    }

    double RaftCore::GetTimeSinceLastLeaderMessage(double now) {
        std::lock_guard<decltype(sharedProperties->mutex)> lock(sharedProperties->mutex);
        return now - sharedProperties->timeOfLastLeaderMessage;
    }

    void RaftCore::SendMessage(std::shared_ptr<Message> message, unsigned int instanceNumber, double now) {
        auto &instance = sharedProperties->instances[instanceNumber];
        instance.timeLastRequestSend = now;
        instance.lastRequest = message;

        sendMessageDelegate(message, instanceNumber);
    }

    void RaftCore::StartElection(double now) {
        ++sharedProperties->configuration.currentTerm;

        sharedProperties->votedThisTerm = true;
        sharedProperties->votedFor = sharedProperties->configuration.selfInstanceNumber;
        sharedProperties->votesForUs = 1;

        const auto message = Message::CreateMessage();
        message->raftMessage->type = RaftMessage::Type::RequestVote;
        message->raftMessage->requestVoteDetails.candidateId = sharedProperties->configuration.selfInstanceNumber;
        message->raftMessage->requestVoteDetails.term = sharedProperties->configuration.currentTerm;

        for (auto &instance:sharedProperties->instances) {
            instance.second.awaitingVote = false;
        }
        for (auto instanceNumber: sharedProperties->configuration.instancesNumbers) {
            if (instanceNumber == sharedProperties->configuration.selfInstanceNumber) {
                continue;
            }
            auto &instance = sharedProperties->instances[instanceNumber];
            instance.awaitingVote = true;

            SendMessage(message, instanceNumber, now);
        }
        double timeOfLastLeaderMessage = timeKeeper->GetCurrentTime();
    }

    void RaftCore::SendHeartBeat(double now) {
        std::lock_guard<decltype(sharedProperties->mutex)> lock(sharedProperties->mutex);
        sharedProperties->votesForUs = 1;
        const auto message = Message::CreateMessage();
        message->raftMessage->type = RaftMessage::Type::HeartBeat;
        message->raftMessage->requestVoteDetails.term = sharedProperties->configuration.currentTerm;

        for (auto instanceNumber: sharedProperties->configuration.instancesNumbers) {
            if (instanceNumber == sharedProperties->configuration.selfInstanceNumber) {
                continue;
            }
            SendMessage(message, instanceNumber, now);
        }
        double timeOfLastLeaderMessage = timeKeeper->GetCurrentTime();
    }

    void RaftCore::DoRetransmission(double now) {
        std::lock_guard<decltype(sharedProperties->mutex)> lock(sharedProperties->mutex);
        for (auto &instanceEntry: sharedProperties->instances) {
            if (instanceEntry.second.awaitingVote &&
                (now - instanceEntry.second.timeLastRequestSend >= sharedProperties->configuration.rpcTimeout)) {
                SendMessage(instanceEntry.second.lastRequest,
                            instanceEntry.first,
                            now);
            }
        }
    }

    void RaftCore::RevertToFollower() {
        for (auto &instanceEntry: sharedProperties->instances) {
            instanceEntry.second.awaitingVote = false;
        }
        sharedProperties->isLeader = false;
        ResetElectionTimer();
    }

    void RaftCore::Worker() {
        ResetElectionTimer();
        int rpcTimeoutMilliseconds = (int) (sharedProperties->configuration.rpcTimeout * 1000.0);
        std::future<void> workerAskedToStop = stopWorker.get_future();
        std::unique_lock<decltype(sharedProperties->mutex)> lock(sharedProperties->mutex);
        while (workerAskedToStop.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            (void) workerAskedToStopOrWeakUp.wait_for(
                    lock,
                    std::chrono::milliseconds(rpcTimeoutMilliseconds)
            );

            const bool signalWorkerLoopCompletion = (sharedProperties->workerLoopCompletion != nullptr);

            lock.unlock();
            const double now = timeKeeper->GetCurrentTime();
            const double timeSinceLastLeaderMessage = GetTimeSinceLastLeaderMessage(now);
            if (sharedProperties->isLeader) {
                if (timeSinceLastLeaderMessage >= sharedProperties->configuration.minimumElectionTimeout / 2) {
                    SendHeartBeat(now);
                }
            } else {
                if (timeSinceLastLeaderMessage >= sharedProperties->currentElectionTimeout) {
                    StartElection(now);
                }
            }
            DoRetransmission(now);
            lock.lock();
            if (signalWorkerLoopCompletion) {
                sharedProperties->workerLoopCompletion->set_value();
                sharedProperties->workerLoopCompletion = nullptr;
            }
        }
        if (sharedProperties->workerLoopCompletion != nullptr) {
            sharedProperties->workerLoopCompletion->set_value();
            sharedProperties->workerLoopCompletion = nullptr;
        }
    }

    bool RaftCore::IsLeader() {
        return sharedProperties->isLeader;
    }
}