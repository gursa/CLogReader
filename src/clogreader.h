#ifndef CLOGREADER_H
#define CLOGREADER_H

#include <regex.h>

class CLogReader
{
public:
    CLogReader();
    bool    SetFilter(const char *filter);   // установка фильтра строк, false - ошибка
    bool    GetNextLine(const
                        char *buf,           // запрос очередной найденной строки,
                           const int bufsize);  // buf - буфер, bufsize - максимальная длина
                                                // false - конец файла или ошибка


    regex::CRegex rgx;
};

#endif // CLOGREADER_H
