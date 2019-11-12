#ifndef QMARS_HPP
#define QMARS_HPP

#include <iomanip> // setw
#include <iostream>
#include <map>
#include <vector>
#include <ctime>
#include <algorithm>
#include <list>
#include "code.hpp"
#include "fqueue.hpp"
#include "warrior.hpp"
using namespace std;


// qmars: implements the simulation arena. The code is heavily
// optimized to generate as fast code as possible. This does not
// include any processor-specific optimisations.
//
// I tried to use templates in a clever way to improve both speed and
// comfort. 


//// CTAssert ///////////////////////////////////////////////////////
// compile time assertation
template<bool> struct CTAssert;
template<> struct CTAssert<true>{};

//// MarsWarrior ////////////////////////////////////////////////////
// warriors as used in the simulator
template<ulong N>
struct MarsWarrior
{
    FQueue<Code*, N> procQueue;
    MarsWarrior<N>* next;
    ulong* win;
    ulong* tie;
    ulong* lose;
    ulong startPosition;
};


//// Behave /////////////////////////////////////////////////////////
// different behaviours are available for mars. Using template
// metaprogramming techniques, the switch between this different
// behaviours does not cause any performance penalties.
enum EBehaviour
{   eScore,   // print scores of warriors, default behaviour
    eDebug,   // print core and wait after each Step
    eAnalyse, // count how often each case in the switch's is executed.
    eSilent,  // do not output anything
    eInfo     // print each executed instruction
};

template<EBehaviour b, ulong TProcesses>
class Behave {
public:
    static inline void init() {}
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize) {}
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w) {}
};

// eAnalyse: count how often each case in a switch is executed. Can be
// used when optimizing mars.
template<ulong TProcesses>
class Behave<eAnalyse, TProcesses> {
private:
    static map<ulong, ulong> sw1;
    static map<ulong, ulong> sw2;
public:
    static inline void init()
    {
	sw1.clear();
	sw2.clear();
    }
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize)
    {
	// switch for the first value (modes + some instructinons)
	ulong c = currPos->ins & fieldASwitchMask;
	++sw1[c];
	if (c<64) ++sw2[(currPos->ins >> fieldBPos) & fieldBMask];
    }
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w)
    {
        // print stats
        map<ulong, ulong>::iterator pos;
        for (pos=sw1.begin(); pos != sw1.end(); ++pos)
            cout << pos->second << ", " << pos->first << ", 1" << endl;

        for (pos=sw2.begin(); pos != sw2.end(); ++pos)
            cout << pos->second << ", " << pos->first << ", 2" << endl;
    }    
};
template<ulong TProcesses>
map<ulong, ulong> Behave<eAnalyse, TProcesses>::sw1;
template<ulong TProcesses>
map<ulong, ulong> Behave<eAnalyse, TProcesses>::sw2;

// eInfo: print each executed instruction
template<ulong TProcesses>
class Behave<eInfo, TProcesses> {
public:
    static inline void init() {}
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize)
    {
	cout << setw(6) << cycle << " "
	     << setw(4) << currPos-core << "  "
             << *currPos << endl;
    }
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w) {}
};

// eDebug: print core and wait after each Step
template<ulong TProcesses>
class Behave<eDebug, TProcesses> {
public:
    static inline void init() {}
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize)
    {
	currPos -= 15;
	if (currPos<core) currPos += coreSize;
	cout << "cycle " << cycle << endl;
	for(int i=-15; i < 15; i++)
	{
	    cout << setw(5) << currPos-core << ": " << *currPos;
	    if (++currPos == core+coreSize) currPos=core;
	    if (!i) cout << "<--";
	    cout << endl;
	}
	cout << endl << endl;
	cin.get();
    }
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w) {}
};

// eScore: print scores of warriors
template<ulong TProcesses>
class Behave<eScore, TProcesses> {    
public:
    static inline void init() {}
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize) {}
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w)
    {
	MarsWarrior<TProcesses>* start=w;
	do 
	{
	    cout << *(w->win) << " " << *(w->tie) << endl;
	    w = w->next;
	} while (w != start);
    }
};

// eSilent: do not output anything
template<ulong TProcesses>
class Behave<eSilent, TProcesses> {
public:
    static inline void init() {}
    static inline void perCycle(ulong cycle, Code* currPos,
				Code* core, ulong coreSize) {}
    static inline void perCase() {}
    static inline void finish(MarsWarrior<TProcesses>* w) {}
};



//// Mars ///////////////////////////////////////////////////////////
template<ulong TWarriors=2,      // number of warriors
         ulong TCoreSize=8000,   // size of core [8000]
         ulong TCycles=80000,    // cycles until tie [80000]
         ulong TProcesses=8000,  // Max. processes [8000]
         ulong TWarriorDist=100, // Min. warrior dist [100]
	 // behaviour of mars - see above
	 EBehaviour TBehaviour=eScore
         >
class Mars
{
public:
    Mars()
        : coreSentinel(core+TCoreSize),
          coreSentinel1(coreSentinel-1)
    {
        // init random-positions
        for (ulong i=0; i<ePositions; ++i) randomPos[i]=TWarriorDist+i;
        time_t t;
        ulong nr = ulong(time(& t));
        srand(nr);
    }

