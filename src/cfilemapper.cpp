#include "cfilemapper.h"
#include <cstdio>

CFileMapper::CFileMapper()
    : CFile(),
      m_fileMapping(NULL),
      m_mapingRegion(NULL),
      m_regionOffset(0),
      m_regionSize(0)
{
    GetSystemInfo(&m_systemInfo);
    m_fileTail.QuadPart = 0;
    m_fileOffset.QuadPart = 0;
}

CFileMapper::~CFileMapper()
{
    DestroyMappingObj();
}

bool CFileMapper::CreateMappingObj(const char *fileName)
{
    if(NULL == fileName){
        fprintf(stderr, "\n[ERROR] %s(): Error in args! fileName = %p\n", __FUNCTION__, (void*)fileName);
        return false;
    }

    if(false == DestroyMappingObj())
        return false;

    if(false == Open(fileName))
        return false;

    fprintf(stdout, "[%s] File size: %lld\n", __FUNCTION__, FileSize().QuadPart);
    m_fileTail = FileSize();

    m_fileMapping = ::CreateFileMapping(
        FileHandle(),
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);


    if(NULL == m_fileMapping) {
        ErrorMsg(__FUNCTION__, "::CreateFileMapping");
        DestroyMappingObj();
        return false;
    }

    return true;
}

bool CFileMapper::DestroyMappingObj()
{
    m_fileTail.QuadPart = 0;
    UnmappingRegion();

    if(m_fileMapping && (0 == ::CloseHandle(m_fileMapping))) {
        ErrorMsg(__FUNCTION__, "::CloseHandle");
    }
    m_fileMapping = NULL;

    return Close();
}

bool CFileMapper::GetString(char *buffer, const int bufferSize)
{
    if(NULL == buffer || (bufferSize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buffer = %p, bufferSize = %d\n", __FUNCTION__, (void*)buffer, bufferSize);
        return false;
    }
    SecureZeroMemory(buffer, bufferSize*sizeof (char));
    SecureZeroMemory(m_fileLine, sizeof (m_fileLine));

    // Если мапирование файла еще не производилось, то сейчас самое время
    if(NULL == m_mapingRegion) {
        if(false == MappingNextRegion()) {
            return false;
        }
        m_regionOffset = 0;
    }

    int add_index = 0;
    // Пробежимся по региону и найдем строку
    if(true == AddSymbolToBuffer(buffer, bufferSize, add_index)) {
        return true;
    }

    // Если найти строку в первом куске не получилось, освободим регион
    if(false == UnmappingRegion()) {
        SecureZeroMemory(buffer, bufferSize*sizeof (char));
        return false;
    }

    // Смапируем еще кусочек
    if(false == MappingNextRegion()) {
        SecureZeroMemory(buffer, bufferSize*sizeof (char));
        return false;
    }

    m_regionOffset = 0;
    // И пройдемся по нему
    if(true == AddSymbolToBuffer(buffer, bufferSize, add_index)) {
        return true;
    }

    // Если в и во второй раз на не попался символ переноса строки, у нас проблема. Выходим с ошибкой
    if(false == UnmappingRegion()) {
        return false;
    }

    return false;
}

bool CFileMapper::MappingNextRegion()
{
    double percentComplete = (double)m_fileOffset.QuadPart / (double)FileSize().QuadPart * 100.0;
    fprintf(stdout, "File processing: %3.3f %% [%lld / %lld]\r", percentComplete, m_fileOffset.QuadPart, FileSize().QuadPart);

    if(!m_fileMapping)
        return false;

    // Вычисляем сколько байт нужно смапировать
    m_regionSize = (m_fileTail.QuadPart < m_systemInfo.dwAllocationGranularity) ?
                m_fileTail.LowPart :
                m_systemInfo.dwAllocationGranularity;
    if(m_regionSize == 0)
        return false;

    m_mapingRegion = (char*)MapViewOfFile(
                m_fileMapping,
                FILE_MAP_READ,
                (DWORD)m_fileOffset.HighPart, // в файле
                m_fileOffset.LowPart, // начальный байт
                m_regionSize); // число проецируемых байт

    if(NULL == m_mapingRegion) {
        ErrorMsg(__FUNCTION__, "::MapViewOfFile");
        Close();
        return false;
    }

    return true;
}

bool CFileMapper::UnmappingRegion()
{
    // Увеличиваем смещение мапирование, а счетчик необработанных байт наоборот уменьшаем на размер региона
    m_fileOffset.QuadPart += m_regionSize;
    m_fileTail.QuadPart -= m_regionSize;

    // Освобождаепм регион
    if(m_mapingRegion && (0 == ::UnmapViewOfFile(m_mapingRegion))) {
        ErrorMsg(__FUNCTION__, "::UnmapViewOfFile");
        return false;
    }
    m_mapingRegion = NULL;
    return true;
}

bool CFileMapper::AddSymbolToBuffer(char *buffer, const int bufferSize, int &bufferIndex)
{
    if(NULL == buffer || (bufferSize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buffer = %p, bufferSize = %d\n", __FUNCTION__, (void*)buffer, bufferSize);
        return false;
    }

    // Бежим по региону, начиная со смещения
    for(DWORD i = m_regionOffset; i < m_regionSize; ++i) {
        // Опускаем символ возврата каретки, если таковой имеется
        if('\r' == m_mapingRegion[i])
            continue;
        // Если нашли символ переноса строки, то выходим
        if('\n' == m_mapingRegion[i]) {
            m_fileLine[bufferIndex] = '\0';
            m_regionOffset = i+1;
            CopyMemory(buffer, m_fileLine, min(bufferIndex+1, bufferSize));
            return true;
        }
        // Все остальные символы добавляем в строку
        m_fileLine[bufferIndex] = m_mapingRegion[i];
        bufferIndex++;
    }
    return false;
}
