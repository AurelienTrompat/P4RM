#include "PModule9DOF.hpp"
#include "PI2C.hpp"

using namespace std;

PModule9DOF::PModule9DOF(PI2C *ParentI2C) : mParentI2C(ParentI2C)
{
    mChronoCheckGyroAngle = Chrono::now();
}

PModule9DOF::~PModule9DOF()
{
    //dtor
}

void PModule9DOF::Gyro_Init()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x39,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x22;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x21,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x80;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x23,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x20;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x2E,I2C_SMBUS_BYTE_DATA, &data);
}

void PModule9DOF::Gyro_Start()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x0C;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);
}

void PModule9DOF::Gyro_Stop()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);
}
void PModule9DOF::Gyro_Shutdown()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);
}
void PModule9DOF::Gyro_CheckAngle()
{
    union i2c_smbus_data data;
    chrono::duration<double> diff = Chrono::now() - mChronoCheckGyroAngle;
    if ((diff.count() > 0.20))
    {
        mChronoCheckGyroAngle = Chrono::now();
        mParentI2C->mI2C_Device = i2c_Device::Gyro;
        mParentI2C->SetAdresse(0x6B);

        int lectureGyro = mParentI2C->BusAccess(I2C_SMBUS_READ,0x2C,I2C_SMBUS_BYTE_DATA, &data);
        if (lectureGyro != -1)
        {
            int16_t vitesseAngulaire = data.byte;
            lectureGyro = mParentI2C->BusAccess(I2C_SMBUS_READ,0x2D,I2C_SMBUS_BYTE_DATA, &data);
            if (lectureGyro != -1)
            {
                vitesseAngulaire += data.byte<<8;
                double AngleRelatif=vitesseAngulaire-60;
                AngleRelatif =AngleRelatif*0.00875;
                positionAngulaire += (AngleRelatif*diff.count());
                mParentI2C->SendEvent(i2c_Event::I2C_ZAxisAngularData, positionAngulaire);
            }
        }
    }
}

void PModule9DOF::Gyro_RAZDefault()
{
    union i2c_smbus_data data;

    Gyro_Stop();
    data.byte = 0x05;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x39,I2C_SMBUS_BYTE_DATA, &data);
    this_thread::sleep_for(chrono::milliseconds(1));
    Gyro_Start();
}

