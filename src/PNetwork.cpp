#include "PNetwork.hpp"

using namespace std;

PNetwork::PNetwork() : mPort(4444), mListener(), mSocket(), mStatus(sf::Socket::Disconnected), mNewDataToSend(false)
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
    stringstream ss;
    switch(command.network_p.type)
    {
        case PCommand::Network_Parameters::Network_Command::NewPosition:
        {
            /*cout << "x : "<<command.network_p.pos.x <<endl;
            cout << "y : "<<command.network_p.pos.y <<endl;*/
            ss << "PP" << ((char)(command.network_p.pos.x >> 8)) << ((unsigned char)(command.network_p.pos.x))
                << ((char)(command.network_p.pos.y >> 8)) << ((unsigned char)(command.network_p.pos.y))
                << command.network_p.pos.phi;
            mDataToSend = ss.str();
            /*for(unsigned int i=0; i<mDataToSend.length(); i++)
            {
                for(int j=0; j<8; j++)
                    cout<<(((int)((int)mDataToSend[i]) >> j) & 1);
                cout << endl;
            }*/
            mNewDataToSend = true;
            break;
        }
        case PCommand::Network_Parameters::Network_Command::NewSensorData:
        {
            switch(command.network_p.newSensorData_p.sensor)
            {
                case PCommand::Network_Parameters::NewSensorData_Parameters::Sensor::LaserGauche:
                {
                    ss << "DLG" << ((unsigned char)(command.network_p.newSensorData_p.valueInt >> 8)) << ((unsigned char)(command.network_p.newSensorData_p.valueInt));
                    break;
                }
                case PCommand::Network_Parameters::NewSensorData_Parameters::Sensor::LaserAvant:
                {
                    ss << "DLA" << ((unsigned char)(command.network_p.newSensorData_p.valueInt >> 8)) << ((unsigned char)(command.network_p.newSensorData_p.valueInt));
                    break;
                }
                case PCommand::Network_Parameters::NewSensorData_Parameters::Sensor::LaserDroit:
                {
                    ss << "DLD" << ((unsigned char)(command.network_p.newSensorData_p.valueInt >> 8)) << ((unsigned char)(command.network_p.newSensorData_p.valueInt));
                    break;
                }
                case PCommand::Network_Parameters::NewSensorData_Parameters::Sensor::Accel:
                {
                    ss << "DA" << setprecision(3) << command.network_p.newSensorData_p.valueDouble;
                    break;
                }
                case PCommand::Network_Parameters::NewSensorData_Parameters::Sensor::Magn:
                {
                    ss << "DM" << ((unsigned char)(command.network_p.newSensorData_p.valueInt >> 8)) << ((unsigned char)(command.network_p.newSensorData_p.valueInt));
                    break;
                }
            }
            mDataToSend = ss.str();
            mNewDataToSend = true;
            break;
        }
        case PCommand::Network_Parameters::Network_Command::ServoDisabled:
        {
            ss << "DS" << '\0';
            mDataToSend = ss.str();
            mNewDataToSend = true;
            break;
        }
        case PCommand::Network_Parameters::Network_Command::ServoEnabled:
        {
            ss << "DS" << 1;
            mDataToSend = ss.str();
            mNewDataToSend = true;
            break;
        }
        case PCommand::Network_Parameters::Network_Command::UsDisabled:
        {
            ss << "DU" << '\0';
            mDataToSend = ss.str();
            mNewDataToSend = true;
            break;
        }
        case PCommand::Network_Parameters::Network_Command::UsEnabled:
        {
            ss << "DU" << 1;
            mDataToSend = ss.str();
            mNewDataToSend = true;
            break;
        }
    }
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
    size_t sent;
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
            }while(mStatus == sf::Socket::NotReady && getRunState() && !mNewDataToSend);
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
                    case 'D':
                    {
                        mMaxLen=2;
                        mDecoder=std::bind(&PNetwork::handleDebug, this);
                        break;
                    }
                    case 'P':
                    {
                        mMaxLen=1;
                        mDecoder=std::bind(&PNetwork::handleMapping, this);
                        break;
                    }
                    case 'M':
                    {
                        mMaxLen=1;
                        mDecoder=std::bind(&PNetwork::handleMode, this);
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
                    if(mStatus == sf::Socket::Done && mRecvLen<=mMaxLen)
                    {
                        /*cout << "\tReceived data second step" <<endl;
                        for(auto itr=mBuffer.begin(); itr!=mBuffer.end(); itr++)
                            cout<<static_cast<signed int>(*itr)<<endl;*/
                        mDecoder();
                    }
                    else if(mStatus == sf::Socket::Done)
                        kick(false, true);
                }

            }
            else if(mNewDataToSend && mStatus != sf::Socket::Disconnected)
            {
                do{
                    mStatus = mSocket.send(mDataToSend.c_str(), mDataToSend.length(), sent);
                }while(mStatus == sf::Socket::NotReady && getRunState());
                mNewDataToSend = false;
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
    cout << "NETWORKStop" <<endl;
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
            event.network_p.motion_p.joystick_p.x = mBuffer[1];
            event.network_p.motion_p.joystick_p.y = mBuffer[2];
            pushEvent(event);
            break;
        }
        case 'R':
        {
            event.network_p.motion_p.type = PEvent::Network_Parameters::MotionParameters::MotionType::Rotation;
            if(mBuffer[1] == 'T')
            {
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationType::Trigo;
                pushEvent(event);
            }
            else if(mBuffer[1] == 'A')
            {
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationType::Anti;
                pushEvent(event);
            }
            else if(mBuffer[1] == 'S')
            {
                event.network_p.motion_p.rotation_p = PEvent::Network_Parameters::MotionParameters::RotationType::Stop;
                pushEvent(event);
            }
            else
                kick(false, true);

            break;
        }
        default:
            kick(false, true);
    }
}

