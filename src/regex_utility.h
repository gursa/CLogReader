#pragma once

namespace regex_utility {

//! ��������� ������
typedef struct regex_t
{
    //!  ����� ��������� ����� �������
    enum type_t {
        typeUnused,         ///< ��������� �����, �� ������������ � ������
        typeDot,            ///< '.', ���������� � ����� �������� � ������
        typeBegin,          ///< '^', ��������� � ������� ������
        typeEnd,            ///< '$', ���������� � ������ ������
        typeQuestion,       ///< '?', ���������� � �� ����� ��� ����� �������� (�� �� ������� ���������)
        typeAsterisk,       ///< '*', ���������� � ����� ����������� ����� �������� � ������ (�� ������� ���������)
        typePlus,           ///< '+', ���������� � �� ����� ��� ����� �������� (�� ������� ���������)
        typeChar,           ///< �����, ���������� ����� ������, �� ���������� ����, ��������-������� ��� �������������� ��������-�������
        typeClass,          ///< '[...]' - ������-�����, ���������� � ���������� � ���������� �������. ��������, [0-9]
        typeDigit,          ///< '\d', ���������� � ������ �������. ���������� [0-9]
        typeNotDigit,       ///< '\D', ���������� � ����, ��� �������� �� �������
        typeAlpha,          ///< '\w', ���������� ������ ������� � �������, � ����� � �������� ������� ������������� '_'
        typeNotAlpha,       ///< '\W', ��������� �� ����, ��� �� �������� �������/�������/'_'
        typeWhitespace,     ///< '\s', ���������� � ��������, ����� � \\t \\f \\r \\n \\v
        typeNotWhitespace   ///< '\S', ���������� �� ����, ��� �� typeWhitespace
    }
    //! ��� ������
    type;

    //! ����������, �������� � ������
    union data_t
    {
        //! ������
        char symbol;
        //! ��������� �� ������� � ���������� �������.
        char* class_ptr;
    }data;
} regex_t;

bool isSpecialSymbol(char symbol);
bool isAlpha(char symbol);
bool isAlphaNumeric(char symbol);
bool isDigit(char symbol);
bool isWhitespace(char symbol);

int matchClass(char symbol, const char* text);
int matchRange(char symbol, const char* text);
int matchSpecialSymbol(char symbol, const char* text);
int matchOne(regex_t token_item, char symbol);
int matchPattern(regex_t* tokens, const char* text);
int matchPlus(regex_t token_item, regex_t* tokens, const char* text);
int matchQuestion(regex_t token_item, regex_t *tokens, const char* text);
int matchAsterisk(regex_t token_item, regex_t* tokens, const char* text);


}
