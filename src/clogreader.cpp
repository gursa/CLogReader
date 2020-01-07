#include "clogreader.h"
#include <cstdio>

namespace {

// ����������� ������������ ������, ��������� ��������� WinAPI
void ErrorMsg(const char *methodName, const char *functionName)
{
    if((NULL == methodName) && (NULL == functionName)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! methodName = %p, functionName = %p\n",
                __FUNCTION__, (void*)methodName, (void*)functionName);
        return;
    }

    // ��������� �������� ��������� ������ �� �� ����
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
        fprintf(stderr, "\n[ERROR] [%s()] %s() failed with error %lu\n", methodName, functionName, errorCode);
        fprintf(stderr, "[ERROR] [%s()] %s() failed with error %lu\n", methodName, "FormatMessageA", GetLastError());
    } else {
        fprintf(stderr, "\n[ERROR] [%s()] %s() failed with error %lu: %s\n", methodName, functionName, errorCode, messageBuffer);
    }

    LocalFree(messageBuffer);
}

}

CLogReader::CLogReader()
    : m_fileMapping( NULL ),
      m_searchFilter( NULL ),
      m_buffer( NULL ),
      m_fileOffset( 0L ),
      m_mapingRegion( NULL ),
      m_startPattern( NULL ),
      m_endPattern( NULL )
{
    SYSTEM_INFO systemInfo;
    GetSystemInfo( &systemInfo );
    m_systemGran = systemInfo.dwAllocationGranularity;
    m_fileSize.QuadPart = 0;
}

CLogReader::~CLogReader()
{
    Close();
}

bool CLogReader::Open( const char* fileName )
{
    if(NULL == fileName){
        fprintf(stderr, "\n[ERROR] %s(): Error in args! fileName = %p\n", __FUNCTION__, (void*)fileName);
        return false;
    }

    Close();

    HANDLE logFile = ::CreateFileA(
        fileName,
        FILE_READ_DATA,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN,
        NULL
    );

    if( INVALID_HANDLE_VALUE == logFile ) {
        ErrorMsg(__FUNCTION__, "::CreateFile");
        return false;
    }

    if(!::GetFileSizeEx( logFile, &m_fileSize )) {
        ErrorMsg(__FUNCTION__, "::GetFileSizeEx");
        CloseHandle( logFile );
        logFile = INVALID_HANDLE_VALUE;
        return false;
    }

    m_fileMapping = ::CreateFileMapping(
                logFile,
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL);
    CloseHandle( logFile );
    logFile = INVALID_HANDLE_VALUE;
    if( !m_fileMapping )
    {
        ErrorMsg(__FUNCTION__, "::CreateFileMapping");
        Close();
        return false;
    }

    return true;
}

void CLogReader::Close()
{
    while( m_startPattern != NULL ) {
        CPatternNode *tmp = m_startPattern->nextNode;
        free(m_startPattern);
        m_startPattern = NULL;
        m_startPattern = tmp;
    }
    m_endPattern = NULL;

    // ����������� ������
    if( m_buffer && !::UnmapViewOfFile(m_buffer) ) {
        ErrorMsg(__FUNCTION__, "::UnmapViewOfFile");
    }
    m_buffer = NULL;

    if( m_fileMapping && !::CloseHandle(m_fileMapping) ) {
        ErrorMsg(__FUNCTION__, "::CloseHandle");
    }
    m_fileMapping = NULL;
}

bool CLogReader::SetFilter( const char* filter )
{
    if(!filter) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! filter = %p\n", __FUNCTION__, (void*)filter);
        Close();
        return false;
    }

    char buffer[2048];
    size_t bufferPosition = 0;
    char prevFilterSymbol = '\0';
    char nextFilterSymbol = filter[0];
    bool isLeftAnchor = true; // ������ ���� ����������, ���� �� � ��� �������� � ������ ������
    for(size_t i = 0; i < strlen(filter); i++)
    {
        nextFilterSymbol = filter[i];
        // isRightAnchor - ����, ������������, ���� �� � ��� �������� � ����� ������
        bool isRightAnchor = (i == strlen(filter)-1) ? true : false;
        // ���� ����� ������������� �����������
        if((nextFilterSymbol == '?') || (nextFilterSymbol == '*'))
        {
            // ���������, �� ���� �� ����� ���� ������, ���� ����, �� ������� ������� ���� � �������
            if(bufferPosition) {
                buffer[bufferPosition] = '\0';
                bufferPosition = 0;
                AddNode(typeText, buffer, strlen(buffer), isLeftAnchor, false);
                isLeftAnchor = false;
            }

            // ����� ��������� ���� �� �������������
            if(nextFilterSymbol == '?')
            {
                AddNode(typeQuestion, "?", strlen("?"), isLeftAnchor, isRightAnchor);
                // ����� isLeftAnchor � isRightAnchor ����� ����� ������ ��� ������.
                //���� ������� ���� ���� ����������, �� ��� ����� ��� ��� ���������� �� ������ ��� ����������� �����
                isLeftAnchor = false;
            }
            else
            {
                // ��� ��������� ��������� ������ ���� ��� ���� ��� ���� ������
                // ����� ������ ���������� "******" �� "*"
                if((prevFilterSymbol != nextFilterSymbol))
                    AddNode(typeAsterics, "*", strlen("*"), false, false);
                isLeftAnchor = false;
            }

        }
        else
        {
            // ���� �� �����������, �� ����������� �����
            buffer[bufferPosition] = nextFilterSymbol;
            bufferPosition++;
        }
        prevFilterSymbol = nextFilterSymbol;
    }

    // ���� �� ���������� ����� � ��� ��� ������� �������������� �����, �� �������������� ���
    if(bufferPosition) {
        buffer[bufferPosition] = '\0';
        bufferPosition = 0;
        AddNode(typeText, buffer, strlen(buffer), isLeftAnchor, true);
        isLeftAnchor = false;
    }
    return true;
}

