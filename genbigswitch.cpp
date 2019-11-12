#include <iostream>
#include <iomanip> // setw
#include <string>
#include "code.hpp"
using namespace std;

// genbigswitch is a standalone program which generates the file
// sw.cpp, which is used by qmars.hpp.  This code is everything but well
// organized, because I often had to modify huge parts in it. Another
// reason for this is that I do not know of any clean method to
// generate sourcecode, you are invited to make it better :)

void NextWarrior(ostream& os)
{
    os << "currWarrior = currWarrior->next;" << endl;
}

void CheckDie(ostream& os)
{
    os << "if (currWarrior->procQueue.empty()) {" << endl
       << "    --warriorCount;" << endl
       << "    if (warriorCount>1) {" << endl
       << "        currWarrior->next = currWarrior->next->next;" << endl
       << "        cycles -= cycles / (warriorCount+1);" << endl
       << "    }" << endl
       << "    else {" << endl
       << "        cycles=1;" << endl
       << "    }" << endl
       << "}" << endl;
    NextWarrior(os);
}

void NextPos(ostream& os)
{
    os << "if (++currPos==coreSentinel) currPos=core;" << endl;
}

void savePush(ostream& os, string pos)
{
    os << "currWarrior->procQueue.savePush(" << pos << ");" << endl;
}

void pop(ostream& os)
{
    os << "currWarrior->procQueue.pop();" << endl;
}


void popAndPush(ostream& os, string pos)
{
    os << "currWarrior->procQueue.popAndPush(" << pos << ");" << endl;
}

void popAndPushNextPos(ostream& os)
{
    // first try
    // os << "if (currPos==coreSentinel1) currWarrior->procQueue.popAndPush(core);" << endl;
    // os << "else currWarrior->procQueue.popAndPush(++currPos);" << endl;

    // second try, a little faster
    // os << "currWarrior->procQueue.popAndPush(currPos==coreSentinel1 ? core : ++currPos);" << endl;

    // third try, even faster
    os << "currWarrior->procQueue.popAndPush(currPos==coreSentinel1 ? core : currPos+1);" << endl;
}
    

