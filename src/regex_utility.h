#pragma once

namespace regex_utility {

//! Структура токена
typedef struct regex_t
{
    //!  Набор возможных типов токенов
    enum type_t {
        typeUnused,         ///< Последний токен, не используется в поиске
        typeDot,            ///< '.', совпадение с любым символом в строке
        typeBegin,          ///< '^', сопдаение с началом строки
        typeEnd,            ///< '$', совпадение с концом строки
        typeQuestion,       ///< '?', совпадение с не более чем одним символом (по не жадному алгоритму)
        typeAsterisk,       ///< '*', совпадение с любым количеством любых символом в строке (по жадному алгоритму)
        typePlus,           ///< '+', совпадение с не менее чем одним символом (по жадному алгоритму)
        typeChar,           ///< текст, совпадение любым символ, не являющимся спец, символом-классом или экранированным символом-классом
        typeClass,          ///< '[...]' - символ-класс, совпадение с содержимым в квадратных скобках. Например, [0-9]
        typeDigit,          ///< '\d', совпадение с любыми цифрами. Аналогично [0-9]
        typeNotDigit,       ///< '\D', совпадение с всем, что является не цифрами
        typeAlpha,          ///< '\w', совпадение любыми цифрами и буквами, а также с символом нижнего подчеркивания '_'
        typeNotAlpha,       ///< '\W', сопадение со всем, что не является цифрами/буквами/'_'
        typeWhitespace,     ///< '\s', совпадение в пробелом, также с \\t \\f \\r \\n \\v
        typeNotWhitespace   ///< '\S', совпадение со всем, что не typeWhitespace
    }
    //! Тип токена
    type;

    //! Информация, хранимая в токене
    union data_t
    {
        //! Символ
        unsigned char symbol;
        //! Указатель на символы в квадратных скобках.
        unsigned char* class_ptr;
    }data;
} regex_t;

int isSpecialSymbol(char symbol);
int isAlpha(char symbol);
int isAlphaNumeric(char symbol);
int isDigit(char symbol);
int isWhitespace(char symbol);

int matchClass(char symbol, const char* text);
int matchRange(char symbol, const char* text);
int matchSpecialSymbol(char symbol, const char* text);
int matchOne(regex_t token_item, char symbol);
int matchPattern(regex_t* tokens, const char* text);
int matchPlus(regex_t token_item, regex_t* tokens, const char* text);
int matchQuestion(regex_t token_item, regex_t *tokens, const char* text);
int matchAsterisk(regex_t token_item, regex_t* tokens, const char* text);


}
