#include "action.h"

#include <bits/shared_ptr_base.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <fstream>

#include "../scanner/Token.h"
#include "GrammarSymbol.h"
#include "GrammarRule.h"

using std::cerr;
using std::endl;
using std::shared_ptr;

Action::Action(char t, long s)
{
    type = t;
    state = s;
    reduction = NULL;
    forge_token = 0;
}

Action::~Action()
{
    reduction = NULL;
}

char Action::which() const
{
    return type;
}

long Action::getState() const
{
    return state;
}

shared_ptr<GrammarRule> Action::getReduction() const
{
    return reduction;
}

void Action::setReduction(shared_ptr<GrammarRule> r)
{
    reduction = r;
}

void Action::print() const
{
    switch(type)
    {
        case 's':   // shift
            cerr << "s " << state << "\t";
            break;
        case 'r':   // reduce
            cerr << "r\t";
            break;
        case 'g':   // goto
            cerr << "g " << state << "\t";
            break;
        case 'e':   // error
            cerr << "e " << state << "\t";
            break;
        case 'a':   // accept
            cerr << "acc\t";
            break;
        default:
            cerr << "FAIL\t";
            break;
    }
}

void Action::log(ofstream &out) const
{
    switch(type)
    {
        case 's':   // shift
            out << "s " << state << "\t";
            break;
        case 'r':   // reduce
            out << "r\t";
            break;
        case 'g':   // goto
            out << "g " << state << "\t";
            break;
        case 'e':   // error
            out << "e " << state << "\t";
            break;
        case 'a':   // accept
            out << "acc\t";
            break;
        default:
            out << "FAIL\t";
            break;
    }
}

/**
 *  surašo duomenis apie save į failą vieno stringo pavidalu
 *  formatas: typeState[,forge_tokenExpected|reduction_id]
 **/
void Action::output(ofstream &out) const
{   
    out << type << state;
    switch(type)
    {
        case 'r':
            out << "," << reduction->getId();
            break;
        case 'e':
            out << "," << forge_token << *expected;
            break;
        default:
            break;
    }
    out << "\t";
}

void Action::error(Token *token) const
{
    if (token->getId() == 0) {
    	throw std::runtime_error("Error at end of input file! ");
    }
    if (token->line != 0) {
        cerr << "Error on line " << token->line << ": " << *expected << " expected, got: " << token->getLexeme() << endl;
    }
}

void Action::setExpected(std::shared_ptr<GrammarSymbol> e)
{
    expected = e;
}

std::shared_ptr<GrammarSymbol> Action::getExpected() const
{
    return expected;
}

unsigned Action::getForge() const
{
    return forge_token;
}

void Action::setForge(unsigned forge)
{
    forge_token = forge;
}
