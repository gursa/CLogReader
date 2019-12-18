#include "cfile.h"

#include <cstdio>

void ErrorMsg(const char *className, const char *methodName, const char *functionName)
{
    if((NULL == className) && (NULL == functionName)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! className = %p, methodName = %p, functionName = %p\n",
                __FUNCTION__, (void*)className, (void*)methodName, (void*)functionName);
        return;
    }

    // Получение описания последней ошибки по ее коду
    LPSTR messageBuffer;
    DWORD errorCode = GetLastError();

    if(!FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                (LPSTR) &messageBuffer,
                0,
                NULL )) {
        fprintf(stderr, "\n[ERROR] [%s()] %s() failed with error %lu\n",
                methodName, functionName, errorCode);
        fprintf(stderr, "[ERROR] [%s()] %s() failed with error %lu\n",
                methodName, "FormatMessageA", GetLastError());
    } else {
        fprintf(stderr, "\n[ERROR] [%s()] %s() failed with error %lu: %s\n",
                methodName, functionName, errorCode, messageBuffer);
    }

    LocalFree(messageBuffer);
}

CFile::CFile()
    : m_fileHandle(INVALID_HANDLE_VALUE)
{
    m_fileSize.QuadPart = 0;
}

CFile::~CFile()
{
    Close();
}

bool CFile::Open(const char *fileName)
{
    if(NULL == fileName){
        fprintf(stderr, "\n[ERROR] CFile::%s(): Error in args! fileName = %p\n", __FUNCTION__, (void*)fileName);
        return false;
    }

    if(false == Close())
        return false;

    m_fileHandle = ::CreateFileA(
            fileName,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);
    if(INVALID_HANDLE_VALUE == m_fileHandle) {
        ErrorMsg("CFile", __FUNCTION__, "::CreateFile");
        return false;
    }

    DWORD fileSizeHigh = 0;
    m_fileSize.LowPart = ::GetFileSize(m_fileHandle, &fileSizeHigh);
    m_fileSize.HighPart = (LONG)fileSizeHigh;
    if(INVALID_FILE_SIZE == m_fileSize.LowPart) {
        ErrorMsg("CFile", __FUNCTION__, "::GetFileSize");
        Close();
        return false;
    }

    fprintf(stdout, "File size: %lld\n", m_fileSize.QuadPart);

    return true;
}

bool CFile::Close()
{
    m_fileSize.QuadPart = 0;

    if(INVALID_HANDLE_VALUE != m_fileHandle) {
        if(0 == ::CloseHandle(m_fileHandle)) {
            ErrorMsg("CFile", __FUNCTION__, "::CloseHandle");
            return false;
        }
    }
    return true;
}

HANDLE CFile::FileHandle() const
{
    return m_fileHandle;
}

LARGE_INTEGER CFile::FileSize() const
{
    return m_fileSize;
}


