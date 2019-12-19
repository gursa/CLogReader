#include "cregex.h"
#include <cstdio>
#include <windows.h>

CRegex::CRegex(bool isCaseSensitive)
    : m_searchFilter(NULL),
      m_caseSensitive(isCaseSensitive)
{

}

CRegex::~CRegex()
{
    FreeFilter();
}

bool CRegex::SetFilter(const char *filter)
{
    if(NULL == filter)
        return false;

    size_t filterLength = strnlen_s( filter, MAX_FILTER_SIZE );
    if(0 == filterLength)
        return false;
    m_searchFilter = (char*) malloc( filterLength + 1 );
    if(NULL == m_searchFilter)
        return false;
    if (0 != strcpy_s( m_searchFilter, filterLength + 1, filter ))
        return false;

    bool vw = false;
    for (char *ps(m_searchFilter), *pd(m_searchFilter);;)
    {
        bool w(false);
        while (*ps == '*')
        {
            w = true;
            *pd = *ps;
            ++ps;
        }
        if (!w)
        {
            *pd = *ps;
            if (*ps == '\0')
                break;
            ++ps;
        }
        else
        {
            vw = true;
        }
        ++pd;
    }

    return true;
}

void CRegex::FreeFilter()
{
    if(m_searchFilter)
        free(m_searchFilter);
    m_searchFilter = NULL;
}

char* CRegex::GetFilter() const
{
    return m_searchFilter;
}

bool CRegex::GetCaseSensitive() const
{
    return m_caseSensitive;
}

bool CRegex::Match(char * __restrict text, CRegex& regex, char altTerminator)
{
    if(NULL == text)
        return false;

    bool matchResult = true;
    char* afterLastFilter = NULL;
    char* afterLastText = NULL;
    char symbolText;
    char symbolFilter;

    char * __restrict filter = regex.GetFilter();
    bool сaseSensitive = regex.GetCaseSensitive();

    while( 1 )
    {
        symbolText = *text;
        symbolFilter = *filter;

        if( !symbolText || symbolText == altTerminator )
        {
            if( !symbolFilter || symbolFilter == altTerminator )
            {
                break;
            }
            else if( symbolFilter == '*' )
            {
                filter++;
                continue;
            }

            else if( afterLastText )
            {
                if( !( *afterLastText ) || *afterLastText == altTerminator )
                {
                    matchResult = false;
                    break;
                }

                text = afterLastText++;
                filter = afterLastFilter;
                continue;
            }

            matchResult = false;
            break;
        }
        else
        {
            if( !сaseSensitive )
            {
                if (symbolText >= 'A' && symbolText <= 'Z')
                    symbolText += ('a' - 'A');

                if (symbolFilter >= 'A' && symbolFilter <= 'Z')
                    symbolFilter += ('a' - 'A');
            }

            if( !IsEqual(symbolText, symbolFilter) )
            {
                if( symbolFilter == '*' )
                {
                    afterLastFilter = ++filter;
                    afterLastText = text;
                    symbolFilter = *filter;

                    if( !symbolFilter || symbolFilter == altTerminator )
                        break;
                    continue;
                }
                else if ( afterLastFilter )
                {
                    if( afterLastFilter != filter )
                    {
                        filter = afterLastFilter;
                        symbolFilter = *filter;

                        if( !сaseSensitive && symbolFilter >= 'A' && symbolFilter <= 'Z' )
                            symbolFilter += ('a' - 'A');

                        if( IsEqual(symbolText, symbolFilter) )
                            filter++;
                    }
                    text++;
                    continue;
                }
                else
                {
                    matchResult = false;
                    break;
                }
            }
        }
        text++;
        filter++;
    }
    return matchResult;
}

bool CRegex::IsUndefined(char symbol)
{
    return symbol == '?';
}

bool CRegex::IsEqual(char lSymbol, char rSymbol)
{
    return lSymbol == rSymbol || IsUndefined(rSymbol);
}
