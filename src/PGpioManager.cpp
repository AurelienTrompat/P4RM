#include "PGpioManager.hpp"

#include <iostream>
#include <set>
#include <sstream>

using namespace std;

PGpioManager::PGpioManager()
{
    set<uint8_t> valeurs;


    string ligne;
    string demiLigne;
    string cleTemp;

    istringstream iss;

    mFile.open("config/pins.txt", ios::in);
    if(mFile.is_open())
    {
        while(!mFile.eof())
        {
            iss.clear();
            getline(mFile, ligne);
            if(!ligne.empty())
            {
            iss.str(ligne);

            getline(iss, demiLigne, ':');
            cleTemp = demiLigne;

            getline(iss, demiLigne, ':');
            if(!(stoi(demiLigne) >= 4 && stoi(demiLigne) <= 27))
                throw runtime_error("La valeur d'une pin dans pins.txt est incorrecte");
            else
            {
                if(!valeurs.insert(stoi(demiLigne)).second)
                    throw runtime_error("Le fichier pins.txt est mal écrit (doublons dans les valeurs de pin)");
            }
            if(!mPinMapFromFile.insert(make_pair(cleTemp, stoi(demiLigne))).second)
                throw runtime_error("Le fichier pins.txt est mal écrit (doublons dans les labels de pin)");
        }



    }
}
else
    throw runtime_error("Le fichier pins.txt n'a pas été trouvé !");
mFile.close();
}

PGpioManager::~PGpioManager()
{
    for(auto itr=mPinMap.begin(); itr!=mPinMap.end(); itr++) //parcourt toute la map pour désactiver toutes les pins
        deletePin(itr->first);
}

void PGpioManager::declarePin(const Pin id)
{
    const auto found = mPinMapFromFile.find(fromPinToString(id));
    if(found != mPinMapFromFile.end())
    {
        if(mPinMap.insert(make_pair(id, found->second)).second)
        {
            mFile.open("/sys/class/gpio/export",ios::out);
            mFile<<to_string(found->second);
            mFile.close();
            mFile.open("/sys/class/gpio/gpio"+to_string(found->second)+"/direction",ios::out);
            if((uint8_t)id>>7)
                mFile<<"out";
            else
                mFile<<"in";
            mFile.close();
        }
        else
            throw std::logic_error("La pin a déjà ete ajoutée !");
    }
    else
        throw runtime_error("Le fichier pins.txt ne définie pas cette pin !");


}

void PGpioManager::deletePin(const Pin id)
{
    auto found = mPinMap.find(id); //on cherche la pin pour verifier qu'elle a été exportée
    if(found != mPinMap.end())
    {
        mFile.open("/sys/class/gpio/unexport",ios::out);
        mFile<<to_string(found->second);
        mFile.close();
        mPinMap.erase(found);
    }
    else
        throw std::logic_error("Cette pin n'a pas été ajoutée donc ne peut pas être suprimée !");

}
bool PGpioManager::read(const Pin id)
{
    bool value = 0;
    auto found = mPinMap.find(id);
    if(found != mPinMap.end())
    {
        mFile.open("/sys/class/gpio/gpio" + to_string(found->second) +"/value",ios::in);
        mFile>>value;
        mFile.close();
    }
    else
        throw std::logic_error("Cette pin n'a pas été ajoutée donc ne peut pas être lue !");
    return value;
}
void PGpioManager::write(const Pin id, const bool value)
{
    auto found = mPinMap.find(id);
    if(found != mPinMap.end())
    {
        if((uint8_t)id>>7) //test si le bit de poids for est egal à 1 donc, que la pin est en sortie
        {
            mFile.open("/sys/class/gpio/gpio"+to_string(found->second)+"/value",ios::out);
            mFile<<to_string(value);
            mFile.close();
        }
        else
            throw std::logic_error("Cette pin est en entrée donc sa valeur ne peut pas être changée !");
    }
    else
        throw std::logic_error("Cette pin n'a pas été ajoutée donc sa valeur ne peut pas être changée !");

}

string PGpioManager::fromPinToString(Pin id)
{
    switch (id)
    {
    case Pin::LED1:
        return "LED1";
    case Pin::SW1:
        return "SW1";
    case Pin::ECHO_AV:
        return "ECHO_AV";
    case Pin::TRIGGER_AV:
        return "TRIGGER_AV";
    }
    return "";
}
