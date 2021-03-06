#include "clogreader.h"
#include <cstdio>

CLogReader::CLogReader()
    : m_fileMapping( NULL ),
      m_searchFilter( NULL ),
      m_buffer( NULL ),
      m_fileOffset( 0L ),
      m_mapingRegion( NULL ),
	  m_startPattern( NULL ),
      m_endPattern( NULL ),
	  m_itemPattern( NULL ),
	  m_substrPtr( NULL ),
	  m_stringOffset( 0 ),
	  m_bufferPosition( 1 ),
	  m_stringPosition( 0 ),
	  m_mapDelta( 0 ),
	  m_mapRegionSize( 0 ),
	  m_availableBytes( 0 )
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
    if(NULL == filter) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! filter = %p\n", __FUNCTION__, (void*)filter);
        Close();
		return false;
    }

	char buffer[2048];
    size_t m_bufferPosition = 0;
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
            if(m_bufferPosition) {
                buffer[m_bufferPosition] = '\0';
                m_bufferPosition = 0;
                if(!AddNode(typeText, buffer, strlen(buffer), isLeftAnchor, false))
				{
					Close();
					return false;
				}
                isLeftAnchor = false;
            }

            // ����� ��������� ���� �� �������������
            if(nextFilterSymbol == '?')
            {
                if(!AddNode(typeQuestion, "?", strlen("?"), isLeftAnchor, isRightAnchor))
				{
					Close();
					return false;
				}
                // ����� isLeftAnchor � isRightAnchor ����� ����� ������ ��� ������.
                //���� ������� ���� ���� ����������, �� ��� ����� ��� ��� ���������� �� ������ ��� ����������� �����
                isLeftAnchor = true;
            }
            else
            {
                // ��� ��������� ��������� ������ ���� ��� ���� ��� ���� ������
                // ����� ������ ���������� "******" �� "*"
                if((prevFilterSymbol != nextFilterSymbol))
				{
                    if(!AddNode(typeAsterics, "*", strlen("*"), false, false))
					{
						Close();
						return false;
					}
				}
                isLeftAnchor = false;
            }

        }
        else
        {
            // ���� �� �����������, �� ����������� �����
            buffer[m_bufferPosition] = nextFilterSymbol;
            m_bufferPosition++;
        }
        prevFilterSymbol = nextFilterSymbol;
    }

    // ���� �� ���������� ����� � ��� ��� ������� �������������� �����, �� �������������� ���
    if(m_bufferPosition)
	{
        buffer[m_bufferPosition] = '\0';
        m_bufferPosition = 0;
        if(AddNode(typeText, buffer, strlen(buffer), isLeftAnchor, true))
		{
			Close();
			return false;
		}
        isLeftAnchor = false;
    }

	return true;
}