bool CLogReader::GetNextLine( char* buf, const int bufsize )
{
    if(NULL == buf || (bufsize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buf = %p, bufferSize = %d\n", __FUNCTION__, (void*)buf, bufsize);
        return false;
    }

    LARGE_INTEGER mapOffset;
    mapOffset.QuadPart = 0;
    LARGE_INTEGER mapSize;
    mapSize.QuadPart = 0;
    DWORD bufferPosition = 1;
    int bufPos = 0;
    LONGLONG mapDelta = 0;
    LONGLONG bufferSize = 0;
    LONGLONG available = 0;
    buf[0] = '\0';

    // �������� ...
    do
    {
        if( bufferPosition >= available )
        {
            // ����������� ���-�� ���� ��� ������. ���������� ������������, ��� ������ ����� ������������
            bufferSize = m_systemGran;
            // �� ���� ����� ��������, ��� �������� ��������� ����� �������������, �� ��������� ������� ��������
            if( m_fileSize.QuadPart - m_fileOffset < m_systemGran )
                bufferSize = m_fileSize.QuadPart - m_fileOffset;

            // ����������� �������� � �����
            mapOffset.QuadPart = (m_fileOffset / m_systemGran) * m_systemGran;
            // � ����� ���-�� ����������� ����
            mapSize.QuadPart = (m_fileOffset % m_systemGran) + bufferSize;
            // �� ��������� ��������� �������, �������� �, ��� �������������, �������� ���
            if( m_mapingRegion )
                UnmapViewOfFile( m_mapingRegion );

            // ��������� ����� �����
            m_mapingRegion = (char*) ::MapViewOfFile(
                        m_fileMapping,
                        FILE_MAP_READ,
                        mapOffset.HighPart, // ��������
                        mapOffset.LowPart, // ��������
                        mapSize.LowPart); // ���-�� ����
            if( !m_mapingRegion ) {
                ErrorMsg(__FUNCTION__, "::MapViewOfFile");
                Close();
                return false;
            }

            // ������������ ������ ����� ���������� ������
            mapDelta = m_fileOffset - mapOffset.QuadPart;
            m_buffer = (char*) m_mapingRegion + mapDelta;
            bufferPosition = 0;
            available = bufferSize;
            m_fileOffset += bufferSize;
        }

        // ���� �� ����������� ������� �������� �����, �� �� ����� ������ ��� �������
        if( m_buffer[bufferPosition] == '\r' || m_buffer[bufferPosition] == '\n' )
        {
            // ���������, �������� �� ��� ���
            if(Match(buf, m_startPattern))
            {                
                m_fileOffset = m_fileOffset - bufferSize + bufferPosition + 1;
                return true;
            }
			
            bufPos = 0;
            buf[bufPos] = '\0';
        }
        else // � ��������� ������� ����������� ������
        {
            // �� ���� ����� �� ����� �� ������� ������������, �� ������� � �������
            if( bufPos >= bufsize - 1 )
            {
                buf[bufPos] = '\0';
                return false;
            }

            buf[bufPos] = m_buffer[bufferPosition];
            ++bufPos;
            buf[bufPos] = '\0';
        }

        ++bufferPosition;
    } while( available > 0 );

    return false;
}

bool CLogReader::IsEqual(char lSymbol, char rSymbol)
{
    return (lSymbol == rSymbol) || (rSymbol == '?');
}

void CLogReader::AddNode(type_t type, const char *data, const size_t dataSize, const bool isLeftAnchor, const bool isRightAnchor)
{
    // ��������� ��������� ���� � ����������� ������. �����, �������� ������� ...
    // malloc ������ � ���� ����, ��� �� ����� �� ������ ���������� ...
    CPatternNode *tempNode = (CPatternNode*) malloc( sizeof(CPatternNode) );
    tempNode->type = type;
    tempNode->data = _strdup( data);
    tempNode->dataSize = dataSize;
    tempNode->isLeftAnchor = isLeftAnchor;
    tempNode->isRightAnchor = isRightAnchor;
    tempNode->nextNode = NULL;

    if(m_startPattern == NULL)
    {
        m_startPattern = tempNode;
        m_endPattern = tempNode;
    }
    else
    {
        m_endPattern->nextNode = tempNode;
        m_endPattern = m_endPattern->nextNode;
    }
}

bool CLogReader::Match(char *string, CPatternNode *compiledPattern)
{
    CPatternNode *itemPattern = compiledPattern;
    size_t length = strlen(string);
    size_t i = 0;
    while(itemPattern != NULL) {
        // ���� ����������� '*' - �� �������� ������, �� ����������� ������, ����� ���� ���������
        if(itemPattern->type == typeAsterics) {
            itemPattern = itemPattern->nextNode;
            if(!itemPattern)
                return true;
            continue;
        } else if(itemPattern->type == typeQuestion) { // ���� '?' - �������� ��� � ��� ��� ������ �� �������
            i++;
            if((i > length) || (itemPattern->isRightAnchor && (i != length)))
                return false;
            string++;
        } else { // ���� �� � ��� �����
            // �� ���� ��� � ������
            char *find = strstr(string, itemPattern->data);
            if(find == NULL)
                return false;

            // ���������, ��� �� ��������� � ������ (������������� �� �� ������)
            i=(find-string) + itemPattern->dataSize;

            if(itemPattern->isLeftAnchor && (find != string)) {
                return false;
            }

            if(itemPattern->isRightAnchor && (i != length)) {
                return false;
            }
        }
        itemPattern = itemPattern->nextNode;
    }
    return true;
}
