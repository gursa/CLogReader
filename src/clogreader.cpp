#include "clogreader.h"
#include <cstdio>

CLogReader::CLogReader(bool isCaseSensitive)
    : m_fileMapper(),
      m_regex(isCaseSensitive)
{
}

CLogReader::~CLogReader(void)
{
    m_fileMapper.DestroyMappingObj();
}

bool CLogReader::Open(const char *filename)
{
    if(NULL == filename){
        fprintf(stderr, "\n[ERROR] %s(): Error in args! filename = %p\n", __FUNCTION__, (void*)filename);
        return false;
    }

    Close();

    if(false == m_fileMapper.CreateMappingObj(filename))
        return false;

    return true;
}

void CLogReader::Close()
{    
    m_fileMapper.DestroyMappingObj();
    m_regex.FreeFilter();
}

bool CLogReader::SetFilter(const char *filter)
{
    if(NULL == filter){
        fprintf(stderr, "\n[ERROR] %s(): Error in args! filter = %p\n", __FUNCTION__, (void*)filter);
        Close();
        return false;
    }

    if(false == m_regex.SetFilter(filter)) {
        Close();
        return false;
    }
    return true;
}

bool CLogReader::GetNextLine(char *buf, const int bufsize)
{
    if(NULL == buf || (bufsize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buf = %p, bufferSize = %d\n", __FUNCTION__, (void*)buf, bufsize);
        return false;
    }

    while(1) {
        // Получаем строку из смапированного файла
        if(false == m_fileMapper.GetString(buf, bufsize))
            break;

        if(CRegex::Match( buf, m_regex ))
            return true;
    }
    return false;
}
