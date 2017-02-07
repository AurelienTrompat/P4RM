#include "PServo.hpp"

using namespace std;

PServo::PServo()
{
    mFile.open("/sys/class/pwm/pwmchip0/export",ios::out);
    mFile<<0;
    mFile.close();
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/period",ios::out);
    mFile<<20000000;
    mFile.close();
    pwm_init();
}

PServo::~PServo()
{
    pwm_stop();
}

void PServo::pwm_init()
{
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle",ios::out);
    mFile<<0;
    mFile.close();
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/enable",ios::out);
    mFile<<1;
    mFile.close();
}

void PServo::pwm_setangle(float newangle)
{
    if (newangle <= 120 && newangle >= 0) angleact = newangle;
    else if (newangle > 120) angleact = 120;
    else angleact = 0;
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle",ios::out);
    mFile<<angleact*100000;
    mFile.close();
}

void PServo::pwm_prevpas()
{
    pwm_setangle(angleact-pas);
}

void PServo::pwm_setpas(float newpas)
{
    pas = newpas;
}

void PServo::pwm_nextpas()
{
    pwm_setangle(pas+angleact);
}

void PServo::pwm_stop()
{
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle",ios::out);
    mFile<<0;
    mFile.close();
    this_thread::sleep_for(chrono::milliseconds(100));
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/enable",ios::out);
    mFile<<0;
    mFile.close();
}
