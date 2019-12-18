#pragma once

#include <windows.h>

/*!
 * \brief Отображение текста ошибки по коду ::GetLastError()
 * \param functionName Имя функции, вызвавшей ошибку
 */
void ErrorMsg(const char *className, const char *methodName, const char *functionName);

class CFile
{
public:
    CFile();
    ~CFile();
    bool Open(const char *fileName);
    bool Close();
    HANDLE FileHandle() const;
    LARGE_INTEGER FileSize() const;
private:
    HANDLE m_fileHandle;
    LARGE_INTEGER m_fileSize;
};
