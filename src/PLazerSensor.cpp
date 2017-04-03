#include "PLazerSensor.hpp"
#include "PI2C.hpp"

// Record the current time to check an upcoming timeout against
#define startTimeout() (mTimeout_start_ms = Chrono::now())

// Check if timeout is enabled (set to nonzero value) and has expired
#define checkTimeoutExpired() (io_timeout > 0 && (((std::chrono::duration<double>)(Chrono::now() - mTimeout_start_ms)).count() > io_timeout))

// Decode VCSEL (vertical cavity surface emitting laser) pulse period in PCLKs
// from register value
// based on VL53L0X_decode_vcsel_period()
#define decodeVcselPeriod(reg_val) (((reg_val) + 1) << 1)

// Calculate macro period in *nanoseconds* from VCSEL period in PCLKs
// based on VL53L0X_calc_macro_period_ps()
// PLL_period_ps = 1655; macro_period_vclks = 2304
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)

// Encode VCSEL pulse period register value from period in PCLKs
// based on VL53L0X_encode_vcsel_period()
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)

PLazerSensor::PLazerSensor(PI2C *ParentI2C) : mParentI2C(ParentI2C), did_timeout(false), io_timeout(0)
{
    //ctor
}

PLazerSensor::~PLazerSensor()
{
    //dtor
}

bool PLazerSensor::Init()
{
    union i2c_smbus_data data;

    //mParentI2C->SetAdresse((uint8_t) mAddress);

// VL53L0X_DataInit() begin

    mParentI2C->BusAccess(I2C_SMBUS_READ,0x89,I2C_SMBUS_BYTE_DATA, &data);
    data.byte = data.byte | 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x89, I2C_SMBUS_BYTE_DATA, &data);

    // I2C standard mode
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x88, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x91, I2C_SMBUS_BYTE_DATA, &data);
    mStopVariable = data.byte;

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);

    // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
    mParentI2C->BusAccess(I2C_SMBUS_READ,0x60,I2C_SMBUS_BYTE_DATA, &data);
    data.byte = data.byte | 0x12;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x60, I2C_SMBUS_BYTE_DATA, &data);

    // set final range signal rate limit to 0.25 MCPS (million counts per second)
    setSignalRateLimit(0.25);

    data.byte = 0xFF;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

// VL53L0X_DataInit() end

// VL53L0X_StaticInit() begin

    uint8_t spad_count;
    bool spad_type_is_aperture;
    cout << "getspad"<<endl;
    if (!getSpadInfo(&spad_count, &spad_type_is_aperture))
        return false;

    // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
    // the API, but the same data seems to be more easily readable from
    // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
    cout << "0"<<endl;
    uint8_t ref_spad_map[7];
    cout << "1"<<endl;
    data.block[0] = 7;
    cout << "2"<<endl;
    mParentI2C->BusAccess(I2C_SMBUS_READ, 0xB0, I2C_SMBUS_BLOCK_DATA, &data);
    cout << "3"<<endl;
    for (int i=0; i<6; i++)
    {
        ref_spad_map[i] = data.block[i+1];
        cout <<+i<<endl;
    }
    // -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4F, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x2C;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4E, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xB4;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xB6, I2C_SMBUS_BYTE_DATA, &data);

    uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
    uint8_t spads_enabled = 0;

    for (uint8_t i = 0; i < 48; i++)
    {
        if (i < first_spad_to_enable || spads_enabled == spad_count)
        {
            // This bit is lower than the first one that should be enabled, or
            // (reference_spad_count) bits have already been enabled, so zero this bit
            ref_spad_map[i / 8] &= ~(1 << (i % 8));
        }
        else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1)
        {
            spads_enabled++;
        }
    }

    data.block[0] = 7;
    for (int i=0; i<6; i++)
    {
        data.block[i+1] = ref_spad_map[i];
    }
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xB0, I2C_SMBUS_BLOCK_DATA, &data);
    cout << "OK" << endl;
    // -- VL53L0X_set_reference_spads() end

    // -- VL53L0X_load_tuning_settings() begin
    // DefaultTuningSettings from vl53l0x_tuning.h
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x09, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x10, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x11, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x24, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xFF;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x25, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x75, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x2C;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4E, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x20;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x09;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x54, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x31, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x03;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x32, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x83;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x40, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x25;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x46, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x60, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x27, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x06;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x50, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x51, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x96;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x52, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x56, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x30;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x57, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x61, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x62, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x64, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x65, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xA0;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x66, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x32;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x22, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x14;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xFF;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x49, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4A, I2C_SMBUS_BYTE_DATA, &data);


    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x0A;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x7A, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x7B, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x21;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x78, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x34;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x23, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x42, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xFF;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x44, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x26;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x45, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x05;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x46, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x40;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x40, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x06;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0E, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x1A;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x20, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x40;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x43, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x03;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x34, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x44;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x35, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x31, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x09;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4B, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x05;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4C, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x4D, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x44, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x20;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x45, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x08;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x28;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x67, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x70, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x71, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xFE;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x72, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x76, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x77, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0D, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0xF8;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);


    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x8E, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);

    // -- VL53L0X_load_tuning_settings() end

