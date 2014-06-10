#include "SyntaxTree.h"
#include <fstream>

using std::ofstream;

const char *SyntaxTree::filename = NULL;

SyntaxTree::SyntaxTree()
{
    error = false;
    s_table = new SymbolTable();
}

SyntaxTree::~SyntaxTree()
{
    delete tree;
    delete s_table;
}

string SyntaxTree::asXml() const
{
    ostringstream oss;
    return tree->asXml(oss, 0).str();
}

SymbolTable *SyntaxTree::getSymbolTable() const
{
    return s_table;
}

void SyntaxTree::setTree(Node *t)
{
    tree = t;
    code = tree->getCode();
}

void SyntaxTree::setErrorFlag()
{
    error = true;
}

const char *SyntaxTree::getFileName()
{
    return filename;
}

void SyntaxTree::setFileName(const char *fname)
{
    filename = fname;
}

bool SyntaxTree::hasSemanticErrors() const
{
    return error;
}

void SyntaxTree::outputCode(ostream &of) const
{
    for (unsigned i = 0; i < code.size(); i++)
        code[i]->output(of);
}

void SyntaxTree::printTables() const
{
    s_table->printTable();
}

void SyntaxTree::logTables()
{
}

void SyntaxTree::logCode()
{
    ofstream outcode;
    outcode.open("logs/int_code");
    if (outcode.is_open())
    {
        outputCode(outcode);
        outcode.close();
    }
    else
        cerr << "Couldn't create file \"logs/int_code\"!\n";
}

vector<Quadruple *> SyntaxTree::getCode() const
{
    return code;
}
