/** \file PGpioManager.hpp
 *  \brief Déclaration de l'énumeration Pin et de la classe PGpioManager.
 */

#ifndef PGPIOMANAGER_HPP
#define PGPIOMANAGER_HPP

#include <map>
#include <memory>
#include <fstream>
#include <string>
#include <mutex>

#include "PSingleTon.hpp"

/** \enum Pin
*   \brief Enumeration qui permet l'abstraction des broches du GPIO.
*   Le bit de poids fort de la valeur associée à chaque énumération sert pour différencier les broches en entrée de celles en sorties.
*   (0 -> entrée, 1 -> sortie)
*/
enum class Pin : uint8_t {
    //LED
    LED1        = 138, ///<Led1

    //Bouton
    SW1         = 7,   ///<Bouton1
    SW2         = 8,   ///<Bouton2

    //Capteur Ultrason
    TRIG_AV     = 129, ///<Trigger capteur ultrason avant
    ECHO_AV     = 0,   ///<Echo capteur ultrason avant
    TRIG_AR     = 133, ///<Trigger capteur ultrason arrière
    ECHO_AR     = 6,   ///<Echo capteur ultrason arrière
    CMD_ALIM_US = 136, ///<Autorise l'alimentation des capteurs ultrason

    //Capteur Laser
    XSHUT_G     = 128, ///<Activation du capteur laser gauche
    GPIO1_G     = 5,   ///<Intérruption du capteur laser gauche
    XSHUT_D     = 130, ///<Activation du capteur laser droit
    GPIO1_D     = 1,   ///<Intérruption du capteur laser droit
    XSHUT_S    = 137, ///<Activation du capteur laser avant
    GPIO1_S     = 9,   ///<Intérruption du capteur laser avant

    //Partie opérative
    GPIO_OP1    = 3,   ///<Partie opérative 1
    GPIO_OP2    = 200, ///<Partie opérative 2
    GPIO_OP3    = 4,   ///<Partie opérative 3
    GPIO_OP4    = 132, ///<Partie opérative 4
    GPIO_OP5    = 135, ///<Partie opérative 5
    GPIO_OP6    = 139, ///<Partie opérative 6
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
        /** \brief Déclare une broche au manager.
        *   \param id Label de la broche à ajouter.
        */
        void declarePin(const Pin id);

        void deletePin(const Pin id);
        std::string fromPinToString(const Pin id);

    private:

        std::map<std::string, uint8_t> mPinMapFromFile;
        std::map<Pin, uint8_t> mPinMap;
        std::fstream mFile;

        std::mutex mMutex;

};

#endif // PGPIOMANAGER_HPP
