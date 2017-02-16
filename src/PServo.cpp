#include "PServo.hpp"

using namespace std;

PServo::PServo()
{
    mFile.open("/sys/class/pwm/pwmchip0/export",ios::out);
    mFile<<0;
    mFile.close();
    pwm_init();
}

PServo::~PServo()
{
    pwm_stop();
}

void PServo::pwm_init()
{
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/period",ios::out);
    mFile<<21000000;  //periode en nanoseconde
    mFile.close();
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle",ios::out);
    mFile<<1000000;
    mFile.close();    //temps haut en nanoseconde
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/enable",ios::out);
    mFile<<1;
    mFile.close();
}

void PServo::pwm_setangle(float newangle)
{
    int frequence;
    int futurangle;
    if (newangle <= 160 && newangle >= 20) angleact = newangle;
    else if (newangle > 160) angleact = 160;
    else angleact = 10;
    futurangle = 750000 + (angleact * 1500000) / 180; //conversion des degrés en nanosecondes
    frequence = 20000000 + futurangle;
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/period",ios::out);
    mFile<<frequence;  //periode en nanoseconde
    mFile.close();
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle",ios::out);
    mFile<<futurangle;
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
    pwm_init();
    this_thread::sleep_for(chrono::milliseconds(100));
    mFile.open("/sys/class/pwm/pwmchip0/pwm0/enable",ios::out);
    mFile<<0;
    mFile.close();
}
