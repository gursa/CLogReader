#pragma once

#include <windows.h>

/// ��� ����
enum type_t {
    typeText = 0, ///< �����
    typeAsterics, ///< ���������� "*" - ������������������ ����� �������� �������������� �����
    typeQuestion, ///< ���������� "?" - ���� ����� ������
};

struct CPatternNode
{
    //! ��� ����
    type_t type;
    //! ���������� ����
    char* data;
    //! ������ �����������
    size_t dataSize;
    //! �������, ������ �� ������� ���� ������ �������� � ������ ������
    bool isLeftAnchor;
    //! �������, ������ �� ������� ���� ������ �������� � ����� ������
    bool isRightAnchor;
    //! ��������� �� ����. ����
    CPatternNode *nextNode;
};

class CLogReader
{
public:
    /*!
     * \brief �����������
     */
    CLogReader();

    /*!
     * \brief ����������
     *
     * �������� ����� Close() ��� ����������� ������������ ��������.
     */
    ~CLogReader();

    /*!
     * \brief ��������� ���� ��� ������
     * \param fileName ��� �����
     * \return false - ������
     *
     * ���������� �������� ��������� �����. ��� filename == NULL ��� ������ �������� ���������� false, � ��������� ������ true.
     */
    bool Open( const char* fileName );

    /*!
     * \brief ������������ ��������
     */
    void Close();

    /*!
     * \brief ��������� ������� �����
     * \param filter ������ �� �������� ����� ������������ �����
     * \return false - ������
     *
     * ���������� ������������� � ��������� ������� ������.
     */
    bool SetFilter( const char* filter );

    /*!
     * \brief ������ ��������� ��������� ������
     * \param buf �����
     * \param bufsize ������������ �����
     * \return false - ����� ����� ��� ������
     *
     * ���������� ������ ��������� ������, ��������������� ������� ������.
     * ����������� ����� ����������� ����� (��� ��������� ��������������)
     * � ������������ ��������� � ��������.
     */
    bool GetNextLine( char* buf, const int bufsize );

private:
    /*!
     * \brief �������� �� ��������� ���� ��������, � ����� �� ��, ��� rSymbol �� �������� '?'
     * \param lSymbol ����� ����������� ������, ������ ��� ���������
     * \param rSymbol ����������� ������
     * \return true ���� lSymbol == rSymbol ��� rSymbol �������� '?'
     */
    static bool IsEqual( char lSymbol, char rSymbol );

    /*!
     * \brief ���������� ���� � ���������������� ������� ������ (������������� ������)
     * \param type ��� ���� ('*', '?' ��� �����)
     * \param data ���������� ����
     * \param text_length ������ �����������
     * \param is_start �������, ������ �� ������� ���� ������ �������� � ������ ������
     * \param is_end �������, ������ �� ������� ���� ������ �������� � ����� ������
     */
    void AddNode(type_t type, const char *data, const size_t dataSize, const bool isLeftAnchor, const bool isRightAnchor);

    /*!
     * \brief ������� �������� �������������� ������ ���������� ��������
     * \param string ����������� ������
     * \param compiledPattern ��������� �� ������ ������������ ������ � ��������������� ��������� ������
     * \return true ���� ������ ������������� ��������� ������
     */
    static bool Match(char *string, struct CPatternNode *compiledPattern);

private:
    //! �������������
    DWORD m_systemGran;

    //! ���������� ��������� �����
    LARGE_INTEGER m_fileSize;

    //! ���������� ������� �������� �����
    HANDLE m_fileMapping;

    //! ���������� ������
    char* m_searchFilter;

    //! ����� �� �������� ������������ �����
    char* m_buffer;

    //! �������� � ����� ��� �����������
    LONGLONG m_fileOffset;

    //! ������������� ������
    LPVOID m_mapingRegion;

    //! ������ ������� ����������������� �������� ������
    CPatternNode* m_startPattern;

    //! ��������� ������� ����������������� �������� ������
    CPatternNode* m_endPattern;
};

