#include <iostream>
#include "LR1Parser.h"
#include "semantic_analyzer/syntax_tree.h"

#define EVER ;;

using std::cerr;
using std::cout;
using std::endl;

bool LR1Parser::log = false;
ofstream LR1Parser::logfile;

LR1Parser::LR1Parser()
{
    output = NULL;
    token = NULL;
    next_token = NULL;
    syntax_tree = NULL;
    custom_grammar = false;
    p_table = new Parsing_table();
    if (log)
        configure_logging();
    if (log)
    {
        p_table->log(logfile);
        p_table->output_html();
    }
    parsing_stack.push(0);
}

LR1Parser::LR1Parser(string gra)
{
    output = NULL;
    token = NULL;
    next_token = NULL;
    syntax_tree = NULL;
    custom_grammar = true;
    p_table = new Parsing_table(gra.c_str());
    if (log)
        configure_logging();
    if (log)
    {
        p_table->log(logfile);
        p_table->output_html();
        p_table->output_table();
    }
    parsing_stack.push(0);
}

LR1Parser::~LR1Parser()
{
    delete p_table;
    if (logfile.is_open())
        logfile.close();
    delete syntax_tree;
}


int LR1Parser::parse(const char *src)
{
    scanner = new Scanner(src);
    if (syntax_tree != NULL)
        delete syntax_tree;
    syntax_tree = new SyntaxTree();
    syntax_tree->setFileName(src);
    current_scope = syntax_tree->getSymbolTable();
    success = true;
    can_forge = true;
    token = scanner->scan();
    Action *action = NULL;
    for (EVER)
    {
        long top = parsing_stack.top();
        action = p_table->action(top, token->type);
        if (action != NULL)
        {
            switch(action->which())
            {
                case 's':
                    shift(action);
                    continue;
                case 'a':       // accept
                    if (success)
                    {
                        syntax_tree->setTree(syntax_stack.top());
                        if (log)
                        {
                            log_syntax_tree();
                            syntax_tree->printTables();
                            syntax_tree->logCode();
                        }
                        delete scanner;
                        return 0;
                    }
                    delete scanner;
                    return 1;
                case 'r':
                    reduce(action);
                    action = NULL;
                    continue;
                case 'e':
                    success = false;
                    error(action);
                    action = NULL;
                    continue;
                default:
                    action->error(token);
                    cerr << "error" << endl;
                    if (output != NULL)
                    {
                        output->close();
                        delete output;
                    }
                    delete scanner;
                    return 1;
            }
        }
        else
            fail("NULL entry in action table!");
    }
    if (output != NULL)
    {
        output->close();
        delete output;
    }
    delete scanner;
    return 1;
}

SyntaxTree *LR1Parser::getSyntaxTree() const
{
    return syntax_tree;
}

void LR1Parser::set_logging(const char *lf)
{
    log = true;
    string logpath = "logs/";
    logpath += lf;
    logfile.open(logpath.c_str());
    if (!logfile.is_open())
    {
        cerr << "Unable to open log file for writing! Filename: " << lf << endl;
        log = false;
    }
}

void LR1Parser::configure_logging()
{
    const char *outfile = "logs/parser.out";
    output = new ofstream;
    output->open(outfile);
    if (!output->is_open())
    {
        cerr << "Unable to create parser output file! Filename: " << outfile << endl;
        delete output;
        output = NULL;
    }
}

void LR1Parser::shift(Action *action)
{
    if (log)
    {
        *output << "Stack: " 
                << parsing_stack.top() 
                << "\tpush " 
                << action->getState() 
                << "\t\tlookahead: " 
                << ((token != NULL) ? token->value : "$end$") 
                << endl;
    }
    parsing_stack.push(action->getState());
    if (success)
    {
        TerminalNode *t_node = new TerminalNode(p_table->getTerminalById(token->type), token);
        adjustScope();
        line = token->line;
        syntax_tree->setLine(line);
        syntax_stack.push(t_node);
    }
    if (next_token != NULL)
    {
        token = next_token;
        next_token = NULL;
    }
    else
    {
        token = scanner->scan();
        can_forge = true;
    }
    if (token == NULL)
    {
        token = new Token;
        token->type = (unsigned)(-1);
    }
    action = NULL;
}

