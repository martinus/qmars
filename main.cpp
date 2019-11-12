#include <iostream>
#include <vector>
#include "warrior.hpp"
#include "qmars.hpp"
#include "parser.hpp"
using namespace std;


int main(int argc, char* argv[])
{
    if (argc != 4) 
    {
	cerr << "usage: qmars <rounds> <file1> <file2>" << endl;
	exit(1);
    }

    const ulong cWarriors    = 2;
    const ulong cCoreSize    = 8000;
    const ulong cCycles      = 80000;
    const ulong cProcesses   = 8000;
    const ulong cWarriorLen  = 100;
    const ulong cWarriorDist = 100;

   
    Parser p;

    // create mars
    Mars<2, 8000, 80000, 8000, 100, eScore> m;
    
    // create array of warriors
    vector<Warrior*> warriors;
    warriors.push_back(p.parse(argv[2], cCoreSize));
    warriors.push_back(p.parse(argv[3], cCoreSize));

    // simulate warriors
    m.simulate(warriors, atoi(argv[1]));
}
