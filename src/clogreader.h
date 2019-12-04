#ifndef CLOGREADER_H
#define CLOGREADER_H

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <regex.h>

/*!
    \brief Класс, реализуюший обработку файла лога
    \author Михаил Макаров
    \date Декабрь 2019 года

    Работа начинается с вызова метода SetFilter для обработки паттерна и последующего получения токенов.
    Затем вызывается GetNextLine, который возвращает следующую строку удовлетворяющцю шаблону. Проверка происходит по тоекнам, полученным при успешном вызове SetFilter.
    \code
    CLogReader lr;
    bool rc = lr.SetFilter("^some[a-zA-Z\\s]*order$");
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
     * \brief Конструктор
     *
     * Инициализирует токены m_tokens NULL-ом
     */
    CLogReader();

    ~CLogReader();
    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедт производится поиск
     * \return false - ошибка
     */
    bool SetFilter(const char *filter);

    /*!
     * \brief Запрос очередной найденной строки
     * \param buf Буфер
     * \param bufsize Максимальная длина
     * \return false - конец файла или ошибка
     */
    bool GetNextLine(char *buf, const int bufsize);
private:
    bool prepare_pattern(const char *filter, const size_t filter_length, char *pattern);
    /*!
     * \brief Компиляция паттерна в токены
     * \param pattern Шаблон по которому будет производится поиск
     * \return true - если компиляция шаблона прошла успешно, false в обратном случае
     */
    bool compile_pattern(const char *pattern);
    bool make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j);
    bool make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j, char *class_buffer, size_t &class_buffer_index);
    /*!
     * \brief Поиск шаблона в строке
     * \param text Текст, в котором производится поиск
     * \param found Индекс найденного в строке шаблона (начиная с 0). Если вхождений не найдено, то будет возвращенно -1.
     * \return false - ошибка или совпадений не найдено
     */
    bool search(const char *text, int found = -1);
private:
    //! Токены шаблона
    struct regex_utility::regex_t *m_tokens;
};

#endif // CLOGREADER_H
