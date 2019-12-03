#include "clogreader.h"
#include <cstring>
#include <cstdio>

CLogReader::CLogReader()
{

}

bool CLogReader::SetFilter(const char *filter)
{
    char buffer[MAX_PATH];
    memset(buffer, '\0', MAX_PATH);

    //Предыдущий символ в паттерне
    char c_old = '*';
    // Текущий символ в паттерне
    char c;
    // Индекс в паттерне
    int i = 0;
    // Индекс текущего токена
    int j = 0;

    while (filter[i] != '\0' && (i < MAX_PATH))
    {
        c = filter[i];
        if(c == '*'){
            if(c_old == '*') {
                i++;
                continue;
            }
            buffer[j] = c;
        } else {
            if(regex_utility::isAlphaNumeric(c) && (i == 0)) {
                buffer[j] = '^';
                j++;
            }
            buffer[j] = c;
        }


        i++;
        j++;
        c_old = c;
    }


    if(regex_utility::isAlphaNumeric(buffer[j-1]))
        buffer[j] = '$';

    return rgx.compile(buffer);
}

bool CLogReader::GetNextLine(const char *buf, const int bufsize)
{
    int match_idx = rgx.search(buf);
    printf("%40s:\tmatch at idx %d.\n", buf, match_idx);
}