// "Set interrupt config to new sample ready"
    // -- VL53L0X_SetGpioConfig() begin

    data.byte = 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0A, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x84, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = data.byte & ~0x10;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x84, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0B, I2C_SMBUS_BYTE_DATA, &data);

    // -- VL53L0X_SetGpioConfig() end

    measurement_timing_budget_us = getMeasurementTimingBudget();

    // "Disable MSRC and TCC by default"
    // MSRC = Minimum Signal Rate Check
    // TCC = Target CentreCheck
    // -- VL53L0X_SetSequenceStepEnable() begin

    data.byte = 0xE8;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    // -- VL53L0X_SetSequenceStepEnable() end

    // "Recalculate timing budget"
    setMeasurementTimingBudget(measurement_timing_budget_us);

    // VL53L0X_StaticInit() end

    // VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

    // -- VL53L0X_perform_vhv_calibration() begin
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    if (!performSingleRefCalibration(0x40))
    {
        return false;
    }

    // -- VL53L0X_perform_vhv_calibration() end

    // -- VL53L0X_perform_phase_calibration() begin
    data.byte = 0x02;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    if (!performSingleRefCalibration(0x00))
    {
        return false;
    }

    // -- VL53L0X_perform_phase_calibration() end

    // "restore the previous Sequence Config"
    data.byte = 0xE8;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    // VL53L0X_PerformRefCalibration() end

    return true;
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

bool PLazerSensor::setSignalRateLimit(float limit_Mcps)
{
    union i2c_smbus_data data;

    if (limit_Mcps < 0 || limit_Mcps > 511.99)
        return false;

    // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
    data.word = limit_Mcps * (1 << 7);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE,0x44,I2C_SMBUS_WORD_DATA, &data);
    return true;
}

bool PLazerSensor::getSpadInfo(uint8_t * count, bool * type_is_aperture)
{
    uint8_t tmp;
    union i2c_smbus_data data;

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x06;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);

    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x83, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = data.byte  | 0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x83, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x07;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x81, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x6b;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x94, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x83, I2C_SMBUS_BYTE_DATA, &data);

    startTimeout();
    while(data.byte == 0x00)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ, 0x83, I2C_SMBUS_BYTE_DATA, &data);

        if (checkTimeoutExpired())
            return false;
    }
    cout << "finwhile" << endl;
    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x83, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x92, I2C_SMBUS_BYTE_DATA, &data);
    tmp = data.byte;

    *count = tmp & 0x7f;
    *type_is_aperture = (tmp >> 7) & 0x01;

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x81, I2C_SMBUS_BYTE_DATA, &data);
    data.byte = 0x06;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);

    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x83, I2C_SMBUS_BYTE_DATA, &data); // attention peut etre faux
    data.byte = data.byte & ~0x04;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x83, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);
    cout << "true" << endl;
    return true;
}

uint32_t PLazerSensor::getMeasurementTimingBudget()
{
    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;

    uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
    uint16_t const EndOverhead        = 960;
    uint16_t const MsrcOverhead       = 660;
    uint16_t const TccOverhead        = 590;
    uint16_t const DssOverhead        = 690;
    uint16_t const PreRangeOverhead   = 660;
    uint16_t const FinalRangeOverhead = 550;

    // "Start and end overhead times always present"
    uint32_t budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc)
    {
        budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    }

    if (enables.dss)
    {
        budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    }
    else if (enables.msrc)
    {
        budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    }

    if (enables.pre_range)
    {
        budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    }

    if (enables.final_range)
    {
        budget_us += (timeouts.final_range_us + FinalRangeOverhead);
    }

    measurement_timing_budget_us = budget_us; // store for internal reuse
    return budget_us;
}

