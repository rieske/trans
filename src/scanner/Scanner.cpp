#include "Scanner.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

#define EVER ;;

const int MAXLINE = 1000;
const char *lex_source = "scanner.lex";

bool Scanner::log = false;
FILE *Scanner::logfile = NULL;

Scanner::Scanner(const char *src)
{
    if ( NULL == (source = fopen(src, "r")) )
    {   // nepavyko atidaryti sorso skaitymui
        fprintf (stderr, "Error: could not open source file for reading. Filename: %s\n", src);
        buffer = NULL;
        return;
    }
    buffer = new char[MAXLINE];
    memset(buffer, EOF, MAXLINE);
    b_index = 0;
    init(lex_source);
}

Scanner::~Scanner()
{
    if (source != NULL)
        fclose(source);
    delete [] buffer;
    map<string, State*>::iterator it;
    for (it = m_state.begin(); it != m_state.end(); ++it)
        delete it->second;
    if (logfile != stderr && log == true)
        fclose(logfile);
}

int Scanner::init(const char *conf)
{
    if ( NULL == (lex_src = fopen(conf, "r")) )
    {   // nepavyko atidaryti sorso skaitymui
        fprintf (stderr, "Error: could not open lex file for reading. Filename: %s\n", conf);
        exit(EXIT_FAILURE);
    }
    char *buffer = new char[MAXLINE];

    string name;
    string id;
    unsigned int next_kw = 1;
    while (NULL != (fgets(buffer, MAXLINE, lex_src)) )
    {
        int i = 1;
        if (buffer[0] == ':')       // nauja būsena
        {
            State *state = new State();
            name = "";
            id = "";

            for (; ( (i < MAXLINE) && (!isspace(buffer[i])) ); i++)        // tempiam jos pavadinimą
                name += buffer[i];
            //printf("%s\n", name.c_str());
            switch (name[0])
            {
                case '%':               // lyginsim su keywordais
                    name = name.substr(1, name.size());
                    state->setKeywordCheck();
                    break;
                case '"':               // tarpus dėsim į leksemą
                    name = name.substr(1, name.size());
                    state->setIgnoreSpaces();
                    break;
                case '!':               // negrąžinsim leksemos iš viso
                    name = name.substr(1, name.size());
                    state->setComment();
                    break;
                case '/':               // eol komentaras
                    name = name.substr(1, name.size());
                    state->setEolComment();
                    break;
                default:
                    break;
            }
            if (start_state.empty())
                start_state = name;
            for (; ( (i < MAXLINE) && (buffer[i] != '\n') ); i++)               // gal būt tai galutinė leksemos būsena
                if (!isspace(buffer[i]) )                                       // jei taip tai saugom leksemos kodą
                    id += buffer[i];
            state->setId(id);
            m_state.insert( make_pair(name, state) );
            continue;
        }
        else if (buffer[0] == '@')       // šuoliai į kitas būsenas
        {
            string next_state_name = "";
            string next_state = "";

            for (i = 1; ( (i < MAXLINE) && (!isspace(buffer[i])) ); i++)        // naujos būsenos pavadinimas
                next_state_name += buffer[i];
            for (; ( (i < MAXLINE) && (buffer[i] != '\n') ); i++)               // kaip ten patekt
                if (!isspace(buffer[i]))
                    next_state += buffer[i]; 
            m_state.at(name)->add_state(next_state_name, next_state);
            continue;
        }
        else if (buffer[0] == '%')                                              // keywordas
        {
            string keyword = "";
            for (i = 1; ( (i < MAXLINE) && (!isspace(buffer[i])) ); i++)
                keyword += buffer[i];
            m_keywords.insert( make_pair(keyword, next_kw++) );
            keyword = "";
            for (; ( (i < MAXLINE) && (buffer[i] != '\n') ); i++)               // galima leisti ir daugiau keyword'ų toje pačioje eilutėje
            {
                while ( !isspace(buffer[i]) )
                    keyword += buffer[i++]; 
                if (!keyword.empty())
                {
                    m_keywords.insert( make_pair(keyword, next_kw++) );
                    --i;
                }
                keyword = "";
            }
        }
    }
    final_state = name;
    delete [] buffer;
    if (log)
        print_states();
    line = 0;
    fclose(lex_src);
    return 0;
}

