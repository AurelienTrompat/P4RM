#ifndef PGPIOMANAGER_HPP
#define PGPIOMANAGER_HPP

#include <map>
#include <memory>
#include <fstream>

enum class Pin : uint8_t {LED1 = 128, SW1 = 0};

class PGpioManager
{
    public:
        PGpioManager();
        ~PGpioManager();

        void addPin(Pin id, const uint8_t pin);
        void removePin(Pin id);
        bool read(Pin id);

    private:

        std::map<Pin, uint8_t> mPinMap;
        std::fstream mFile;
};

#endif // PGPIOMANAGER_HPP
