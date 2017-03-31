#include "PLazerSensor.hpp"
#include "PI2C.hpp"

PLazerSensor::PLazerSensor(PI2C *ParentI2C) : mParentI2C(ParentI2C)
{
    //ctor
}

PLazerSensor::~PLazerSensor()
{
    //dtor
}

bool PLazerSensor::Init()
{

    return 0;
}

bool PLazerSensor::SetAddress(AddressLaser newAdress)
{
    union i2c_smbus_data data;
    mParentI2C->SetAdresse(0x29);
    mAddress = newAdress;
    data.byte = (uint8_t) newAdress;
    if(mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x8A,I2C_SMBUS_BYTE_DATA, &data) == -1)
        return 1;
    else
        return 0;
}

void PLazerSensor::Measure()
{

}
