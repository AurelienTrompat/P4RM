#ifndef PNETWORK_HPP
#define PNETWORK_HPP

#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <functional>
#include <iostream>
#include <iomanip>
#include "SFML/Network.hpp"

#include "PAgent.hpp"


class PNetwork : public PAgent
{
    public:
        PNetwork();
        ~PNetwork();
    private:
        void handleCommand(const PCommand &command);
        void preRun();
        void run();
        void postRun();

        void kick(bool force, bool message);

        void handleMotion();
        void handleConnection(bool eventType);
        void handleDebug();
    private:
        std::vector<unsigned char> mBuffer;
        std::function<void()> mDecoder;

        sf::Uint16 mPort;
        sf::TcpListener mListener;
        sf::TcpSocket mSocket;
        sf::Socket::Status mStatus;

        char mFirst;
        size_t mMaxLen;
        size_t mRecvLen;

        std::atomic_bool mNewDataToSend;
        std::string mDataToSend;
};

#endif // PNETWORK_HPP
