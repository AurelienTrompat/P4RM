#ifndef PLAZERSENSOR_HPP
#define PLAZERSENSOR_HPP

#include <iostream>
#include <unistd.h>
#include <chrono>

typedef std::chrono::high_resolution_clock Chrono;

class PI2C;

class PLazerSensor
{
public:
    PLazerSensor(PI2C *ParentI2C);
    ~PLazerSensor();

    enum class AddressLaser : uint8_t
    {
        Avant = 0x11,
        Gauche = 0x21,
        Droit = 0x31,
    };

    enum vcselPeriodType
    {
         VcselPeriodPreRange,
         VcselPeriodFinalRange
    };

    bool Init();
    bool SetAddress(AddressLaser newAdress);
    inline void setTimeout(uint16_t timeout) { io_timeout = timeout; }
     bool setSignalRateLimit(float limit_Mcps);
     bool setVcselPulsePeriod(vcselPeriodType type, uint8_t period_pclks);
     bool setMeasurementTimingBudget(uint32_t budget_us);
    void Measure();


private:
    PI2C *mParentI2C;
    AddressLaser mAddress;
    uint8_t mStopVariable;
    uint32_t measurement_timing_budget_us;
    Chrono::time_point mTimeout_start_ms;
    bool did_timeout;
    double io_timeout;
    struct SequenceStepEnables
    {
        bool tcc, msrc, dss, pre_range, final_range;
    };

    struct SequenceStepTimeouts
    {
        uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

        uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
        uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
    };


private:
    bool getSpadInfo(uint8_t * count, bool * type_is_aperture);
    uint32_t getMeasurementTimingBudget();
    void getSequenceStepEnables(SequenceStepEnables * enables);
    void getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts);
    uint8_t getVcselPulsePeriod(vcselPeriodType type);
    uint32_t timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
    uint16_t decodeTimeout(uint16_t reg_val);
    bool performSingleRefCalibration(uint8_t vhv_init_byte);
    uint32_t timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
    uint16_t encodeTimeout(uint16_t timeout_mclks);
    uint16_t readRangeContinuousMillimeters();
    uint16_t readRangeSingleMillimeters();
    bool timeoutOccurred();
};

#endif // PLAZERSENSOR_HPP
