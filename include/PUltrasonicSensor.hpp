#ifndef PULTRASONICSENSOR_HPP
#define PULTRASONICSENSOR_HPP

#include <atomic>
#include <vector>
#include <cmath>

#include "PCommand.hpp"
#include "PAgent.hpp"
#include "PGpioManager.hpp"

#define ConvertCm 17241.379310345
#define ConvertPas 457341.79049395

typedef PCommand::US_Parameters::US_Command us_Command;
typedef PEvent::US_Parameters::US_Event us_Event;
typedef PEvent::US_Parameters::US_Device us_Device;
typedef PEvent::US_Parameters::US_Seuil us_Seuil;
typedef std::chrono::high_resolution_clock Chrono;

class PUltrasonicSensor : public PAgent
{
    public:
        PUltrasonicSensor();
        ~PUltrasonicSensor();

    private:
        void preRun();
        void run();
        void postRun();
        void handleCommand(const PCommand &command);
        void SendEvent(PEvent::US_Parameters::US_Event);

        void Us_Start();
        void Us_Stop();
        void Us_OneMesure();
        bool Us_TestSeuil();

    private:
        PGpioManager &mPm;
        us_Command mUS_Command;
        us_Event mUS_Event;
        us_Device mUS_Device;
        us_Seuil mUS_Seuil;
        Pin mEcho;
        Pin mTrig;

        bool mNewCommand;
        bool mActive;
        std::atomic_int mSeuilUtilisateur;
        std::atomic_int mDistanceObstacle;
        std::chrono::high_resolution_clock::time_point mChronoMesure;
        std::vector<double> mMesureOk;
        std::vector<double> mDernierreMesure;
        uint8_t mCptErreur;

};


#endif // PULTRASONICSENSOR_HPP
