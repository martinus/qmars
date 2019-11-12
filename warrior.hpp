#ifndef WARRIOR_HPP
#define WARRIOR_HPP

#include <list>
#include "code.hpp"
using namespace std;

class Warrior
{
public:
    // hm. is it good to have all this data public? any better solution?
    list<Code> code;
    ulong start;
    ulong win;
    ulong tie;
    ulong lose;

    Warrior::Warrior()
	: start(0), win(0), tie(0), lose(0)
    {}
};

#endif