void Scanner::print_states() const
{
    map<string, State*>::const_iterator it;           // išspausdinam sudarytą baigtinį automatą
    for (it = m_state.begin(); it != m_state.end(); ++it)
    {
        fprintf(logfile, "%s:%d\n", it->first.c_str(), it->second->getId());
        it->second->listing(logfile);
    }
    
    if (!m_keywords.empty())
    {
        fprintf(logfile, "\nKeywords:\n");
        map<string, unsigned int>::const_iterator kw_it;        // išspausdinam raktinių žodžių lentelę
        for (kw_it = m_keywords.begin(); kw_it != m_keywords.end(); ++kw_it)
            fprintf(logfile, "%s:%d\n", kw_it->first.c_str(), kw_it->second);
    }
}

Token *Scanner::scan()
{
    if (source == NULL)
        return NULL;

    token = "";
    current_state = m_state.at(start_state);
    current_state_str = start_state;
    for (EVER)
    {
        while (buffer[b_index] != EOF)  // kol buferis netuščias, dirbam su juo
        {
            current_state = update_state(buffer[b_index]);
            if (current_state == m_state.at(final_state))           // galutinė būsena - turim naują leksemą
            {
                Token *ret = new Token;
                if ( (m_state.at(current_state_str)->need_lookup()) && (m_keywords.find(token) != m_keywords.end()) )   // tai keywordas
                    ret->type = m_keywords.at(token);
                else
                    ret->type = m_state.at(current_state_str)->getId();
                ret->value = token;
                ret->line = line;
                token = "";
                current_state = m_state.at(start_state);
                current_state_str = start_state;
                return ret;
            } 
            ++b_index;
        }
        if (buffer[b_index] == EOF)     // jei buferis tuščias
        {
            line++;
            b_index = 0;
            memset(buffer, EOF, MAXLINE);
            if ( NULL == (fgets(buffer, MAXLINE, source)) )
            {
                if (token != "")
                    fprintf(stderr, "Line %d: error unindentified token: '%s'\n", (int)line, token.c_str());
                return NULL;    // nepavyko gauti eilutės
            }
        }
    }
}

State *Scanner::update_state(char c)
{
    string next = current_state->get_next(c);

    if (next == "CURRENT")
    {
        token += c;
        return current_state;
    }
    else if (next == "NONE")
    {
        token = "";
        return m_state.at(start_state);
    }
    else if (next == "EOL_COMMENT")
    {
        if (c != '\n')
            return current_state; 
        else
            token = "";
        return m_state.at(start_state);
    }
    else if (next == "ERROR")
    {
        if (c != '\n')
            token += c;
        fprintf(stderr, "Line %d: error unindentified token: '%s'\n", (int)line, token.c_str());
        if (log)
            fprintf(logfile, "State: %s\n", current_state_str.c_str());
        return m_state.at(start_state);
    }
    else
    {
        State *ret = m_state.at(start_state);
        
        try
        {
            ret = m_state.at(next);
        }
        catch (std::out_of_range& err)
        {
            fprintf(stderr, "Scanner error: can't reach final state from state '%s'!\n\tCheck your '%s' file!\n", next.c_str(), lex_source); 
            exit(1);
        }
        if ( next != final_state )
        {
            current_state_str = next;
            token += c;
        }
        if (current_state->is_comment())
        {
            token = "";
            b_index--;
            return m_state.at(start_state);
        }
        return ret;
    }
    return m_state.at(start_state);
}

void Scanner::set_logging(const char *lf)
{
    log = true;
    string logpath = "logs/";
    logpath += lf;
    if ( NULL == (logfile = fopen(logpath.c_str(), "w")))
        logfile = stderr;
}