// generate code for the specific needs
void GenCodeAddrMode(ostream& os, EAddrMode ma, EAddrMode mb,
		     bool da, bool ra_a, bool ra_b,
		     bool db, bool rb_a, bool rb_b)
{
    os << "{" << endl;
    os << "// a-field address mode calculation" << endl;
    
    if (db || rb_a || rb_b || (ra_b && eIm) || 
	(mb==ePreDecInA) || (mb==ePreDecInB) ||
	(mb==ePostIncInB) || (mb==ePostIncInA))
	os << "rb_b = currPos->valB;" << endl;

    if (rb_a && (mb==eIm))
    {
	// necessary if B-mode is immediate
	os << "rb_a = currPos->valA;" << endl;
    }
    
    switch (ma)
    {
    case eIm:
	if (ra_a)
	    os << "ra_a = currPos->valA;" << endl;
	if (ra_b)
	    os << "ra_b = rb_b;" << endl;
	if (da)
	    os << "da = currPos;" << endl;
	break;

    case eDir:
	if (da || ra_a || ra_b)
	    os << "da = currPos+currPos->valA;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	break;

    case eInA:
	if (da || ra_a || ra_b) 
	{
	    os << "da = currPos+currPos->valA;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	    os << "da += da->valA;" << endl;
	    os << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	}
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	break;

    case eInB:
	if (da || ra_a || ra_b) 
	{
	    os << "da = currPos+currPos->valA;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	    os << "da += da->valB;" << endl;
	    os << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	}
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	break;
	
    case ePreDecInA:
	//  always inc/dec
	os << "da = currPos+currPos->valA;" << endl
	   << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	if (da || ra_a || ra_b)
	{
            os << "ulong* pofs = &(da->valA);" << endl;
	    os << "ulong t = *pofs;" << endl;
	    os << "if (--t == -1UL) t=eCoreSize1;" << endl;
	    os << "*pofs = t;" << endl;
	    os << "da += t;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	}
	else
	    os << "da->valA = (da->valA ? da->valA-1 : eCoreSize1);" << endl;
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	break;

    case ePreDecInB:
	//  always inc/dec
	os << "da = currPos+currPos->valA;" << endl
	   << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	if (da || ra_a || ra_b) 
	{
	    os << "ulong* pofs = &(da->valB);" << endl;
	    os << "ulong t = *pofs;" << endl;
	    os << "if (--t == -1UL) t=eCoreSize1;" << endl;
	    os << "*pofs = t;" << endl;
	    os << "da += t;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	}
	else
	    os << "da->valB = (da->valB ? da->valB-1 : eCoreSize1);" << endl;
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	break;
	
    case ePostIncInA:
	//  always inc/dec
	os << "da = currPos+currPos->valA;" << endl
	   << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	os << "ulong* pofs = &(da->valA);" << endl;
	os << "ulong t = *pofs;" << endl;
	if (da || ra_a || ra_b)
	    os << "da += t;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	os << "if (++t == TCoreSize) t=0;" << endl;
	os << "*pofs = t;" << endl;
	break;	

    case ePostIncInB:
	os << "da = currPos+currPos->valA;" << endl
	   << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	os << "ulong* pofs = &(da->valB);" << endl;
	os << "ulong t = *pofs;" << endl;
	if (da || ra_a || ra_b)
	    os << "da += t;" << endl
	       << "if (da >= coreSentinel) da -= TCoreSize;" << endl;
	if (ra_a)
	    os << "ra_a = da->valA;" << endl;
	if (ra_b)
	    os << "ra_b = da->valB;" << endl;
	os << "if (++t == TCoreSize) t=0;" << endl;
	os << "*pofs = t;" << endl;
	break;

    default:
	assert(0);
	break;
    }

    os << "}" << endl;
    os << "{" << endl;
    
    os << "// b-field address mode calculation" << endl;
    switch (mb)
    {
    case eIm:
	// rb_a saved before a-mode calculation
	if (db)
	    os << "db = currPos;" << endl;
	break;

    case eDir:
	if (db || rb_a || rb_b)
	    os << "db = currPos+rb_b;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	break;

    case eInA:
	if (db || rb_a || rb_b) 
	{
	    os << "db = rb_b+currPos;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	    os << "db += db->valA;" << endl;
	    os << "if (db >= coreSentinel) db -= TCoreSize;" << endl;    
	}
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	break;

    case eInB:
	if (db || rb_a || rb_b) 
	{
	    os << "db = rb_b+currPos;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	    os << "db += db->valB;" << endl;
	    os << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	}
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	break;
	
    case ePreDecInA:
	//  always inc/dec
	os << "db = rb_b+currPos;" << endl
	   << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	if (db || rb_a || rb_b)
	{
	    os << "ulong* pofs = &(db->valA);" << endl;
	    os << "ulong t = *pofs;" << endl;
	    os << "if (--t == -1UL) t=eCoreSize1;" << endl;
	    os << "*pofs = t;" << endl;
	    os << "db += t;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	}
	else
	    os << "db->valA = (db->valA ? db->valA-1 : eCoreSize1);" << endl;
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	break;

    case ePreDecInB:
	//  always inc/dec
	os << "db = rb_b+currPos;" << endl
	   << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	if (db || rb_a || rb_b) 
	{
            os << "ulong* pofs = &(db->valB);" << endl;
	    os << "ulong t = *pofs;" << endl;
	    os << "if (--t == -1UL) t=eCoreSize1;" << endl;
	    os << "*pofs = t;" << endl;
	    os << "db += t;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	}
	else
	    os << "db->valB = (db->valB ? db->valB-1 : eCoreSize1);" << endl;
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	break;
	
    case ePostIncInA:
	//  always inc/dec
	os << "db = rb_b+currPos;" << endl
	   << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	os << "ulong* pofs = &(db->valA);" << endl;
	os << "ulong t = *pofs;" << endl;
	if (db || rb_a || rb_b)
	    os << "db += t;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	os << "if (++t == TCoreSize) t=0;" << endl;
	os << "*pofs = t;" << endl;
	break;	

    case ePostIncInB:
	os << "db = rb_b+currPos;" << endl
	   << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	os << "ulong* pofs = &(db->valB);" << endl;
	os << "ulong t = *pofs;" << endl;
	if (db || rb_a || rb_b)
	    os << "db += t;" << endl
	       << "if (db >= coreSentinel) db -= TCoreSize;" << endl;
	if (rb_a)
	    os << "rb_a = db->valA;" << endl;
	if (rb_b)
	    os << "rb_b = db->valB;" << endl;
	os << "if (++t == TCoreSize) t=0;" << endl;
	os << "*pofs = t;" << endl;
	break;

    default:
	assert(0);
	break;
    }
    os << "}" << endl;
}







// generate the same code as above using local variables
void GenCodeAddrModeLocal(ostream& os, EAddrMode ma, EAddrMode mb,
		     bool da, bool ra_a, bool ra_b,
		     bool db, bool rb_a, bool rb_b)
{
    os << "Code* da, *db;" << endl
       << "ulong ra_b, rb_a;" << endl
       << "ulong ra_a, rb_b;" << endl;

    GenCodeAddrMode(os, ma, mb,
		    da, ra_a, ra_b,
		    db, rb_a, rb_b);
}




int main()
{
    ostream & os=cout;
    Code c;

    ////// generate 1st switch
    // A, B Register values
    os << "Code* da, *db;" << endl
       << "ulong ra_b, rb_a;" << endl
       << "ulong ra_a, rb_b;" << endl;
        // effective addresses of instruction's a- and b-fields
    
    os << "switch(currPos->ins & fieldASwitchMask)" << endl
       << "{" << endl;

    // full address mode calculations
    for (ulong i=0; i<64; i++)
    {
        os << "case " << i << ": // full calculation" << endl;
	os << "{" << endl;
	c.ins = i;
	GenCodeAddrMode(os, c.GetAddrModeA(), c.GetAddrModeB(),
			true, true, true,
			true, true, true);
	os << "}" << endl;
        os << "break;" << endl
           << endl;
    }

    // special modes
    for (ulong i=64; i < ((specialModes+1)<<specialPos); i++)
    {
        os << "case " << i << ": // ";
	os.setf(ios::hex);
	os.unsetf(ios::dec);
	os.setf(ios::showbase);
	os << i << endl;
	os.setf(ios::dec);
	os.unsetf(ios::showbase);
	os.unsetf(ios::hex);
	
	// set to something that does not need special handling
	c.ins = i;
        EOpCode op = c.GetOpCode();
        EAddrMode ma = c.GetAddrModeA();
        EAddrMode mb = c.GetAddrModeB();

	os << "{" << endl;
        // some special opcodes
        switch (op)
        {
        case eDat:
	    GenCodeAddrModeLocal(os, ma, mb,
			    false, false, false,
			    false, false, false);
            os << "// DAT" << endl;
	    pop(os);
            CheckDie(os);
            os << "continue;" << endl;
            break;
            
        case eNop:
	    GenCodeAddrModeLocal(os, ma, mb,
			    false, false, false,
			    false, false, false);
            os << "// NOP" << endl;
	    popAndPushNextPos(os);
            NextWarrior(os);
            os << "continue;" << endl;
            break;
            
        case eJmp:
	    GenCodeAddrModeLocal(os, ma, mb,
			    true,  false, false,
			    false, false, false);
            os << "// JMP" << endl;
            popAndPush(os, "da");
            NextWarrior(os);
            os << "continue;" << endl;
            break;
            
        case eSpl:
	    GenCodeAddrModeLocal(os, ma, mb,
			    true, false, false,
			    false, false, false);
            os << "// SPL" << endl;
	    popAndPushNextPos(os);
	    savePush(os, "da");
	    // end if
            NextWarrior(os);
            os << "continue;" << endl;
            break;

	case eMov:
	    // this is only for mov.i
	    GenCodeAddrModeLocal(os, ma, mb,
			    true, true, true,
			    true, false, false);
	    os << "db->valA = ra_a;" << endl
	       << "db->valB = ra_b;" << endl;
	    os << "db->ins = da->ins;" << endl;
	    popAndPushNextPos(os);
	    NextWarrior(os);
            os << "continue;" << endl;
	    break;
	    
        default:
	    // no known special mode
	    assert(0);
            break;
        }

	os << "}" << endl;
        os << "break;" << endl
           << endl;
    }

    // that was the first switch!
    os << "default:" << endl
       << "    assert(0);" << endl
       << "    break;" << endl
       << "}" << endl << endl;



    // the generation of the next switch is not really necessary, it
    // would also be easily possible to handwrite it. But I would give
    // up some flexibility, when I want to come up with further
    // optimisations.

    // set addrModes to 0, so that GetOpCode checks the opcode in
    // modifiers.
    // generate 2nd switch
    os  << "switch((currPos->ins >> fieldBPos) & fieldBMask)" << endl
        << "{" << endl;

    for (int op = eDat; op <= eSne; op++)
    {
	if ((op==eDat) ||
	    (op==eNop) ||
	    (op==eSpl) ||
	    (op==eJmp)) continue;
	
	for (int mod=eI; mod <= eX; mod++)
	{
	    if ((op==eMov) && (mod==eI)) continue;	
	    int nr = ((op<<opCodePos)|(mod<<modifierPos)) >> fieldBPos;
	    os << "case " << nr << ": // ";
	    os.setf(ios::hex);
	    os.unsetf(ios::dec);
	    os.setf(ios::showbase);
	    os << nr << endl;
	    os.setf(ios::dec);
	    os.unsetf(ios::showbase);
	    os.unsetf(ios::hex);

	    os << "{" << endl;
	    
	    
	    switch (op)
	    {
	    case eMov:
		switch (mod)
		{
		case eA:
		    os << "db->valA = ra_a;" << endl;
		    break;
		case eF:
		    os << "db->valA = ra_a;" << endl; // fallthrough!
		case eB:
		    os << "db->valB = ra_b;" << endl;
		    break;
		case eAB:
		    os << "db->valB = ra_a;" << endl;
		    break;
		case eX:
		    os << "db->valB = ra_a;" << endl; // fallthrough!
		case eBA:
		    os << "db->valA = ra_b;" << endl;
		    break;
		case eI:
		    // this mov already handled in first switch!
		    // (special case)
		    assert(0);
		    
		    os << "db->valA = ra_a;" << endl
		       << "db->valB = ra_b;" << endl;
		    os << "db->ins = da->ins;" << endl;
		    break;
		default: assert(0);
		}
		popAndPushNextPos(os);
		NextWarrior(os);
		break;
		
	    case eJmz:
		switch (mod)
		{
		case eBA:  // fallthrough!
		case eA:
		    os << "if (rb_a) {" << endl;
		    popAndPushNextPos(os);
		    os << "} else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		    
		case eAB:  // fallthrough!
		case eB:
		    os << "if (rb_b) {" << endl;
		    popAndPushNextPos(os);
		    os << "} else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		    
		case eF:   // fallthrough!
		case eX:   // fallthrough!
		case eI:
		    os << "if (rb_a || rb_b) {" << endl;
		    popAndPushNextPos(os);
		    os << "} else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		default: assert(0);
		}
		NextWarrior(os);
		break;          
		
	    case eJmn:
		switch (mod)
		{
		case eBA:  // fallthrough!
		case eA:
		    os << "if (!rb_a) {" << endl;
		    popAndPushNextPos(os);
		    os << " } else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		    
		case eAB:  // fallthrough!
		case eB:
		    os << "if (!rb_b) {" << endl;
		    popAndPushNextPos(os);
		    os << "} else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		    
		case eF:   // fallthrough!
		case eX:   // fallthrough!
		case eI:
		    os << "if (!(rb_a || rb_b)) {" << endl;
		    popAndPushNextPos(os);
		    os << "} else {" << endl;
		    popAndPush(os, "da");
		    os << "}" << endl;
		    break;
		default: assert(0);
		}
		NextWarrior(os);
		break;          
		
	    case eAdd:
		switch (mod)
		{
		case eI:   // fallthrough!
		case eF:
		    os << "ulong t = ra_b + rb_b;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    os << "t = ra_a + rb_a;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		    
		case eA:
		    os << "ulong t = ra_a + rb_a;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		    
		case eB:
		    os << "ulong t = ra_b + rb_b;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		    
		case eX:   // fallthrough!
		    os << "ulong t = ra_b + rb_a;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    os << "t = ra_a + rb_b;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		case eAB:
		    os << "ulong t = ra_a + rb_b;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		    
		case eBA:
		    os << "ulong t = ra_b + rb_a;" << endl
		       << "if (t >= TCoreSize) t -= TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		default: assert(0);
		}
		popAndPushNextPos(os);
		NextWarrior(os);
		break;
		
	    case eSub:
		switch (mod)
		{
		case eI:   // fallthrough!
		case eF:
		    os << "ulong t = rb_b - ra_b;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    os << "t = rb_a - ra_a;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		case eA:
		    os << "ulong t = rb_a - ra_a;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		    
		case eB:
		    os << "ulong t = rb_b - ra_b;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		    
		case eX:   // fallthrough!
		    os << "ulong t = rb_a - ra_b;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    os << "t = rb_b - ra_a;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		case eAB:
		    os << "ulong t = rb_b - ra_a;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valB = t;" << endl;
		    break;
		    
		case eBA:
		    os << "ulong t = rb_a - ra_b;" << endl
		       << "if (t+TCoreSize < TCoreSize) t += TCoreSize;" << endl
		       << "db->valA = t;" << endl;
		    break;
		default: assert(0);
		}
		popAndPushNextPos(os);
		NextWarrior(os);
		break;
		
	    case eMul:
		switch (mod)
		{
		case eI:   // fallthrough!
		case eF:   // fallthrough!
		    os << "db->valB = (rb_b * ra_b) % TCoreSize;" << endl;
		case eA:
		    os << "db->valA = (rb_a * ra_a) % TCoreSize;" << endl;
		    break;
		    
		case eB:
		    os << "db->valB = (rb_b * ra_b) % TCoreSize;" << endl;
		    break;
		    
		case eX:   // fallthrough!
		    os << "db->valA = (rb_a * ra_b) % TCoreSize;" << endl;
		case eAB:
		    os << "db->valB = (rb_b * ra_a) % TCoreSize;" << endl;
		    break;
		    
		case eBA:
		    os << "db->valA = (rb_a * ra_b) % TCoreSize;" << endl;
		    break;
		default: assert(0);
		}
		popAndPushNextPos(os);
		NextWarrior(os);
		break;
		
	    case eDiv:
		switch (mod)
		{
		case eI:   // fallthrough!
		case eF:   // fallthrough!
		    os << "if (ra_a) db->valA = rb_a / ra_a;" << endl
		       << "if (ra_b) db->valB = rb_b / ra_b;" << endl
		       << "if (ra_a && ra_b) {" << endl;
		    break;
                    
		case eX:
		    os << "if (ra_b) db->valA = rb_a / ra_b;" << endl
		       << "if (ra_a) db->valB = rb_b / ra_a;" << endl
		       << "if (ra_a && ra_b) {" << endl;
		    break;
		    
		case eA:
		    os << "if (ra_a) {" << endl
		       << "    db->valA = rb_a / ra_a;" << endl;
		    break;
		    
		case eB:
		    os << "if (ra_b) {" << endl
		       << "    db->valB = rb_b / ra_b;" << endl;
                break;

            case eAB:
                os << "if (ra_a) {" << endl
                   << "    db->valB = rb_b / ra_a;" << endl;
                break;

            case eBA:
                os << "if (ra_b) {" << endl
                   << "    db->valB = rb_a / ra_b;" << endl;
                break;
            default: assert(0);
            }
	    // ok, no division by 0
	    popAndPushNextPos(os);
	    NextWarrior(os);
	    os << "} else {" << endl; // division by 0, same as execution of DAT
	    pop(os);
	    CheckDie(os);
	    os << "}" << endl;
            break;

        case eMod:
            switch (mod)
            {
            case eI:   // fallthrough!
            case eF:   // fallthrough!
                os << "if (ra_a) db->valA = rb_a % ra_a;" << endl
                   << "if (ra_b) db->valB = rb_b % ra_b;" << endl
                   << "if (ra_a && ra_b) {" << endl;
                break;
                    
            case eX:
                os << "if (ra_b) db->valA = rb_a % ra_b;" << endl
                   << "if (ra_a) db->valB = rb_b % ra_a;" << endl
                   << "if (ra_a && ra_b) {" << endl;
                break;

            case eA:
                os << "if (ra_a) {" << endl
                   << "    db->valA = rb_a % ra_a;" << endl;
                break;
                
            case eB:
                os << "if (ra_b) {" << endl
                   << "    db->valB = rb_b % ra_b;" << endl;
		
                break;

            case eAB:
                os << "if (ra_a) {" << endl
                   << "    db->valB = rb_b % ra_a;" << endl;
                break;

            case eBA:
                os << "if (ra_b) {" << endl
                   << "    db->valB = rb_a % ra_b;" << endl;
                break;
            default: assert(0);
            }
	    popAndPushNextPos(os);
	    NextWarrior(os);
	    os << "} else {";
	    // division by 0, same as execution of DAT
	    pop(os);
	    CheckDie(os);
	    os << "}";
	    break;


            //os << "currPos = currPos==eCoreSize1 ? 0 : currPos+1;" << endl
            
        case eCmp:  // same as SEQ
            switch (mod)
            {
            case eA:
                os << "if (ra_a == rb_a) {" << endl;
                break;
                
            case eB:
                os << "if (ra_b == rb_b) {" << endl;
                break;

            case eAB:
                os << "if (ra_a == rb_b) {" << endl;
                break;

            case eBA:
                os << "if (ra_b == rb_a) {" << endl;
                break;

            case eI:
                os << "if ((da->ins == db->ins) &&" << endl
		   << "    (ra_a == rb_a) && (ra_b == rb_b)) {" << endl;
                break;

            case eF:   // F
                os << "if ((ra_a == rb_a) && (ra_b == rb_b)) {" << endl;
                break;
                
            case eX:   // X
                os << "if ((ra_a == rb_b) && (ra_b == rb_a)) {" << endl;
                break;
            default: assert(0);
            }
	    NextPos(os);
	    os << "}" << endl;
	    // endif
	    popAndPushNextPos(os);
	    NextWarrior(os);
            break;
            

        case eDjn:
            switch (mod)
            {
            case eBA:  // fallthrough!
            case eA:
                os << "ulong t = db->valA;" << endl
		   << "if (--t == -1UL) t=eCoreSize1;" << endl
                   << "db->valA = t;" << endl
                   << "if (rb_a == 1) {" << endl;
                break;

            case eAB:  // fallthrough!
            case eB:
                os << "ulong t = db->valB;" << endl
		   << "if (--t == -1UL) t=eCoreSize1;" << endl
                   << "db->valB = t;" << endl
                   << "if (rb_b == 1) {" << endl;
                break;
                
            case eF:   // fallthrough!
            case eX:   // fallthrough!
            case eI:
                os << "ulong t = db->valA;" << endl
		   << "if (--t == -1UL) t=eCoreSize1;" << endl
                   << "db->valA = t;" << endl
                   << "t = db->valB;" << endl
		   << "if (--t == -1UL) t=eCoreSize1;" << endl
                   << "db->valB = t;" << endl
                   << "if ((rb_a==1) && (rb_b==1)) {" << endl;
                break;
		
            default: assert(0);
            }
	    popAndPushNextPos(os);
	    os << "} else {" << endl;
	    popAndPush(os, "da");
	    os << "}" << endl;
	    NextWarrior(os);
            break;          

        case eSlt:
            switch (mod)
            {
            case eA:
                os << "if (ra_a < rb_a) {" << endl;
                break;
                
            case eB:
                os << "if (ra_b < rb_b) {" << endl;
                break;

            case eAB:
                os << "if (ra_a < rb_b) {" << endl;
                break;

            case eBA:
                os << "if (ra_b < rb_a) {" << endl;
                break;

            case eI:    // fallthrough!
            case eF:   
                os << "if ((ra_a < rb_a) && (ra_b < rb_b)) {" << endl;
                break;
        
            case eX:   // X
                os << "if ((ra_a < rb_b) && (ra_b < rb_a)) {" << endl;
                break;
            default: assert(0);
            }
	    NextPos(os);
	    os << "}" << endl;
	    // end if
	    popAndPushNextPos(os);
	    NextWarrior(os);
            break;

        case eSne:
            switch (mod)
            {
            case eA:
                os << "if (ra_a != rb_a) {" << endl;
                break;
                
            case eB:
                os << "if (ra_b != rb_b) {" << endl;
                break;

            case eAB:
                os << "if (ra_a != rb_b) {" << endl;
                break;

            case eBA:
                os << "if (ra_b != rb_a) {" << endl;
                break;

            case eI:
                os << "if ((da->ins != db->ins) ||" << endl
		   << "    (ra_a != rb_a) || (ra_b != rb_b)) {" << endl;
                break;

            case eF:   // F
                os << "if ((ra_a != rb_a) || (ra_b != rb_b)) {" << endl;
                break;
                
            case eX:   // X
                os << "if ((ra_a != rb_b) || (ra_b != rb_a)) {" << endl;
                break;
            default: assert(0);
            }
	    NextPos(os);
	    os << "}" << endl;
	    // end if
	    popAndPushNextPos(os);
	    NextWarrior(os);
            break;

        default:
            cerr << "ops, Opcode " << op << " not allowed here" << endl;
            assert(0); // did i forget any opcodes?
	    }
	    os << "}" << endl;
	    os << "break;" << endl
	       << endl;
	} // for modifier
    } // for opcode

    os << "default:" << endl
       << "    assert(0);" << endl
       << "    break;" << endl
       << "}" << endl << endl;   
}
