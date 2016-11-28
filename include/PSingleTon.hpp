#ifndef PSINGLETON_HPP
#define PSINGLETON_HPP

template<typename T>
class PSingleTon
{
    protected:
        PSingleTon(){}
        ~PSingleTon(){}

    public:

        static T& getInstance()
        {
            return m_singleTon;
        }
    private:
        T& operator= (const T&){}

    private:
        static T m_singleTon;

};
template<typename T>
T PSingleTon<T>::m_singleTon = T();
#endif // PSINGLETON_HPP