// Get sequence step enables
// based on VL53L0X_GetSequenceStepEnables()
void PLazerSensor::getSequenceStepEnables(SequenceStepEnables * enables)
{
    union i2c_smbus_data data;
    mParentI2C->BusAccess(I2C_SMBUS_READ,0x01,I2C_SMBUS_BYTE_DATA, &data);
    uint8_t sequence_config = data.byte;

    enables->tcc          = (sequence_config >> 4) & 0x1;
    enables->dss          = (sequence_config >> 3) & 0x1;
    enables->msrc         = (sequence_config >> 2) & 0x1;
    enables->pre_range    = (sequence_config >> 6) & 0x1;
    enables->final_range  = (sequence_config >> 7) & 0x1;
}

// Get sequence step timeouts
// based on get_sequence_step_timeout(),
// but gets all timeouts instead of just the requested one, and also stores
// intermediate values
void PLazerSensor::getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts)
{
    union i2c_smbus_data data;

    timeouts->pre_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodPreRange);

    mParentI2C->BusAccess(I2C_SMBUS_READ,0x46,I2C_SMBUS_BYTE_DATA, &data);
    timeouts->msrc_dss_tcc_mclks = data.byte + 1;
    timeouts->msrc_dss_tcc_us =
        timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    mParentI2C->BusAccess(I2C_SMBUS_READ,0x51,I2C_SMBUS_WORD_DATA, &data);
    timeouts->pre_range_mclks =
        decodeTimeout(data.word);

    timeouts->pre_range_us =
        timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                                   timeouts->pre_range_vcsel_period_pclks);

    timeouts->final_range_vcsel_period_pclks = getVcselPulsePeriod(VcselPeriodFinalRange);

    mParentI2C->BusAccess(I2C_SMBUS_READ,0x71,I2C_SMBUS_WORD_DATA, &data);
    timeouts->final_range_mclks =
        decodeTimeout(data.word);

    if (enables->pre_range)
    {
        timeouts->final_range_mclks -= timeouts->pre_range_mclks;
    }

    timeouts->final_range_us =
        timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                                   timeouts->final_range_vcsel_period_pclks);
}

// Get the VCSEL pulse period in PCLKs for the given period type.
// based on VL53L0X_get_vcsel_pulse_period()
uint8_t PLazerSensor::getVcselPulsePeriod(vcselPeriodType type)
{
    union i2c_smbus_data data;

    if (type == VcselPeriodPreRange)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ,0x50,I2C_SMBUS_BYTE_DATA, &data);
        return decodeVcselPeriod(data.byte);
    }
    else if (type == VcselPeriodFinalRange)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ,0x70,I2C_SMBUS_BYTE_DATA, &data);
        return decodeVcselPeriod(data.byte);
    }
    else
    {
        return 255;
    }
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
uint32_t PLazerSensor::timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

    return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
uint16_t PLazerSensor::decodeTimeout(uint16_t reg_val)
{
    // format: "(LSByte * 2^MSByte) + 1"
    return (uint16_t)((reg_val & 0x00FF) <<
                      (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

// Set the measurement timing budget in microseconds, which is the time allowed
// for one measurement; the ST API and this library take care of splitting the
// timing budget among the sub-steps in the ranging sequence. A longer timing
// budget allows for more accurate measurements. Increasing the budget by a
// factor of N decreases the range measurement standard deviation by a factor of
// sqrt(N). Defaults to about 33 milliseconds; the minimum is 20 ms.
// based on VL53L0X_set_measurement_timing_budget_micro_seconds()
bool PLazerSensor::setMeasurementTimingBudget(uint32_t budget_us)
{
    union i2c_smbus_data data;
    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;

    uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
    uint16_t const EndOverhead        = 960;
    uint16_t const MsrcOverhead       = 660;
    uint16_t const TccOverhead        = 590;
    uint16_t const DssOverhead        = 690;
    uint16_t const PreRangeOverhead   = 660;
    uint16_t const FinalRangeOverhead = 550;

    uint32_t const MinTimingBudget = 20000;

    if (budget_us < MinTimingBudget)
    {
        return false;
    }

    uint32_t used_budget_us = StartOverhead + EndOverhead;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc)
    {
        used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    }

    if (enables.dss)
    {
        used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    }
    else if (enables.msrc)
    {
        used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    }

    if (enables.pre_range)
    {
        used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    }

    if (enables.final_range)
    {
        used_budget_us += FinalRangeOverhead;

        // "Note that the final range timeout is determined by the timing
        // budget and the sum of all other timeouts within the sequence.
        // If there is no room for the final range timeout, then an error
        // will be set. Otherwise the remaining time will be applied to
        // the final range."

        if (used_budget_us > budget_us)
        {
            // "Requested timeout too big."
            return false;
        }

        uint32_t final_range_timeout_us = budget_us - used_budget_us;

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."

        uint16_t final_range_timeout_mclks =
            timeoutMicrosecondsToMclks(final_range_timeout_us,
                                       timeouts.final_range_vcsel_period_pclks);

        if (enables.pre_range)
        {
            final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        data.word = encodeTimeout(final_range_timeout_mclks);
        mParentI2C->BusAccess(I2C_SMBUS_READ, 0x71, I2C_SMBUS_WORD_DATA, &data);

        // set_sequence_step_timeout() end

        measurement_timing_budget_us = budget_us; // store for internal reuse
    }
    return true;
}

// based on VL53L0X_perform_single_ref_calibration()
bool PLazerSensor::performSingleRefCalibration(uint8_t vhv_init_byte)
{
    union i2c_smbus_data data;

    data.byte = 0x01 | vhv_init_byte;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    startTimeout();
    data.byte = 0x00;
    while ((data.byte & 0x07) == 0)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ, 0x13, I2C_SMBUS_BYTE_DATA, &data);
        if (checkTimeoutExpired())
        {
            return false;
        }
    }

    data.byte = 0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0B, I2C_SMBUS_BYTE_DATA, &data);

    data.byte = 0x00;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);

    return true;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
