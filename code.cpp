// $Id: code.cpp,v 1.4 2002/03/02 23:46:13 martin Exp $
#include <iostream>
#include <iomanip>
#include <string>
#include "code.hpp"
using namespace std;

Code::Code()
    : valA(0), valB(0)
{
    ins=0;
    SetOpCode(eDat);
    SetModifier(eF);
    SetAddrModeA(eDir);
    SetAddrModeB(eDir);
}

Code::Code(const Code& c)
    : ins(c.ins), valA(c.valA), valB(c.valB)
{}

Code::Code(EOpCode op, EModifier mod, EAddrMode ma,
	   ulong va, EAddrMode mb, ulong vb)
    : valA(va), valB(vb)
{
    ins=0;
    SetOpCode(op);
    SetModifier(mod);
    SetAddrModeA(ma);
    SetAddrModeB(mb);
}
    
// get data
EOpCode Code::GetOpCode() const
{
    switch(exSpecial())
    {
    case 0:
	return(static_cast<EOpCode>(exOpCode()));
	break; 
    case 1: return(eDat); break;
    case 2: return(eNop); break;
    case 3: return(eSpl); break;
    case 4: return(eJmp); break;
    case 5: return(eMov); break;
    default:
	break;
    }
    // unknown special code
    assert(0);
    return(eDat);
}

EModifier Code::GetModifier() const
{
    return(static_cast<EModifier>(exModifier()));
}

EAddrMode Code::GetAddrModeA() const
{
    return(static_cast<EAddrMode>(exAddrModeA()));
}

EAddrMode Code::GetAddrModeB() const
{
    return(static_cast<EAddrMode>(exAddrModeB()));
}

ulong Code::GetValA() const
{
    return(valA);
}

ulong Code::GetValB() const
{
    return(valB);
}

// set data
void Code::SetOpCode(EOpCode op) 
{
    inOpCode(0);
    inSpecial(0);
    switch(op)
    {
    case eDat: inSpecial(1); break;
    case eNop: inSpecial(2); break;
    case eSpl: inSpecial(3); break;
    case eJmp: inSpecial(4); break;
    case eMov:
	if (GetModifier()==eI) inSpecial(5);
	else inOpCode(op);
	break;
    default:
	inOpCode(op);
    }
}

void Code::SetModifier(EModifier mo)
{
    EOpCode op = GetOpCode();
    inOpCode(0);
    inModifier(mo);
    SetOpCode(op);
}
    
void Code::SetAddrModeA(EAddrMode ad)
{
    inAddrModeA(ad);
}

void Code::SetAddrModeB(EAddrMode ad)
{
    inAddrModeB(ad);
}    
    
void Code::SetValA(ulong va)
{
    valA = va;
}

void Code::SetValB(ulong vb)
{
    valB = vb;
}

int Code::exOpCode() const
{
    return((ins>>opCodePos) & opCodeMask);
}

int Code::exModifier() const
{
    return((ins>>modifierPos) & modifierMask);
}

int Code::exSpecial() const 
{
    return((ins>>specialPos) & specialMask);
}

int Code::exAddrModeA() const 
{
    return((ins>>addrModeAPos) & addrModeAMask);
}

int Code::exAddrModeB() const
{
    return((ins>>addrModeBPos) & addrModeBMask);
}

void Code::inOpCode(int x)
{
    ins = ins & ((opCodeMask << opCodePos) ^ allMask);
    ins = ins | (x << opCodePos);
}
    
void Code::inModifier(int x)
{
    ins = ins & ((modifierMask << modifierPos) ^ allMask);
    ins = ins | (x << modifierPos);
}

void Code::inSpecial(int x)
{
    ins = ins & ((specialMask << specialPos) ^ allMask);
    ins = ins | (x << specialPos);
}

void Code::inAddrModeA(int x)
{
    ins = ins & ((addrModeAMask << addrModeAPos) ^ allMask);
    ins = ins | (x << addrModeAPos);
}

void Code::inAddrModeB(int x)
{
    ins = ins & ((addrModeBMask << addrModeBPos) ^ allMask);
    ins = ins | (x << addrModeBPos);
}

ostream& operator<<(ostream& os, const Code& c)
{
    switch (c.GetOpCode())
    {
    case eDat: os << "dat"; break;
    case eMov: os << "mov"; break;
    case eAdd: os << "add"; break;
    case eSub: os << "sub"; break;
    case eMul: os << "mul"; break;
    case eDiv: os << "div"; break;
    case eMod: os << "mod"; break;
    case eJmp: os << "jmp"; break;
    case eJmz: os << "jmz"; break;
    case eJmn: os << "jmn"; break;
    case eDjn: os << "djn"; break;
    case eSpl: os << "spl"; break;
    case eSlt: os << "slt"; break;
    case eCmp: os << "seq"; break;
    case eSne: os << "sne"; break;
    case eNop: os << "nop"; break;
    default: throw("unknown Opcode");
    }

    os << ".";

    switch(c.GetModifier())
    {
    case eI:  os << "i  "; break;
    case eA:  os << "a  "; break;
    case eB:  os << "b  "; break;
    case eAB: os << "ab "; break;
    case eBA: os << "ba "; break;
    case eF:  os << "f  "; break;
    case eX:  os << "x  "; break;
    default: throw("unknown Modifier");
    }

    switch(c.GetAddrModeA())
    {
    case eIm:         os << '#'; break;
    case eDir:        os << '$'; break;
    case eInB:        os << '@'; break;
    case ePreDecInB:  os << '<'; break;
    case ePostIncInB: os << '>'; break;
    case eInA:        os << '*'; break;
    case ePreDecInA:  os << '{'; break;
    case ePostIncInA: os << '}'; break;
    default: throw("unknown Address Mode");
    }
    
    os << std::setw(5)
       << c.GetValA()
       << " , ";

    switch(c.GetAddrModeB())
    {
    case eIm:         os << '#'; break;
    case eDir:        os << '$'; break;
    case eInB:        os << '@'; break;
    case ePreDecInB:  os << '<'; break;
    case ePostIncInB: os << '>'; break;
    case eInA:        os << '*'; break;
    case ePreDecInA:  os << '{'; break;
    case ePostIncInA: os << '}'; break;
    default: throw("unknown Address Mode");
    }
    
    os << std::setw(5)
       << c.GetValB();
    return(os);
}
