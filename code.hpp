#ifndef CODE_HPP
#define CODE_HPP

#include <iostream>
#include <cassert>
using namespace std;

/* 
 Data representation:
 The command is encoded into one 32-bit value:
 
       |<---------- field B ---------->|<--------- field A ----------->|
       |                               |                               |
       |                  OpCode  Modif|            special ModeA ModeB|
       |                 |1 2 3 4|1 2 3|           |1 2 3 4|1 2 3|1 2 3|
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
       |0|0|0|0|0|0|0|0|0| | | | | | | |0|0|0|0|0|0| | | | | | | | | | |
       +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
	  3                   2                   1

  Special cases:  
    0: no special case
    1: DAT
    2: NOP
    3: SPL
    4: JMP
    5: MOV.I

  This representation looks weired, but it makes it possible to handle
  some simple instructions as special cases, which can be very fast.

  The 'special'-Bitfield is used for some special opcodes, e.g the
  DAT. DAT does not need any adress calculation at all (except for a
  decrement/increment), so it's adressing mode calculation can be very
  different from other opcodes. The same applies for NOP, JMP and SPL.
  Although there is nothing special about MOV.I, it is also encoded
  into the special-Bitfield, because MOV.I is heavily used by every
  warrior and therefor has to be very fast.

  The encoding is done in the following way:

  1. Are we encoding a special opcode?
      Yes: write the according number to the 'special' field
      No:  write 0 into the special field

  2. Encode the Addressing-modes into ModeA and ModeB

  3. Encode the Opcode and the Modifier into OpCode and Modif.
      These fields are only used if there is no special opcode (when
      special=0)

  
  Now to the more interesting decoding process, which I used for the
  optimized interpreter:

  1. Extract the last 9 Bits and create a switch over this value. When
     using 5 special cases, This is a switch over (5+1)*(2^6) = 384
     values. I create the sourcecode automatically. When special=0 an
     ordinary address-mode calculation is done.

  2. If we did not have a special opcode, a second switch is executed,
     it uses the Opcode and Modifier-fields. This is a switch with a
     maximum of 128 cases (depending on how many special cases we
     have).

*/

typedef unsigned long ulong;

enum EOpCode {
    // the order is important
    eDat, eNop, eSpl, eJmp, eMov,
    eAdd, eSub, eMul, eDiv, eMod, eJmz, eJmn, eDjn, eSlt, eCmp, eSne
    // SEQ is equivalent to CMP
    // no LDP, STP
};

enum EModifier {
    eI, eA, eB, eAB, eBA, eF, eX 
};

enum EAddrMode {
    eIm, // #
    eDir, // $
    eInB, // @
    ePreDecInB, // <
    ePostIncInB, // >
    eInA, // *
    ePreDecInA, // {
    ePostIncInA  // }
};


class Code 
{
    // FIXME: this should not be public. How can the Mars-template be
    // a friend of Code?
public:
    ulong ins;
    ulong valA;
    ulong valB;
    
public:
    // Constructors / Destructors
    Code();
    Code(const Code& c);
    Code(EOpCode op, EModifier mod, EAddrMode ma,
	 ulong va, EAddrMode mb, ulong vb);
    
    // get data
    EOpCode GetOpCode() const;
    EModifier GetModifier() const;
    EAddrMode GetAddrModeA() const;
    EAddrMode GetAddrModeB() const;
    ulong GetValA() const;
    ulong GetValB() const;

    // set data
    void SetOpCode(EOpCode op);
    void SetModifier(EModifier mo);
    void SetAddrModeA(EAddrMode ad);
    void SetAddrModeB(EAddrMode ad);
    void SetCode(const Code& c);
    void SetValA(ulong va);
    void SetValB(ulong vb);

    // output
    friend ostream& operator<<(ostream& os, const Code& c);

private:
    int exOpCode() const; // extract the special-field
    int exModifier() const;
    int exSpecial() const;
    int exAddrModeA() const;
    int exAddrModeB() const;
    void inOpCode(int x); // inject new Opcode-value
    void inModifier(int x);
    void inSpecial(int x);
    void inAddrModeA(int x);
    void inAddrModeB(int x);
};

static const Code emptyCode;


// number of special modes
const int specialModes = 5;

// positions of bits
const ulong opCodePos = 19;
const ulong modifierPos = 16;
const ulong specialPos = 6;
const ulong addrModeAPos = 3;
const ulong addrModeBPos = 0;

const ulong opCodeMask = 15;
const ulong modifierMask = 7;
const ulong specialMask = 15;
const ulong addrModeAMask = 7;
const ulong addrModeBMask = 7;

const ulong fieldAMask = 511;
const ulong fieldBMask = 127;

const ulong fieldAPos = 0;
const ulong fieldBPos = 16;

const ulong fieldASwitchMask = fieldAMask << fieldAPos;
const ulong fieldBSwitchMask = fieldBMask << fieldBPos;

const ulong allMask = 0xffffffff;

#endif
