#ifndef PGPIO_HPP
#define PGPIO_HPP

#include <cstdint>
#include <fstream>
#include <unistd.h>

class PGpio
{
    public:

        /** \brief Constructeur
         *
         * \param pin const uint8_t Numéro de la broche du gpio
         * \param direction bool direction de la broche (0-> in, 1 -> out)
         *
         */
        PGpio(const uint8_t pin, const bool direction);

        /** \brief Destructeur
         *
         */
        ~PGpio();

        /** \brief Lit la valeur de la broche
         *
         * \return bool valeur de la broche (0 -> 0V, 1 -> 3.3V)
         *
         */
        bool read();

        /** \brief Positionne la valeur de la broche
         *
         * \param value const bool valeur de la broche (0 -> 0V, 1 -> 3.3V)
         * \return void
         *
         */
        void write(const bool value);

    private:
        uint8_t mPin;
        bool mDirection;
        std::fstream mFile;
};

#endif // PGPIO_HPP