void PNetwork::handleDebug()
{
    PEvent event;
    event.network_p.type = PEvent::Network_Parameters::Network_Event::DebugAction;

    switch(mBuffer[0])
    {
        case 'R':
        {
            event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::RAZDefaults;
            pushEvent(event);
            break;
        }
        case 'U':
        {
            if(mBuffer[1])
                event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::UsEnable;
            else
                event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::UsDisable;
            pushEvent(event);
            break;
        }
        case 'S':
        {
            if(mBuffer[1])
                event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::ServoEnable;
            else
                event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::ServoDisable;
            pushEvent(event);
            break;
        }
        case 's':
        {
            event.network_p.debug_p.type = PEvent::Network_Parameters::DebugActionParameters::DebugActionType::ServoValueChanged;
            event.network_p.debug_p.servoValue = mBuffer[1];
            pushEvent(event);
            break;
        }
        default:
            kick(false, true);
    }
}
void PNetwork::handleMapping()
{
    PEvent event;
    switch(mBuffer[0])
    {
        case 'R':
        {
            event.network_p.type = PEvent::Network_Parameters::Network_Event::ButtonRAZPosition;
            pushEvent(event);
            break;
        }
        default:
            kick(false, true);
    }

    pushEvent(event);
}
void PNetwork::handleMode()
{
    PEvent event;
    event.network_p.type = PEvent::Network_Parameters::Network_Event::ModeChanged;

    switch(mBuffer[0])
    {
        case 'M':
        {
            event.network_p.mode = PEvent::Network_Parameters::Mode::Manual;
            pushEvent(event);
            break;
        }
        case 'A':
        {
            event.network_p.mode = PEvent::Network_Parameters::Mode::Auto;
            pushEvent(event);
            break;
        }
        case 'D':
        {
            event.network_p.mode = PEvent::Network_Parameters::Mode::Debug;
            pushEvent(event);
            break;
        }
        default:
            kick(false, true);
    }
}
