#ifndef PNETWORK_HPP
#define PNETWORK_HPP

#include <thread>
#include <atomic>
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
        void preRun();
        void run();
        void postRun();

        void kick(bool force);

        void handleJoystick();
    private:
        std::vector<signed char> mBuffer;
        std::function<void()> mDecoder;

        sf::Uint16 mPort;
        sf::TcpListener mListener;
        sf::TcpSocket mSocket;
        sf::Socket::Status mStatus;

        char mFirst;
        size_t mMaxLen;
        size_t mRecvLen;
};

#endif // PNETWORK_HPP
