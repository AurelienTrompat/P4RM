#include "PNetwork.hpp"

using namespace std;

PNetwork::PNetwork() : mPort(4444), mListener(), mSocket(), mStatus(sf::Socket::Disconnected)
{
    setAgent(Agent::Network);
    mListener.setBlocking(false);
    mSocket.setBlocking(false);
}

PNetwork::~PNetwork()
{
}
void PNetwork::handleCommand(const PCommand& command)
{
 cout<<"test";
}

void PNetwork::preRun()
{
    mListener.listen(mPort);
    cout << "\tStart Listening" <<endl;

    while(mStatus != sf::Socket::Done && getRunState()) mStatus=mListener.accept(mSocket);
    if(mStatus == sf::Socket::Done)
    {
        //cout << "\tConnected" <<endl;
        handleConnection(true);
    }
}


void PNetwork::run()
{
    switch(mStatus)
    {
        case sf::Socket::Disconnected:
        {
            //cout << "\tDisconnected" <<endl;
            handleConnection(false);
            while(mStatus != sf::Socket::Done && getRunState()) mStatus=mListener.accept(mSocket);
            if(mStatus == sf::Socket::Done)
            {
                //cout << "\tConnected" <<endl;
                handleConnection(true);
            }
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
                /*cout << "\tReceived data first step" <<endl;
                cout<<mFirst<<endl;*/
                switch(mFirst)
                {
                    case 'm':
                    {
                        mMaxLen=3;
                        mDecoder=std::bind(&PNetwork::handleMotion, this);
                        break;
                    }
                    case 'D' :
                    {
                        mMaxLen=1;
                        mDecoder=std::bind(&PNetwork::handleDebug, this);
                        break;
                    }
                    default:
                    {
                        kick(false, true);
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
                        /*cout << "\tReceived data second step" <<endl;
                        for(auto itr=mBuffer.begin(); itr!=mBuffer.end(); itr++)
                            cout<<static_cast<signed int>(*itr)<<endl;*/
                        mDecoder();
                    }
                    else
                    {
                        kick(false, true);
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
    kick(true, false);
    mListener.close();
}

void PNetwork::kick(bool force, bool message)
{
    size_t sentLen;
    if(message)
        cout << "\tInvalid Client !!! Kicking it..." <<endl;
    do
    {
        mStatus = mSocket.send(";", 1, sentLen);
    }
    while(mStatus == sf::Socket::NotReady && (getRunState() || force));
    mSocket.disconnect();
    mStatus = sf::Socket::Disconnected;
}

//////////////////////////////////////////////////////////////////////

void PNetwork::handleConnection(bool eventType)
{
    PEvent event;
    if(eventType)
        event.network_p.type=PEvent::Network_Parameters::Network_Event::ClientConnected;
    else
        event.network_p.type=PEvent::Network_Parameters::Network_Event::ClientDisconnected;
    pushEvent(event);
}


void PNetwork::handleMotion()
{
    PEvent event;
    event.network_p.type = PEvent::Network_Parameters::Network_Event::Motion;
    /*cout<<"Joystick x : "<<static_cast<signed int>(mBuffer[0])<<endl;
    cout<<"Joystick y : "<<static_cast<signed int>(mBuffer[1])<<endl;*/
    switch(mBuffer[0])
    {
        case 'J':
        {
            event.network_p.motion_p.type = PEvent::Network_Parameters::MotionParameters::MotionType::Joystick;
            event.network_p.motion_p.joystick.x = mBuffer[1];
            event.network_p.motion_p.joystick.y = mBuffer[2];
            break;
        }
        case 'R':
        {
            event.network_p.motion_p.type = PEvent::Network_Parameters::MotionParameters::MotionType::Rotation;
            if(mBuffer[1] == 'T')
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationParameters::Trigo;
            else if(mBuffer[1] == 'A')
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationParameters::Anti;
            else if(mBuffer[1] == 'S')
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationParameters::Stop;
            else
                kick(false, true);

            break;
        }
        default:
            kick(false, true);
    }
    pushEvent(event);
}

void PNetwork::handleDebug()
{
    PEvent event;
    switch(mBuffer[0])
    {
        case 'R':
        {
            event.network_p.type = PEvent::Network_Parameters::Network_Event::ButtonRAZDefaults;
            break;
        }
        default:
            kick(false, true);
    }

    pushEvent(event);
}
