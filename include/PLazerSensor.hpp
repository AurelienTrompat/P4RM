#ifndef PLAZERSENSOR_HPP
#define PLAZERSENSOR_HPP

#include <iostream>
#include <unistd.h>

class PI2C;

class PLazerSensor
{
    public:
        PLazerSensor(PI2C *ParentI2C);
        ~PLazerSensor();

        enum class AddressLaser
        {
            Avant = 0x11,
            Gauche = 0x21,
            Droit = 0x31,
        };

        bool Init();
        bool SetAddress(AddressLaser newAdress);
        void Measure();

    private:
        PI2C *mParentI2C;
        AddressLaser mAddress;


};

#endif // PLAZERSENSOR_HPP