bool CLogReader::GetNextLine( char* buf, const int bufsize )
{
    if(NULL == buf || (bufsize <= 0)) {
        fprintf(stderr, "\n[ERROR] %s(): Error in args! buf = %p, bufsize = %d\n", __FUNCTION__, (void*)buf, bufsize);
        return false;
    }

    m_mapOffset.QuadPart = 0;
    m_mapSize.QuadPart = 0;
    m_bufferPosition = 1;
    m_stringPosition = 0;
    m_mapDelta = 0;
    m_mapRegionSize = 0;
    m_availableBytes = 0;
    buf[0] = '\0';

    // �������� ...
    do
    {
        if( m_bufferPosition >= m_availableBytes )
        {
            // ����������� ���-�� ���� ��� ������. ���������� ������������, ��� ������ ����� ������������
            m_mapRegionSize = m_systemGran;
            // �� ���� ����� ��������, ��� �������� ��������� ����� �������������, �� ��������� ������� ��������
            if( m_fileSize.QuadPart - m_fileOffset < m_systemGran )
                m_mapRegionSize = m_fileSize.QuadPart - m_fileOffset;

            // ����������� �������� � �����
            m_mapOffset.QuadPart = (m_fileOffset / m_systemGran) * m_systemGran;
            // � ����� ���-�� ����������� ����
            m_mapSize.QuadPart = (m_fileOffset % m_systemGran) + m_mapRegionSize;
            // �� ��������� ��������� �������, �������� �, ��� �������������, �������� ���
            if( m_mapingRegion )
                UnmapViewOfFile( m_mapingRegion );

            // ��������� ����� �����
            m_mapingRegion = (char*) ::MapViewOfFile(
                        m_fileMapping,
                        FILE_MAP_READ,
                        m_mapOffset.HighPart, // ��������
                        m_mapOffset.LowPart, // ��������
                        m_mapSize.LowPart); // ���-�� ����
            if( !m_mapingRegion ) {
                ErrorMsg(__FUNCTION__, "::MapViewOfFile");
                Close();
                return false;
            }

            // ������������ ������ ����� ���������� ������
            m_mapDelta = m_fileOffset - m_mapOffset.QuadPart;
            m_buffer = (char*) m_mapingRegion + m_mapDelta;
            m_bufferPosition = 0;
            m_availableBytes = m_mapRegionSize;
            m_fileOffset += m_mapRegionSize;
        }

        // ���� �� ����������� ������� �������� �����, �� �� ����� ������ ��� �������
        if( m_buffer[m_bufferPosition] == '\r' || m_buffer[m_bufferPosition] == '\n' )
        {			
            // ���������, �������� �� ��� ���
            if(Match(buf))
            {
				m_fileOffset = m_fileOffset - m_mapRegionSize + m_bufferPosition + 1;
                return true;
            }
			
            m_stringPosition = 0;
            buf[m_stringPosition] = '\0';
        }
        else // � ��������� ������� ����������� ������
        {
            // �� ���� ����� �� ����� �� ������� ������������, �� ������� � �������
            if( m_stringPosition >= bufsize - 1 )
            {
                buf[m_stringPosition] = '\0';
                return false;
            }

            buf[m_stringPosition] = m_buffer[m_bufferPosition];
            ++m_stringPosition;
            buf[m_stringPosition] = '\0';
        }

        ++m_bufferPosition;
    } while( m_availableBytes > 0 );

    return false;
}

bool CLogReader::AddNode(type_t type, const char *data, const size_t dataSize, const bool isLeftAnchor, const bool isRightAnchor)
{
	if(NULL == data)
	{
		fprintf(stderr, 
			"\n[ERROR] %s(): Error in args! type = %d data = %p, dataSize = %d isLeftAnchor = %d isRightAnchor = %d\n", 
			__FUNCTION__, type, data, dataSize, isLeftAnchor, isRightAnchor);
        return false;
	}
	
	// ��������� ��������� ���� � ����������� ������. �����, �������� ������� ...
    // malloc ������ � ���� ����, ��� �� ����� �� ������ ���������� ...
    CPatternNode *tempNode = (CPatternNode*) malloc( sizeof(CPatternNode) );
	if(NULL == tempNode)
	{
		fprintf(stderr, "\n[ERROR] %s(): Failed to allocate memory for CPatternNode pointer\n", __FUNCTION__);        
		return false;
	}
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
	return true;
}

bool CLogReader::Match(char *string)
{
	if(NULL == string)
	{
		fprintf(stderr, "\n[ERROR] %s(): Error in args! string = %p\n", __FUNCTION__, string);
        return false;
	}

	m_itemPattern = m_startPattern;
	m_stringOffset = 0;
	m_substrPtr = NULL;
    while(m_itemPattern != NULL) {
        // ���� ����������� '*' - �� �������� ������, �� ����������� ������, ����� ���� ���������
        if(m_itemPattern->type == typeAsterics) {
            m_itemPattern = m_itemPattern->nextNode;
			if(!m_itemPattern) {
				return true;
			}
            continue;
        } else if(m_itemPattern->type == typeQuestion) { // ���� '?' - �������� ��� � ��� ��� ������ �� �������
            m_stringOffset++;
            if((m_stringOffset > m_stringPosition) || (m_itemPattern->isRightAnchor && (m_stringOffset != m_stringPosition)))
                return false;
        } else { // ���� �� � ��� �����
            // �� ���� ��� � ������
			m_substrPtr = strstr(string + m_stringOffset, m_itemPattern->data);
            if(m_substrPtr == NULL)
                return false;            

            if(m_itemPattern->isLeftAnchor && (m_substrPtr != string + m_stringOffset)) {
                return false;
            }

			// ���������, ��� �� ��������� � ������ (������������� �� �� ������)
            m_stringOffset=(m_substrPtr-string) + m_itemPattern->dataSize;

            if(m_itemPattern->isRightAnchor && (m_stringOffset != m_stringPosition)) {
                return false;
            }
        }
        m_itemPattern = m_itemPattern->nextNode;
    }
    return true;
}

void CLogReader::ErrorMsg(const char *methodName, const char *functionName)
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