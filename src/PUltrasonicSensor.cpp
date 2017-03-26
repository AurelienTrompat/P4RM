#include "PUltrasonicSensor.hpp"

using namespace std;

PUltrasonicSensor::PUltrasonicSensor() : mPm(PGpioManager::getInstance()),
                   mNewCommand(false), mActive(false), mSeuilUtilisateur(0), mDistanceObstacle(0), mCptErreur(0)
{
    setAgent(Agent::US);
}

PUltrasonicSensor::~PUltrasonicSensor()
{
    //dtor
}

void PUltrasonicSensor::preRun()
{
    mPm.write(Pin::CMD_ALIM_US, false);
    mPm.write(Pin::TRIG_AV, false);
    mPm.write(Pin::TRIG_AR, false);
}

void PUltrasonicSensor::run()
{
    std::chrono::duration<double> diff = Chrono::now() - mChronoMesure;

    if (mNewCommand)
    {
        switch (mUS_Command)
        {
            case us_Command::StartAvant :
            {
                mUS_Device = us_Device::CapteurAvant;
                mTrig = Pin::TRIG_AV;
                mEcho = Pin::ECHO_AV;
                Us_Start();
                break;
            }
            case us_Command::StartArriere :
            {
                mUS_Device = us_Device::CapteurArriere;
                mTrig = Pin::TRIG_AR;
                mEcho = Pin::ECHO_AR;
                Us_Start();
                break;
            }
            case us_Command::StopUS :
            {
                Us_Stop();
                break;
            }
            case us_Command::Reset :
            {
               if(mActive==true)
               {
                    Us_Stop();
                    this_thread::sleep_for(chrono::milliseconds(10));
                    Us_Start();
                    break;
               }
            }
            default :
                break;
        }
        mNewCommand = false;
    }
    else if (mActive && diff.count() > 0.06)
    {
        Us_OneMesure();
        mChronoMesure = Chrono::now();
    }
}

void PUltrasonicSensor::postRun()
{
    mActive = false;
    mPm.write(Pin::CMD_ALIM_US, false);
    mPm.write(Pin::TRIG_AV, false);
    mPm.write(Pin::TRIG_AR, false);
    cout << "USStop" <<endl;
}

void PUltrasonicSensor::handleCommand(const PCommand &command)
{
    switch(command.us_p.type)
    {
        case us_Command::StartAvant :
        case us_Command::StartArriere :
        case us_Command::Reset :
        case us_Command::StopUS :
        {
            mUS_Command = command.us_p.type;
            mNewCommand = true;
            break;
        }
        case us_Command::SetSeuil :
        {
            mSeuilUtilisateur = command.us_p.seuil;
            break;
        }
        case us_Command::GetDistance :
        {
            SendEvent(us_Event::US_Distance);
            break;
        }
    }

    while((mNewCommand & getRunState()));
}

void PUltrasonicSensor::SendEvent(us_Event typeEvent)
{
    PEvent event;
    event.us_p.type = typeEvent;
    event.us_p.device = mUS_Device;
    if (typeEvent == us_Event::US_Distance || typeEvent == us_Event::US_Obstacle)
    {
        event.us_p.distObstacle = (uint16_t)mDistanceObstacle;
        event.us_p.seuil = mUS_Seuil;
    }
    pushEvent(event);
}

void PUltrasonicSensor::Us_Start()
{
    mDernierreMesure.clear();
    mMesureOk.clear();
    mDistanceObstacle = 0;
    mPm.write(Pin::CMD_ALIM_US, true);
    this_thread::sleep_for(chrono::milliseconds(10));
    mActive = true;
}

void PUltrasonicSensor::Us_Stop()
{
    mPm.write(Pin::CMD_ALIM_US, false);
    mActive = false;
}

void PUltrasonicSensor::Us_OneMesure()
{
    mPm.write(mTrig, true);
    this_thread::sleep_for(chrono::microseconds(10));
    mPm.write(mTrig, false);

    Chrono::time_point waitEcho = Chrono::now();
    std::chrono::duration<double> diff = Chrono::now() - waitEcho;
    while (diff.count() < 0.03 && !mPm.read(mEcho))
    {
        diff = Chrono::now() - waitEcho;
    }
    waitEcho = Chrono::now();
    if(diff.count() < 0.03)
    {
        while (diff.count() < 0.05 && mPm.read(mEcho))
        {
            diff = Chrono::now() - waitEcho;

        }
        if(diff.count() < 0.05)
        {
            std::vector<double>::iterator it;
            double newMesure = (double)(diff.count()*ConvertCm);
            double MoyDMesure =0;
            if (mDernierreMesure.size() !=0)
            {
                for (it = mDernierreMesure.begin();it < mDernierreMesure.end();it++)
                MoyDMesure += *it;
                MoyDMesure = (double)(MoyDMesure / mDernierreMesure.size());
            }

            if ((((newMesure <= (MoyDMesure+20))) && (newMesure >= (MoyDMesure-20)))|| mDistanceObstacle == 0)
            {
                it = mMesureOk.begin();
                mMesureOk.insert(it,newMesure);
            }

            if (mMesureOk.size() > 3)
                mMesureOk.pop_back();
            mDistanceObstacle=0;

            if (mMesureOk.size() != 0)
            {
                 for (it = mMesureOk.begin();it < mMesureOk.end();it++)
                    mDistanceObstacle += round(*it);
                mDistanceObstacle = round((double)(mDistanceObstacle/mMesureOk.size()));

            }

            if (Us_TestSeuil())
            {
                 mDistanceObstacle = (double)(diff.count()*ConvertPas);
                SendEvent(us_Event::US_Obstacle);
            }



            it = mDernierreMesure.begin();
            mDernierreMesure.insert(it,newMesure);
            if (mDernierreMesure.size() > 3)
                mDernierreMesure.pop_back();
            mCptErreur = 0;
        }
        else
        {
            Us_Stop();
            Us_Start();
            mCptErreur++;
            if (mCptErreur >= 4)
                SendEvent(us_Event::US_Error);

        }
    }
    else
    {
        Us_Stop();
        Us_Start();
        mCptErreur++;
        if (mCptErreur >= 4)
            SendEvent(us_Event::US_Error);
    }
}

bool PUltrasonicSensor::Us_TestSeuil()
{
    if (mUS_Seuil != us_Seuil::NoObstacle && mDistanceObstacle > 120)
    {
        mUS_Seuil = us_Seuil::NoObstacle;
        return 1;
    }
    else if (mDistanceObstacle < 100 && mUS_Seuil != us_Seuil::Seuil1m && mDistanceObstacle > 70)
    {
        mUS_Seuil = us_Seuil::Seuil1m;
        return 1;
    }
    else if (mDistanceObstacle < 50 && mUS_Seuil != us_Seuil::Seuil50cm && mDistanceObstacle > 40)
    {
        mUS_Seuil = us_Seuil::Seuil50cm;
        return 1;
    }
    else if (mDistanceObstacle < 25 && mUS_Seuil != us_Seuil::Seuil25cm && mDistanceObstacle > 15)
    {
        mUS_Seuil = us_Seuil::Seuil25cm;
        return 1;
    }
    else if (mDistanceObstacle < 10 && mUS_Seuil != us_Seuil::Seuil10cm)
    {
        mUS_Seuil = us_Seuil::Seuil10cm;
        return 1;
    }
    else
        return 0;
}
