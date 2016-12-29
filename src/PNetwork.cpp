#include "PNetwork.hpp"

using namespace std;

PNetwork::PNetwork() : mPort(4444), mListener(), mSocket(), mStatus(sf::Socket::Disconnected)
{
    mListener.setBlocking(false);
    mSocket.setBlocking(false);
}

PNetwork::~PNetwork()
{
}

void PNetwork::preRun()
{
    mListener.listen(mPort);
    cout << "\tStart Listening" <<endl;

    while(mStatus != sf::Socket::Done && getRunState()) mStatus=mListener.accept(mSocket);
    if(mStatus == sf::Socket::Done) cout << "\tConnected" <<endl;
}


void PNetwork::run()
{
    switch(mStatus)
    {
        case sf::Socket::Disconnected:
        {
            cout << "\tDisconnected" <<endl;
            while(mStatus != sf::Socket::Done && getRunState()) mStatus=mListener.accept(mSocket);
            if(mStatus == sf::Socket::Done) cout << "\tConnected" <<endl;
            break;
        }
        case sf::Socket::Done:
        {
            do
            {
                mStatus = mSocket.receive(&mFirst, 1, mRecvLen);
            }while(mStatus == sf::Socket::NotReady && getRunState());
            if(mStatus == sf::Socket::Done)
            {
                cout << "\tReceived data first step" <<endl;
                cout<<mFirst<<endl;
                switch(mFirst)
                {
                    case 'J':
                    {
                        mMaxLen=2;
                        mDecoder=std::bind(&PNetwork::handleJoystick, this);
                        break;
                    }
                    default:
                    {
                        cout << "\tInvalid Client !!! Kicking it..." <<endl;
                        kick(false);
                    }
                        case ';':
                        {
                            mStatus = sf::Socket::Disconnected;
                            break;
                        }
                }
                if(mStatus == sf::Socket::Done)
                {
                    mBuffer.resize(mMaxLen);
                    do
                    {
                        mStatus = mSocket.receive(&mBuffer[0], mMaxLen, mRecvLen);
                    }while(mStatus == sf::Socket::NotReady && getRunState());
                    if(mStatus == sf::Socket::Done && mRecvLen==mMaxLen)
                    {
                        cout << "\tReceived data second step" <<endl;
                        for(auto itr=mBuffer.begin(); itr!=mBuffer.end(); itr++)
                            cout<<static_cast<signed int>(*itr)<<endl;
                        mDecoder();
                    }
                    else
                    {
                        cout << "\tInvalid CLient !!! Kicking it..." <<endl;
                        kick(false);
                    }
                }

            }
            break;
        }
        default:
        {}
    }

}

void PNetwork::postRun()
{
    cout << "\tStop Listening" <<endl;
    kick(true);
    mListener.close();
}

void PNetwork::kick(bool force)
{
    size_t sentLen;
    do
    {
        mStatus = mSocket.send(";", 1, sentLen);
    }
    while(mStatus == sf::Socket::NotReady && (getRunState() || force));
    mSocket.disconnect();
    mStatus = sf::Socket::Disconnected;
}

//////////////////////////////////////////////////////////////////////

void PNetwork::handleJoystick()
{
    cout<<"Joystick x : "<<static_cast<signed int>(mBuffer[0])<<endl;
    cout<<"Joystick y : "<<static_cast<signed int>(mBuffer[1])<<endl;
}

