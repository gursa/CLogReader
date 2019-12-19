#pragma once

#include <cfilemapper.h>
#include <cregex.h>

/*!
    \brief Класс, реализуюший обработку файла лога
    \author Михаил Макаров
    \date Декабрь 2019 года

    Работа начинается с вызова метода SetFilter для обработки паттерна и последующего получения токенов.
    Затем вызывается GetNextLine, который возвращает следующую строку удовлетворяющцю шаблону. Проверка происходит по тоекнам, полученным при успешном вызове SetFilter.
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
     * \brief Конструктор
     * \param isCaseSensitive Флаг, отвечающий за чувствительность к регистру при поиске
     */
    CLogReader(bool isCaseSensitive = true);

    /*!
     * \brief Деструктор
     *
     * Вызывает метод Close() для корректного освобождения ресурсов.
     */
    ~CLogReader();

    /*!
     * \brief Открывает файл для чтения
     * \param filename Имя файла
     * \return false - ошибка
     *
     * Производит открытие текствого файла. При filename == NULL или ошибке открытия возвращает false, в противном случае true.
     */
    bool Open(const char *filename);

    /*!
     * \brief Освобождение ресурсов
     *
     */
    void Close();

    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедет производится поиск
     * \return false - ошибка
     *
     * Производит предобработку и установку шаблона поиска.
     */
    bool SetFilter(const char *filter);

    /*!
     * \brief Запрос очередной найденной строки
     * \param buf Буфер
     * \param bufsize Максимальная длина
     * \return false - конец файла или ошибка
     *
     * Производит запрос очередной строки, удовлетворяющей фильтру поиска.
     * Реализовано через мапирование файла (для повышения быстродействия)
     * и последующего сравнение с фильтром.
     */
    bool GetNextLine(char *buf, const int bufsize);

private:

    CFileMapper m_fileMapper;
    CRegex m_regex;
};
