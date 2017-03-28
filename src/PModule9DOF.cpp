#include "PModule9DOF.hpp"
#include "PI2C.hpp"

using namespace std;

PModule9DOF::PModule9DOF(PI2C *ParentI2C) : mParentI2C(ParentI2C), mGyroEnable(false), mAxelEnable(false), mMagnEnable(false), mRobotImmobile(false)
{
    mChronoCheckGyroAngle = Chrono::now();
    mChronoCheckMagnAngle = Chrono::now();
    mChronoCheckAxelZ = Chrono::now();
}

PModule9DOF::~PModule9DOF()
{
    //dtor
}

void PModule9DOF::PModule9DOF_Init()
{
    Gyro_Init();
    Magn_Init();
    Axel_Init();
}

void PModule9DOF::PModule9DOF_Shutdown()
{
    Gyro_Shutdown();
    Magn_Shutdown();
    Axel_Shutdown();
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

    mGyroEnable = true;
}

void PModule9DOF::Gyro_Stop()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    mGyroEnable = false;
}
void PModule9DOF::Gyro_Shutdown()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Gyro;
    mParentI2C->SetAdresse(0x6B);

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    mGyroEnable = false;
}
void PModule9DOF::Gyro_CheckAngle()
{
    union i2c_smbus_data data;
    chrono::duration<double> diff = Chrono::now() - mChronoCheckGyroAngle;
    if ((diff.count() > 0.20) && mGyroEnable)
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
                mPositionAngulaire += (AngleRelatif*diff.count());
                if (mPositionAngulaire < 0)
                    mPositionAngulaire += 360;
                else if (mPositionAngulaire > 360)
                    mPositionAngulaire -= 360;
                mParentI2C->SendEvent(i2c_Event::I2C_ZAxisAngularData, mPositionAngulaire);
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

void PModule9DOF::Magn_Init()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Magn;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x26,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x88;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x16,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0xFA;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x17,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0xD6;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x18,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0xF7;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x19,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x40;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x1F,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x78;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x24,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x20;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x25,I2C_SMBUS_BYTE_DATA, &data);
}

void PModule9DOF::Magn_Start()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Magn;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x60;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x24,I2C_SMBUS_BYTE_DATA, &data);

    mMagnEnable = true;
}

void PModule9DOF::Magn_Stop()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Magn;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x78;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x24,I2C_SMBUS_BYTE_DATA, &data);

    mMagnEnable = false;
}

void PModule9DOF::Magn_Shutdown()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Magn;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x78;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x24,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x02;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x26,I2C_SMBUS_BYTE_DATA, &data);

    mMagnEnable = false;
}

void PModule9DOF::Magn_CheckOrientation()
{
    union i2c_smbus_data data;
    chrono::duration<double> diff = Chrono::now() - mChronoCheckMagnAngle;

    if ((diff.count() > 0.30) && mMagnEnable)
    {
        mChronoCheckMagnAngle = Chrono::now();
        mParentI2C->mI2C_Device = i2c_Device::Magn;
        mParentI2C->SetAdresse(0x1D);

        int lectureMagn = mParentI2C->BusAccess(I2C_SMBUS_READ,0x08,I2C_SMBUS_BYTE_DATA, &data);
        if (lectureMagn != -1)
        {
            int16_t axeX = data.byte;
            lectureMagn = mParentI2C->BusAccess(I2C_SMBUS_READ,0x09,I2C_SMBUS_BYTE_DATA, &data);
            if (lectureMagn != -1)
            {
                axeX += data.byte<<8;

                lectureMagn = mParentI2C->BusAccess(I2C_SMBUS_READ,0x0A,I2C_SMBUS_BYTE_DATA, &data);
                if (lectureMagn != -1)
                {
                    int16_t axeY = data.byte;
                    lectureMagn = mParentI2C->BusAccess(I2C_SMBUS_READ,0x0B,I2C_SMBUS_BYTE_DATA, &data);
                    if (lectureMagn != -1)
                    {
                        axeY += data.byte<<8;

                        double angleNord = 180 * atan2(axeY,axeX)/M_PI;
                        if(angleNord < 0)
                            angleNord += 360;
                        if (mRobotImmobile == 1 && ((mPositionAngulaire > (angleNord+15)) || (mPositionAngulaire < (angleNord-15))))
                        {
                            mPositionAngulaire = angleNord;
                            mParentI2C->SendEvent(i2c_Event::I2C_ZAxisAngularData, mPositionAngulaire);
                        }
                    }
                }
            }
        }
    }
}

void PModule9DOF::Axel_Init()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Axel;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x40;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x1F,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x58;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x88;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x21,I2C_SMBUS_BYTE_DATA, &data);
}

void PModule9DOF::Axel_Start()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Axel;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x5C;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    mAxelEnable = true;
}

void PModule9DOF::Axel_Stop()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Axel;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x58;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    mAxelEnable = false;
}

void PModule9DOF::Axel_Shutdown()
{
    union i2c_smbus_data data;

    mParentI2C->mI2C_Device = i2c_Device::Axel;
    mParentI2C->SetAdresse(0x1D);

    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x20,I2C_SMBUS_BYTE_DATA, &data);

    mAxelEnable = false;
}

void PModule9DOF::Axel_CheckAxeleration()
{
    union i2c_smbus_data data;
    chrono::duration<double> diff = Chrono::now() - mChronoCheckAxelZ;
    if ((diff.count() > 0.20) && mAxelEnable)
    {
        mChronoCheckAxelZ = Chrono::now();
        mParentI2C->mI2C_Device = i2c_Device::Axel;
        mParentI2C->SetAdresse(0x1D);

        int lectureAxel = mParentI2C->BusAccess(I2C_SMBUS_READ,0x2C,I2C_SMBUS_BYTE_DATA, &data);
        if (lectureAxel != -1)
        {
            int16_t AxelZ = data.byte;
            lectureAxel = mParentI2C->BusAccess(I2C_SMBUS_READ,0x2D,I2C_SMBUS_BYTE_DATA, &data);
            if (lectureAxel != -1)
            {
                AxelZ += data.byte<<8;
                /*double AxelZConvert = (double)(AxelZ*0.000122);
                if (AxelZConvert > 1.1)
                    mParentI2C->SendEvent(i2c_Event::I2C_ErrorRobotLift);
                cout << "Acceleration sur Z : " << AxelZConvert << endl;*/
            }
        }
    }
}

void PModule9DOF::SetRobotImmobile(uint8_t robotImmobile)
{
    mRobotImmobile = robotImmobile;

}
