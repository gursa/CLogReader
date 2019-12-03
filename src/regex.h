#pragma once

#include <regex_utility.h>

namespace regex
{

/*!
    \brief Класс, реализуюший обработку регулярных выражений
    \author Михаил Макаров
    \date Декабрь 2019 года

    Работа начинается с вызова метода compile для обработки паттерна и последующего получения токенов.
    Затем вызывается search, в который передается исследуемый текст. Проверка происходит по тоекнам, полученным при успешном вызове compile.
    \code
    CRegex rgx;
    bool compile_result = rgx.compile("^some[a-zA-Z\\s]*order$");
    if(compile_result)
    {
        int search_result = rgx.search("some    order");
    }
    \endcode
*/

class CRegex
{
public:    
    /*!
     * \brief Конструктор
     *
     * Инициализирует токены m_tokens NULL-ом
     */
    CRegex();

    /*!
     * \brief Компиляция паттерна в токены
     * \param pattern Шаблон по которому будет производится поиск
     * \return true - если компиляция шаблона прошла успешно, false в обратном случае
     */
    bool compile(const char* pattern);

    /*!
     * \brief Поиск шаблона в строке
     * \param text Текст, в котором производитсчя поиск
     * \return Индекс найденного в строке шаблона (начиная с 0). Если вхождений не найдено, то будет возвращенно -1.
     */
    int search(const char *text);
private:
    void make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, int &i, int &j);
    bool make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, int &i, int &j, unsigned char *class_buffer, int &class_buffer_index);
private:
    //! Токены шаблона
    struct regex_utility::regex_t *m_tokens;
};

}