    // simulate fights
    void simulate(vector<Warrior*>& w, ulong rounds)
    {
	Behave<TBehaviour, TProcesses>::init();
	Clip(rounds);
	InitWarriors(w);
        for (ulong round=0; round<rounds; ++round)
        {
            CTAssert< (TWarriors==2) > only_2_warriors_allowed_at_the_moment;
            Clear();
            SetStartPositions(warriors, round);
            AddWarriors(w, warriors);

            ulong cycles = TWarriors*TCycles + 1;
            ulong warriorCount=TWarriors;
            while (--cycles)
            {
                Code* currPos = currWarrior->procQueue.front();
		Behave<TBehaviour, TProcesses>::perCycle(cycles, currPos, core, TCoreSize);
		// sw.cpp is automaticall generated. The source is not
		// as optimized as it could be, but the compilers are
		// smart enough to do the rest (g++ 2.95.3 is)
#include "sw.cpp"
                // no code allowed in front of the next }
            }
	    AddScores();	    
        }
	Behave<TBehaviour, TProcesses>::finish(warriors);
    }

    
// member methods
private:
    // set positions and order of warriors. The code is somewhat clever:
    // * no fights are done twice
    // * positions are random
    // * the warrior who starts is swapped every time
    void SetStartPositions(MarsWarrior<TProcesses>* warriors, ulong round)
    {
        if (round < ePositions)
        {
            ulong randomArrayPos = (rand() % (ePositions-round)) + round;
            // swap positions
            ulong warriorPos = randomPos[randomArrayPos];
            randomPos[randomArrayPos] = randomPos[round];
            randomPos[round] = warriorPos;
            // set positions
            warriors[0].startPosition=0;
            warriors[1].startPosition=warriorPos;
	    //warriors[1].startPosition=1157;
            if (round % 2) currWarrior=warriors+1;
            else currWarrior=warriors;
        }
        else 
        {
            // same positions
            warriors[0].startPosition=0;
            warriors[1].startPosition=randomPos[round-ePositions];
            // but this time the other one starts first
            if ((round-ePositions) % 2) currWarrior = warriors;
            else currWarrior=warriors+1;
        }
    }

    // add new calculated scores to warriors. At the moment this does
    // only work for 2 warriors.
    void AddScores()
    {
	CTAssert< (TWarriors==2) > only_2_warriors_allowed_at_the_moment;
	ulong alive=0;
	// count nr. of alive warriors. If more than 1 warrior
	// survived, we have a tie
	for (ulong i=0; i<TWarriors; i++)
	    if (!warriors[i].procQueue.empty()) ++alive;
	
	// tie
	if (alive >= 2)
	{
	    for (ulong i=0; i<TWarriors; i++) *(warriors[i].tie) += 1;
	}
	else 
	{
	    // all alive warriors get a score of 3
	    for (ulong i=0; i<TWarriors; i++)
	    {
		if (!warriors[i].procQueue.empty()) *(warriors[i].win) += 1;
		else *(warriors[i].lose) +=1;
	    }
	}
    }

    // init MarsWarriors
    void InitWarriors(vector<Warrior*>& w)
    {
	// setup marswarriors
        for (ulong i=0; i<TWarriors; i++)
        {
            warriors[i].win = &(w[i]->win);
            warriors[i].tie = &(w[i]->tie);
            warriors[i].lose = &(w[i]->lose);
            warriors[i].next = warriors+i+1;
        }
        // correct last warrior to cyclic list
        warriors[TWarriors-1].next = warriors;
    }
    
    // clear the core
    void Clear()
    {
        Code* c;
        for (c=core; c < core+TCoreSize%8; ++c) *c = emptyCode;
        while (c < coreSentinel)
        {
	    // unrolled loop
            *c = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            *(++c) = emptyCode;
            ++c;
	}
        for (ulong i=0; i<TWarriors; ++i) warriors[i].procQueue.clear();
    }

    // clip rounds:
    // 0 -> maximum of possible fights
    // bigger than maximum -> maximum
    void Clip(ulong& rounds)
    {
	if (rounds == 0) rounds = eFights;
	else if (rounds > eFights) rounds = eFights;
    }

    // insert codes of warriors into core, set starting positions
    void AddWarriors(vector<Warrior*>& w, MarsWarrior<TProcesses>* mw)
    {
        for (ulong i=0; i<TWarriors; i++)
        {            
            list<Code>::iterator it = w[i]->code.begin();
            Code* c = core + mw[i].startPosition;
            while (it != w[i]->code.end())
            {
                *c = *it;
                ++c;
                ++it;
            }
            mw[i].procQueue.push(core + mw[i].startPosition + w[i]->start);
        }
    }

// member variables
private:
    // number of possible fights. there are twice as many fights as
    // positions available, because it depends which warriors starts
    enum { eFights = 2*(TCoreSize+1 - 2*TWarriorDist) };
    // number of possible positions
    enum { ePositions = eFights/2 };
    enum { eCoreSize1 = TCoreSize-1 };
    // Core Memory
    Code core[TCoreSize];
    const Code* coreSentinel;
    const Code* coreSentinel1;
    // positions
    ulong randomPos[ePositions];
    MarsWarrior<TProcesses> warriors[TWarriors];
    // current Warrior
    MarsWarrior<TProcesses>* currWarrior;
};

#endif
