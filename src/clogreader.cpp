
#include "clogreader.h"

CLogReader::CLogReader(bool isCaseSensitive)
    : 	m_fileMapping(NULL),
        m_mapingRegion(NULL),
        m_regionOffset(0),
        m_regionSize(0),
        m_searchFilter( NULL ),
        m_caseSensitive(isCaseSensitive)
{
    GetSystemInfo(&m_systemInfo);
	m_fileSize.QuadPart = 0;
    m_fileTail.QuadPart = 0;
    m_fileOffset.QuadPart = 0;
}

CLogReader::~CLogReader(void)
{
    Close();
}

bool CLogReader::Open(const char *filename)
{
    Close();

    if(NULL == filename){
        fprintf(stderr, "\n[ERROR] %s(): Error in args! filename = %p\n", __FUNCTION__, filename);
        return false;
    }

    HANDLE fileLog = ::CreateFileA(
        filename,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL);
    if(INVALID_HANDLE_VALUE == fileLog) {
        ErrorMsg("::CreateFile");
        return false;
    }

    DWORD fileSizeHigh = 0;
    m_fileSize.LowPart = ::GetFileSize(fileLog, &fileSizeHigh);
    m_fileSize.HighPart = (LONG)fileSizeHigh;
    if(INVALID_FILE_SIZE == m_fileSize.LowPart) {
        ErrorMsg("::GetFileSize");
        m_fileSize.QuadPart = 0;
        return false;
    }

	m_fileTail.QuadPart = m_fileSize.QuadPart;

    m_fileMapping = ::CreateFileMapping(
        fileLog,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL);

    if(0 == ::CloseHandle(fileLog)) {
        ErrorMsg("::CloseHandle");
    }

    if(NULL == m_fileMapping) {
        ErrorMsg("::CreateFileMapping");
        m_fileTail.QuadPart = 0;
        return false;
    }
    return true;
}

void CLogReader::Close()
{
    if(m_searchFilter)
        free(m_searchFilter);
    m_searchFilter = NULL;

    UnmappingRegion();

    if(m_fileMapping && (0 == ::CloseHandle(m_fileMapping))) {
        ErrorMsg("::CloseHandle");
    }
    m_fileMapping = NULL;
}

bool CLogReader::SetFilter(const char *filter)
{
    size_t filterLength = strnlen_s( filter, MAX_FILTER_SIZE );
    if(0 == filterLength)
        return false;
    m_searchFilter = (char*) malloc( filterLength + 1 );
    if(NULL == m_searchFilter)
        return false;
    if (0 != strcpy_s( m_searchFilter, filterLength + 1, filter )) {
        Close();
		return false;
    }

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

bool CLogReader::GetNextLine(char *buf, const int bufsize)
{
    if(NULL == buf || (bufsize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buf = %p, bufferSize = %d\n", __FUNCTION__, (void*)buf, bufsize);
        return false;
    }

    int found = -1;
    while(1) {
        // Получаем строку из смапированного файла
        if(false == GetString(buf, bufsize))
            break;

        if(Match( buf, m_searchFilter, m_caseSensitive ))
            return true;
    }
    return false;
}

bool CLogReader::IsUndefined(char symbol)
{
    return symbol == '?';
}

bool CLogReader::IsEqual(char lSymbol, char rSymbol)
{
    return lSymbol == rSymbol || IsUndefined(rSymbol);
}

bool CLogReader::Match(char * __restrict text, char * __restrict filter, bool bCaseSensitive, char cAltTerminator)
{
    bool matchResult = true;
    char* afterLastFilter = NULL;
    char* afterLastText = NULL;
    char symbolText;
    char symbolFilter;

    while( 1 )
    {
        symbolText = *text;
        symbolFilter = *filter;

        if( !symbolText || symbolText == cAltTerminator )
        {
            if( !symbolFilter || symbolFilter == cAltTerminator )
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
                if( !( *afterLastText ) || *afterLastText == cAltTerminator )
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
            if( !bCaseSensitive )
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

                    if( !symbolFilter || symbolFilter == cAltTerminator )
                        break;
                    continue;
                }
                else if ( afterLastFilter )
                {
                    if( afterLastFilter != filter )
                    {
                        filter = afterLastFilter;
                        symbolFilter = *filter;

                        if( !bCaseSensitive && symbolFilter >= 'A' && symbolFilter <= 'Z' )
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

bool CLogReader::MappingNextRegion()
{
	double percentComplete = (double)m_fileOffset.QuadPart / (double)m_fileSize.QuadPart * 100.0;
	fprintf(stdout, "File processing: %3.3f %%\r", percentComplete);
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
        ErrorMsg("::MapViewOfFile");
        Close();
        return false;
    }

    return true;
}

bool CLogReader::UnmappingRegion()
{
    // Увеличиваем смещение мапирование, а счетчик необработанных байт наоборот уменьшаем на размер региона
    m_fileOffset.QuadPart += m_regionSize;
    m_fileTail.QuadPart -= m_regionSize;

    // Освобождаепм регион
    if(m_mapingRegion && (0 == ::UnmapViewOfFile(m_mapingRegion))) {
        ErrorMsg("::UnmapViewOfFile");
        return false;
    }
    m_mapingRegion = NULL;
    return true;
}

bool CLogReader::AddSymbolToBuffer(char *buffer, const int bufferSize, int &bufferIndex)
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

bool CLogReader::GetString(char *buffer, const int bufferSize)
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

void CLogReader::ErrorMsg(const char *functionName)
{
    if(NULL == functionName) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! functionName = %p\n", __FUNCTION__, (void*)functionName);
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
        fprintf(stderr, "\n[ERROR] %s() failed with error %lu\n", functionName, errorCode);
        fprintf(stderr, "[ERROR] %s() failed with error %lu\n", "FormatMessageA", GetLastError());
    } else {
        fprintf(stderr, "\n[ERROR] %s() failed with error %lu: %s\n", functionName, errorCode, messageBuffer);
    }

    LocalFree(messageBuffer);
}