uint32_t PLazerSensor::timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks)
{
    uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

    return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
uint16_t PLazerSensor::encodeTimeout(uint16_t timeout_mclks)
{
    // format: "(LSByte * 2^MSByte) + 1"

    uint32_t ls_byte = 0;
    uint16_t ms_byte = 0;

    if (timeout_mclks > 0)
    {
        ls_byte = timeout_mclks - 1;

        while ((ls_byte & 0xFFFFFF00) > 0)
        {
            ls_byte >>= 1;
            ms_byte++;
        }

        return (ms_byte << 8) | (ls_byte & 0xFF);
    }
    else
    {
        return 0;
    }
}

// Set the VCSEL (vertical cavity surface emitting laser) pulse period for the
// given period type (pre-range or final range) to the given value in PCLKs.
// Longer periods seem to increase the potential range of the sensor.
// Valid values are (even numbers only):
//  pre:  12 to 18 (initialized default: 14)
//  final: 8 to 14 (initialized default: 10)
// based on VL53L0X_set_vcsel_pulse_period()
bool PLazerSensor::setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks)
{
    union i2c_smbus_data data;
    uint8_t vcsel_period_reg = encodeVcselPeriod(period_pclks);

    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;

    getSequenceStepEnables(&enables);
    getSequenceStepTimeouts(&enables, &timeouts);

    // "Apply specific settings for the requested clock period"
    // "Re-calculate and apply timeouts, in macro periods"

    // "When the VCSEL period for the pre or final range is changed,
    // the corresponding timeout must be read from the device using
    // the current VCSEL period, then the new VCSEL period can be
    // applied. The timeout then must be written back to the device
    // using the new VCSEL period.
    //
    // For the MSRC timeout, the same applies - this timeout being
    // dependant on the pre-range vcsel period."


    if (type == VcselPeriodPreRange)
    {
        // "Set phase check limits"
        switch (period_pclks)
        {
            case 12:
            {
                data.byte = 0x18;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x57, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }


            case 14:
            {
                data.byte = 0x30;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x57, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }

            case 16:
            {
                data.byte = 0x40;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x57, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }

            case 18:
            {
                data.byte = 0x50;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x57, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }

            default:
                // invalid period
                return false;
        }
        data.byte = 0x08;
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x56, I2C_SMBUS_BYTE_DATA, &data);

        // apply new VCSEL period
        data.byte = vcsel_period_reg;
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x50, I2C_SMBUS_BYTE_DATA, &data);

        // update timeouts

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_PRE_RANGE)

        uint16_t new_pre_range_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.pre_range_us, period_pclks);

        data.byte = encodeTimeout(new_pre_range_timeout_mclks);
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x51, I2C_SMBUS_WORD_DATA, &data);

        // set_sequence_step_timeout() end

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_MSRC)

        uint16_t new_msrc_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.msrc_dss_tcc_us, period_pclks);

        data.byte = (new_msrc_timeout_mclks > 256) ? 255 : (new_msrc_timeout_mclks - 1);
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x46, I2C_SMBUS_BYTE_DATA, &data);


        // set_sequence_step_timeout() end
    }
    else if (type == VcselPeriodFinalRange)
    {
        switch (period_pclks)
        {
            case 8:
            {
                data.byte = 0x10;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x08;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x02;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x32, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x0C;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x01;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x30;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x00;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }


            case 10:
                {
                data.byte = 0x28;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x08;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x03;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x32, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x09;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x01;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x20;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x00;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }
            case 12:
                {
                data.byte = 0x38;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x08;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x03;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x32, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x08;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x01;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x20;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x00;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }
            case 14:
                {
                data.byte = 0x48;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x48, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x08;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x47, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x03;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x32, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x07;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x01;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x20;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x30, I2C_SMBUS_BYTE_DATA, &data);
                data.byte = 0x00;
                mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
                break;
            }
            default:
                // invalid period
                return false;
        }

        // apply new VCSEL period
        data.byte = vcsel_period_reg;
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x70, I2C_SMBUS_BYTE_DATA, &data);

        // update timeouts

        // set_sequence_step_timeout() begin
        // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

        // "For the final range timeout, the pre-range timeout
        //  must be added. To do this both final and pre-range
        //  timeouts must be expressed in macro periods MClks
        //  because they have different vcsel periods."

        uint16_t new_final_range_timeout_mclks =
            timeoutMicrosecondsToMclks(timeouts.final_range_us, period_pclks);

        if (enables.pre_range)
        {
            new_final_range_timeout_mclks += timeouts.pre_range_mclks;
        }

        data.word = encodeTimeout(new_final_range_timeout_mclks);
        mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x71, I2C_SMBUS_WORD_DATA, &data);

        // set_sequence_step_timeout end
    }
    else
    {
        // invalid type
        return false;
    }

    // "Finally, the timing budget must be re-applied"

    setMeasurementTimingBudget(measurement_timing_budget_us);

    // "Perform the phase calibration. This is needed after changing on vcsel period."
    // VL53L0X_perform_phase_calibration() begin

    mParentI2C->BusAccess(I2C_SMBUS_READ, 0x01, I2C_SMBUS_BYTE_DATA, &data);
    uint8_t sequence_config = data.byte;
    data.byte = 0x02;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);
    performSingleRefCalibration(0x0);
    data.byte = sequence_config;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    // VL53L0X_perform_phase_calibration() end

    return true;
}

// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
uint16_t PLazerSensor::readRangeContinuousMillimeters()
{
    union i2c_smbus_data data;
    startTimeout();
    data.byte =0x00;
    while ((data.byte & 0x07) == 0)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ, 0x13, I2C_SMBUS_BYTE_DATA, &data);
        if (checkTimeoutExpired())
        {
            did_timeout = true;
            return 65535;
        }
    }

    // assumptions: Linearity Corrective Gain is 1000 (default);
    // fractional ranging is not enabled
    mParentI2C->BusAccess(I2C_SMBUS_READ, (0x14+10), I2C_SMBUS_WORD_DATA, &data);
    uint16_t range = data.word;

    data.byte =0x01;
    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x0B, I2C_SMBUS_BYTE_DATA, &data);

    return range;
}

// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
uint16_t PLazerSensor::readRangeSingleMillimeters()
{
    union i2c_smbus_data data;
     data.byte = 0x01;
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
     data.byte = 0x00;
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);
     data.byte = mStopVariable;
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x91, I2C_SMBUS_BYTE_DATA, &data);
     data.byte = 0x01;
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x00, I2C_SMBUS_BYTE_DATA, &data);
     data.byte = 0x00;
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0xFF, I2C_SMBUS_BYTE_DATA, &data);
     mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x80, I2C_SMBUS_BYTE_DATA, &data);

    mParentI2C->BusAccess(I2C_SMBUS_WRITE, 0x01, I2C_SMBUS_BYTE_DATA, &data);

    // "Wait until start bit has been cleared"
    startTimeout();
    data.byte = 0x01;
    while (data.byte & 0x01)
    {
        mParentI2C->BusAccess(I2C_SMBUS_READ, 0x00, I2C_SMBUS_BYTE_DATA, &data);
        if (checkTimeoutExpired())
        {
            did_timeout = true;
            return 65535;
        }
    }

    return readRangeContinuousMillimeters();
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool PLazerSensor::timeoutOccurred()
{
    bool tmp = did_timeout;
    did_timeout = false;
    return tmp;
}

void PLazerSensor::Measure()
{
    cout << "Laser : Distance obstacle = " << readRangeSingleMillimeters() <<" mm"<< endl;
    if (timeoutOccurred())
    {
        cout << "Laser : TIMEOUT" << endl;;
    }
}
