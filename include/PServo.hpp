#ifndef PSERVO_HPP
#define PSERVO_HPP

#include <fstream>
#include <thread>
#include <chrono>

class PServo
{
    public:
        PServo();
        ~PServo();

        void    pwm_init();
        void    pwm_start();
        void    pwm_setangle(float newangle);
        void    pwm_setpas(float newpas);
        void    pwm_nextpas();
        void    pwm_prevpas();
        void    pwm_stop();

    private:
        float   pas; //en degré
        float   angleact; //en degré

        std::fstream mFile;
};

#endif // PSERVO_HPP
