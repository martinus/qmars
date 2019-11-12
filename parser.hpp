#ifndef PARSER_HPP
#define PARSER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <map>
#include "warrior.hpp"
#include "code.hpp"
using namespace std;

static const int emptySy = -1;
static const int orgSy = -2;
static const int startSy = -3;

class Parser
{
public:
    Warrior* parse(char* f, ulong coreSize)
    {
	ifstream file(f);
        ch=' ';
        if (!file) return(0);

        w = new Warrior;
        w->start=0;
        lineNr=0;
        while (ReadLine(coreSize, file));
        file.close();
        return w;
    }

    Parser::Parser()
    {
        str2sy["DAT"]=eDat;
        str2sy["NOP"]=eNop;
        str2sy["SPL"]=eSpl;
        str2sy["JMP"]=eJmp;
        str2sy["MOV"]=eMov;
        str2sy["ADD"]=eAdd;
        str2sy["SUB"]=eSub;
        str2sy["MUL"]=eMul;
        str2sy["DIV"]=eDiv;
        str2sy["MOD"]=eMod;
        str2sy["JMZ"]=eJmz;
        str2sy["JMN"]=eJmn;
        str2sy["DJN"]=eDjn;
        str2sy["SLT"]=eSlt;
        str2sy["CMP"]=eCmp;
        str2sy["SEQ"]=eCmp; // SEQ==CMP
        str2sy["SNE"]=eSne;

        str2sy["I"]=eI;
        str2sy["A"]=eA;
        str2sy["B"]=eB;
        str2sy["AB"]=eAB;
        str2sy["BA"]=eBA;
        str2sy["F"]=eF;
        str2sy["X"]=eX;

        str2sy["#"]=eIm;
        str2sy["$"]=eDir;
        str2sy["@"]=eInB;
        str2sy["<"]=ePreDecInB;
        str2sy[">"]=ePostIncInB;
        str2sy["*"]=eInA;
        str2sy["{"]=ePreDecInA;
        str2sy["}"]=ePostIncInA;

        str2sy["ORG"]=orgSy;
        str2sy[""]=emptySy;
    }
    
private:
    int lineNr;
    Warrior* w;
    char ch;
    map<string, int> str2sy;

    bool ReadLine(ulong coreSize, ifstream& file)
    {
        string s = NewSy(file);
        switch(str2sy[s])
        {
        case emptySy:
            return(false);
            break;
            
        case orgSy:
            str2sy[NewSy(file)]=startSy;
            break;

        case startSy: // fallthrough!
            w->start = lineNr;
	    s=NewSy(file);
                
        default:
            EOpCode op    = static_cast<EOpCode>(str2sy[s]);
            EModifier mod = static_cast<EModifier>(str2sy[NewSy(file)]);
            EAddrMode ma  = static_cast<EAddrMode>(str2sy[NewSy(file)]);
            int valA      = atoi(NewSy(file).c_str());
            EAddrMode mb  = static_cast<EAddrMode>(str2sy[NewSy(file)]);
            int valB      = atoi(NewSy(file).c_str());
            if (valA<0) valA += coreSize;
            if (valB<0) valB += coreSize;
            w->code.push_back(Code(op, mod, ma, valA, mb, valB));
            lineNr++;
	    break;
        }
        return(true);
    }

    string NewSy(ifstream& file)
    {
        string s = "";
        while (overreadable(ch))
        {
            if ((ch=file.get()) == EOF) return(s);
        }

        do
        {
            s += toupper(ch);
            file.get(ch);
        } while (!overreadable(ch));
        return s;
    }
    
    bool overreadable(char ch)
    {
        return((ch==' ') || (ch=='\n')
               || (ch=='\t') || (ch=='.') || (ch==','));
    }
};

#endif
