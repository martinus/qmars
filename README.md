# qmars

QMars stands for Quicker Mars. It is a completely new implementation of a mars simulator, which is an interpreter of the Assembler-like language RedCode, which is used in CoreWars. QMars is heavily optimized to be as fast as possible. Here are the main advantages/disadvantages of QMars:

| pros | cons |
|---|---|
| completely written in C++ | slow compilation: with full optimization over 1 minute (on a AMD Duron 650). |
| simple interface |  huge amount of RAM needed during compilation (with full optimization: 110MB) |
| fastest simulator on planet | does not support LDP, STP (but should not be difficult to add) |
| small executeable, low RAM-usage | less tested than other simulators  |
| extensible: using Template Metaprogramming techniques, the behaviour (output, debugging) of the simulator can easily be modified  | |

IMHO QMars is a nice addition to other existing mars simulators. pmars has a lot of features and a nice user interface, exhaust is a lightweight simulator written in C.
	
The source is under a BSDish licence. This is not as restrictive as GPL (you can even sell this program, if you want...).
	
## Benchmarks

Both projects are compiled using g++ 2.95.3 and this command-line option -O9 -fomit-frame-pointer -fforce-mem -fforce-addr -finline-functions -funroll-loops -mcpu=i686 -march=i686.

```
                          pmars 0.9.2   QMars           
           Fixed vs    Fixed    24.93       16.65   66.8%
          Jaguar vs    Fixed    29.16       18.92   64.9%
          Jaguar vs   Jaguar    43.21       27.47   63.6%
         Stalker vs    Fixed    24.84       17.56   70.7%
         Stalker vs   Jaguar    26.06       18.52   71.1%
         Stalker vs  Stalker    21.12       15.40   72.9%
        nPaperII vs    Fixed    35.46       23.18   65.4%
        nPaperII vs   Jaguar    44.55       30.14   67.7%
        nPaperII vs  Stalker    30.33       20.48   67.5%
        nPaperII vs nPaperII    50.75       32.89   64.8%
        -------------------------------------------------
        Sum                    330.28      221.21   
                               100%         67.0%
```             

As you can see, QMars takes only 67% of the time that pmars uses. pmars can do 10*2000/330.28 = 60.6 fights/seconds, QMars 90.4. This makes QMars (90.4-60.6)/60.6 = 49.2% faster than pmars :-)

## Usage

QMars is very easy to use, which makes it a good choice for Evolvers. What follows is a perfectly workable code:
	

```cpp
#include <iostream>
#include <vector>
#include "warrior.hpp"
#include "qmars.hpp"
#include "parser.hpp"
using namespace std;

int main(int argc, char* argv[])
{
    const ulong cWarriors    = 2;
    const ulong cCoreSize    = 8000;
    const ulong cCycles      = 80000;
    const ulong cProcesses   = 8000;
    const ulong cWarriorDist = 100;
    Parser p;

    // create mars for redcode-standard. It should not output anything.
    Mars<cWarriors, cCoreSize, cCycles, cProcesses, cWarriorDist, eSilent> m;
    // create array of pointers warriors. The simulator will use this array
    vector<Warrior*> warriors(2);
    
    // create an Imp
    Warrior* imp = new Warrior;
    imp->code.push_back(Code(eMov, eI, eDir, 0, eDir, 1));

    // parse a warrior (the parser needs the output of pmars)
    warriors[0] = imp;
    warriors[1] = p.parse("warriors/jaguar.rc", cCoreSize);

    // simulate warriors with 1000 fights
    m.simulate(warriors, 1000);

    // print scores
    cout << imp->win << " " << imp->tie << endl
         << warriors[1]->win << " " << warriors[1]->tie << endl;
}
```
