#ifndef CLOGREADER_H
#define CLOGREADER_H

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <regex.h>

//! ������ ������ ��� �������� �� ���� ��������� � ���������� �������
#define MAX_CLASSES_BUFFER_LEN 100

/*!
    \brief �����, ����������� ��������� ����� ����
    \author ������ �������
    \date ������� 2019 ����

    ������ ���������� � ������ ������ SetFilter ��� ��������� �������� � ������������ ��������� �������.
    ����� ���������� GetNextLine, ������� ���������� ��������� ������ ��������������� �������. �������� ���������� �� �������, ���������� ��� �������� ������ SetFilter.
    \code
    CLogReader lr;
    bool rc = lr.Open("test.log");
    rc = lr.SetFilter("^some[a-zA-Z\\s]*order$");
    if(rc)
    {
        char buf[4096];
        int bufsize = sizeof(bufsize);
        bool search_result = lr.GetNextLine(buf, bufsize);
    }
    \endcode
*/

class CLogReader
{
public:
    /*!
     * \brief �����������
     *
     * �������������� ��������� m_tokens NULL-��.
     */
    CLogReader();

    /*!
     * \brief ����������
     *
     * ������� ���������� ������ ��� ������ m_tokens. �������� ����� Close() ��� ����������� �������� �����.
     */
    ~CLogReader();

    /*!
     * \brief ��������� ���� ��� ������
     * \param filename ��� �����
     * \return false - ������
     *
     * ���������� �������� ��������� �����. ��� filename == NULL ��� ������ �������� ���������� false, � ��������� ������ true.
     */
    bool Open(const char *filename);

    /*!
     * \brief �������� ���������� �����
     *
     * ��������� ���������� ����, ���� ��������������� ���������� m_file != NULL.
     */
    void Close();

    /*!
     * \brief ��������� ������� �����
     * \param filter ������ �� �������� ����� ������������ �����
     * \return false - ������
     *
     * ���������� ��������� ������� ������. ��� ����� �������� prepare_pattern ��� ���������� �������� � ����������� ���������� � ������ ������ m_tokens.
     * ���� filter == NULL ��� ���������� ������ ��� ��������� ������ ��� ������� ������������ false. � ��������� ������ ������������ ��������� prepare_pattern.
     */
    bool SetFilter(const char *filter);

    /*!
     * \brief ������ ��������� ��������� ������
     * \param buf �����
     * \param bufsize ������������ �����
     * \return false - ����� ����� ��� ������
     *
     * ���������� ������ ��������� ������, ��������������� ������� m_tokens. ����������� ����� ���������� ������ ����� � ������� fgets � ������������
     * ��������� � �������� ����������� ������� search. � ������ ���� search ������� ���������� ������� ����������� � true, � ��������� ������ ������ ������������.
     * false ������������ ���� buf == NULL ��� bufsize == 0 ��� �������� ���������� m_file �� ��������������� ��������������� fopen � ������� Open ��� ���������� ������ ������,
     * � ����� ���� ��������� ����� �����.
     */
    bool GetNextLine(char *buf, const int bufsize);
private:
    /*!
     * \brief ����������� ������� �� �������� ���� �� ���������� �������
     * \param filter ������
     * \param filter_length ����� �������
     * \param pattern ���������� �������
     * \return false ���� ���� ������ � ����������, � ��������� ������ ��������� compile_pattern
     *
     * ���������� �������� ���������� ��������� ('*'), � ����� ��������� ����� ������ ('^') � ����� ('$') �������, ���� ������ ���������� � ������������� � ����/���� ��������������.
     * ���������� false ���� filter == NULL ��� filter_length == 0 ��� pattern == NULL, � ��������� ������ ��������� compile_pattern
     */
    bool prepare_pattern(const char *filter, const size_t filter_length, char *pattern);

    /*!
     * \brief ���������� �������� � ������
     * \param pattern ������� �� �������� ����� ������������ �����
     * \return true - ���� ���������� ������� ������ �������, false � �������� ������
     * \warning ���� � �������� ������������ �������-������ (�� ��� ��������� ����� ���������� ������ '[' � ']'), �� �� ����� �� ������ ��������� MAX_CLASSES_BUFFER_LEN ��������.
     *
     */
    bool compile_pattern(const char *pattern);

    /*!
     * \brief ��������������� ���������������� ������� (�� ��� ������� ����� '\') � ������
     * \param pattern ������� �� �������� ����� ������������ �����
     * \param tokens_compiled ������, � ������� ������������ �����������
     * \param i ������ � �������� pattern, � �������� ������������� ������
     * \param j ������ � ������� ������� tokens_compiled, � ������� ����� ������������ ������-������
     * \return false ���� ���� ������ � ����������
     *
     * ��������� ��������� ������ �� '\' � �������������. ������������ false ���� pattern == NULL ��� tokens_compiled == NULL, � ��������� ������ true.
     */
    bool make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j);

    /*!
     * \brief ��������������� �������-������ (�� ��� ��������� ����� ���������� ������ '[' � ']') � ������
     * \param pattern ������� �� �������� ����� ������������ �����
     * \param tokens_compiled ������, � ������� ������������ �����������
     * \param i ������ � �������� pattern, � �������� ������������� ������
     * \param j ������ � ������� ������� tokens_compiled, � ������� ����� ������������ ������-������
     * \param class_buffer ������ � ������� ������������� ��� �������, ������������ � ������-������
     * \param class_buffer_index ������ �� class_buffer
     * \return false - ������
     *
     * ��������� ��������� ����� '[' � ']' � ������������� regex_utility::regex_t::data::class_ptr. � ������ ������, ������������ true, � ��������� ������ ������������ false.
     */
    bool make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j, char *class_buffer, size_t &class_buffer_index);

    /*!
     * \brief ����� ������� � ������
     * \param text �����, � ������� ������������ �����
     * \param found ������ ���������� � ������ ������� (������� � 0). ���� ��������� �� �������, �� ����� ����������� -1.
     * \return false - ������ ��� ���������� �� �������
     */
    bool search(const char *text, int &found);
private:
    //! ������ �������
    struct regex_utility::regex_t *m_tokens;

    //! ����, �� �������� ������������ �����
    FILE *m_file;

    //! ����� ������ � �����
    size_t m_strnum;
};

#endif // CLOGREADER_H
