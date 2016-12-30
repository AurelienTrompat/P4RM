#ifndef PCOMMAND_HPP
#define PCOMMAND_HPP


class PCommand
{
    public:

        enum class Agent
        {
            Network
        };

        enum Type
        {
            Quit,
            Test

        };
        Type mType;
        Agent mAgent;
};

#endif // PCOMMAND_HPP