void LR1Parser::reduce(Action *action)
{
    Rule *reduction = NULL;
    Action *gt = NULL;
    reduction = action->getReduction();
    vector<Node *> right_side;
    if (reduction != NULL)
    {
        if (log)
            reduction->log(*output);
    }
    else
        fail("NULL reduction found!");
    for (unsigned i = reduction->getRight()->size(); i > 0; i--)
    {
        if (log)
        {
            *output << "Stack: " 
                    << parsing_stack.top() 
                    << "\tpop " 
                    << parsing_stack.top() 
                    << "\t\t";
            if (i > 1)
                *output << endl;
        }
        if (success)
        {   
            right_side.push_back(syntax_stack.top());
            syntax_stack.pop();
        }
        parsing_stack.pop();
    }
    gt = p_table->go_to(parsing_stack.top(), *reduction->getLeft());
    if (gt != NULL)
    {
        if (log)
        {
            *output << "Stack: " 
                    << parsing_stack.top() 
                    << "\tpush " 
                    << gt->getState() 
                    << "\t\tlookahead: " 
                    << ((token != NULL) ? token->value : "$end$") 
                    << endl;
        }
        if (success)
        {
            mknode(*reduction->getLeft(), right_side, reduction->rightStr());
        }
        parsing_stack.push(gt->getState());
        action = NULL;
    }
    else
        fail("NULL goto entry found!");
}

void LR1Parser::error(Action *action)
{
    if (action->error(token))
    {
        if (action->getForge() != 0 && can_forge)
        {
            next_token = token;
            token = new Token;
            token->type = action->getForge();
            if (log)
            {
                cerr << "Inserting " << action->getExpected() << " into input stream.\n";
            }
            can_forge = false;
        }
        else
        {
            parsing_stack.push(action->getState());
            token = scanner->scan();
            if (log)
            {
                cerr << "Stack: " 
                     << parsing_stack.top() 
                     << "\tpush " 
                     << action->getState() 
                     << "\t\tlookahead: " 
                     << ((token != NULL) ? token->value : "$end$") 
                     << endl;
            }
        }
        if (token == NULL)
        {
            token = new Token;
            token->type = (unsigned)(-1);
        }
    }
    else
        fail("Parsing failed!");
}

