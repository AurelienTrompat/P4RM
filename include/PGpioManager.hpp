/** \file PGpioManager.hpp
 *  \brief Déclaration de l'énumeration Pin et de la classe PGpioManager.
 */

#ifndef PGPIOMANAGER_HPP
#define PGPIOMANAGER_HPP

#include <map>
#include <memory>
#include <fstream>
#include <string>

#include "PSingleTon.hpp"

/** \enum Pin
*   \brief Enumeration qui permet l'abstraction des broches du GPIO.
*   Le bit de poids fort de la valeur associée à chaque énumération sert pour différencier les broches en entrée de celles en sorties.
*   (0 -> entrée, 1 -> sortie)
*/
enum class Pin : uint8_t {
    LED1 = 128, ///<Led1
    SW1 = 0 ///<Bouton1
    }; // le bit de poids fort à 1 indique que la broche est une sortie

/** \class PGpioManager PGpioManager.hpp PGpioManager.hpp Gère les broches du GPIO.
*/
class PGpioManager : public PSingleTon<PGpioManager> //Pour assurer l'instance unique du manager
{
    friend class PSingleTon<PGpioManager>;

    private:
        PGpioManager();
        PGpioManager(const PGpioManager&){}
        ~PGpioManager();
    public:
        /** \brief Déclare une broche au manager.
        *   \param id Label de la broche à ajouter.
        */
        void declarePin(const Pin id);


        /** \brief Lit une broche.
        *   \param id Label de la broche à lire.
        *   \return Valeur de la broche.
        */
        bool read(const Pin id);

        /** \brief Modifie la valeur d'une broche.
        *   \param id Label de la broche à modifier.
        *   \param value Valeur de la broche
        */
        void write(const Pin id, const bool value);

    private:
        void deletePin(const Pin id);
        std::string fromPinToString(const Pin id);

    private:

        std::map<std::string, uint8_t> mPinMapFromFile;
        std::map<Pin, uint8_t> mPinMap;
        std::fstream mFile;

};

#endif // PGPIOMANAGER_HPP