void LR1Parser::mknode(string left, vector<Node *> children, string reduction)
{
    Node *n_node = NULL;
    if (custom_grammar)
        n_node = new NonterminalNode(left, children, reduction);
    else
    {
        if (left == "<u_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<m_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<add_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<s_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<ml_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<eq_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<a_op>")
            n_node = new CarrierNode(left, children);
        else if (left == "<term>")
            n_node = new TermNode(left, children, reduction, current_scope, line);
        else if (left == "<postfix_expr>")
            n_node = new PostfixExprNode(left, children, reduction, current_scope, line);
        else if (left == "<u_expr>")
            n_node = new UExprNode(left, children, reduction, current_scope, line);
        else if (left == "<cast_expr>")
            n_node = new CastExprNode(left, children, reduction, current_scope, line);
        else if (left == "<factor>")
            n_node = new FactorNode(left, children, reduction, current_scope, line);
        else if (left == "<add_expr>")
            n_node = new AddExprNode(left, children, reduction, current_scope, line);
        else if (left == "<s_expr>")
            n_node = new SExprNode(left, children, reduction, current_scope, line);
        else if (left == "<ml_expr>")
            n_node = new MLExprNode(left, children, reduction, current_scope, line);
        else if (left == "<eq_expr>")
            n_node = new EQExprNode(left, children, reduction, current_scope, line);
        else if (left == "<and_expr>")
            n_node = new AndExprNode(left, children, reduction, current_scope, line);
        else if (left == "<xor_expr>")
            n_node = new XorExprNode(left, children, reduction, current_scope, line);
        else if (left == "<or_expr>")
            n_node = new OrExprNode(left, children, reduction, current_scope, line);
        else if (left == "<log_and_expr>")
            n_node = new LogAndExprNode(left, children, reduction, current_scope, line);
        else if (left == "<log_expr>")
            n_node = new LogExprNode(left, children, reduction, current_scope, line);
        else if (left == "<a_expressions>")
            n_node = new AExpressionsNode(left, children, reduction);
        else if (left == "<a_expr>")
            n_node = new AExprNode(left, children, reduction, current_scope, line);
        else if (left == "<expr>")
            n_node = new ExprNode(left, children, reduction, current_scope, line);
        else if (left == "<jmp_stmt>")
            n_node = new JmpStmtNode(left, children, reduction, current_scope, line);
        else if (left == "<io_stmt>")
            n_node = new IOStmtNode(left, children, reduction, current_scope, line);
        else if (left == "<loop_hdr>")
            n_node = new LoopHdrNode(left, children, reduction, current_scope, line);
        else if (left == "<unmatched>")
            n_node = new UnmatchedNode(left, children, reduction, current_scope, line);
        else if (left == "<matched>")
            n_node = new MatchedNode(left, children, reduction, current_scope, line);
        else if (left == "<stmt>")
            n_node = new CarrierNode(left, children);
        else if (left == "<statements>")
            n_node = new CarrierNode(left, children);
        else if (left == "<param_decl>")
            n_node = new ParamDeclNode(left, children, reduction, current_scope, line);
        else if (left == "<param_list>")
            n_node = new ParamListNode(left, children, reduction);
        else if (left == "<dir_decl>")
        {
            n_node = new DirDeclNode(left, children, reduction, current_scope, line);
            params = ((DirDeclNode *)n_node)->getParams();
        }
        else if (left == "<ptr>")
            n_node = new PtrNode(left, children, reduction);
        else if (left == "<block>")
            n_node = new BlockNode(left, children);
        else if (left == "<decl>")
            n_node = new DeclNode(left, children, reduction);
        else if (left == "<decls>")
            n_node = new DeclsNode(left, children, reduction);
        else if (left == "<type_spec>")
            n_node = new CarrierNode(left, children);
        else if (left == "<var_decl>")
            n_node = new VarDeclNode(left, children, reduction, current_scope, line);
        else if (left == "<func_decl>")
            n_node = new FuncDeclNode(left, children, reduction, current_scope, line);
        else if (left == "<var_decls>")
            n_node = new CarrierNode(left, children);
        else if (left == "<func_decls>")
            n_node = new CarrierNode(left, children);
        else if (left == "<program>")
            n_node = new CarrierNode(left, children);
        else
        {
            cerr << "Error! Syntax node matching nonterminal " << left << "found!\n";
            return;
        }
    }
    if (true == n_node->getErrorFlag())
    {
        syntax_tree->setErrorFlag();
    }
    syntax_stack.push(n_node);
}

void LR1Parser::adjustScope()
{
    if (!custom_grammar)
    {
        if (token->value == "{")
        {
            current_scope = current_scope->newScope();
            if (params.size())
            {
                for (unsigned i = 0; i < params.size(); i++)
                {   
                    current_scope->insertParam(params[i]->getPlace()->getName(), params[i]->getPlace()->getBasicType(), 
                                            params[i]->getPlace()->getExtendedType(), line);
                }
                params.clear();
            }
        }
        else if (token->value == "}")
        {
            current_scope = current_scope->getOuterScope();
        }
    }
}

void LR1Parser::fail(string err)
{
    if (output != NULL)
    {
        output->close();
        delete output;
    }
    cerr << "Error! " << err << endl;
    exit(1);
}

void LR1Parser::log_syntax_tree() const
{
    const char *outfile = "logs/syntax_tree.xml";
    ofstream xmlfile;
    xmlfile.open(outfile);
    if (!xmlfile.is_open())
    {
        cerr << "Unable to create syntax tree xml file! Filename: " << outfile << endl;
        return;
    }
    xmlfile << syntax_tree->asXml();
}
