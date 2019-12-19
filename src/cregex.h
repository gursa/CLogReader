#pragma once

/*!
    \brief Класс, реализуюший обработку регулярных выражений
    \author Михаил Макаров
    \date Декабрь 2019 года
    \warning Максимальный размер фильтра равен MAX_FILTER_SIZE == 1024 байт

    Работа начинается с вызова метода SetFilter для обработки фильтра.
    Затем вызывается Match, который призводит сравнение с шаблоном
*/
class CRegex
{
public:
    /*!
     * \brief Конструктор
     * \param isCaseSensitive Флаг, отвечающий за чувствительность к регистру при поиске
     */
    CRegex(bool isCaseSensitive = true);

    /*!
     * \brief Деструктор
     *
     * Освобождает занятые ресурсы посредством вызова FreeFilter
     */
    ~CRegex();

    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедет производится поиск
     * \return false - ошибка
     * \warning Максимальный размер фильтра равен MAX_FILTER_SIZE == 1024 байт
     *
     * Производит предобработку и установку шаблона поиска.
     */
    bool SetFilter(const char *filter);

    /*!
     * \brief Освобождение занятых ресурсов
     */
    void FreeFilter();

    /*!
     * \brief Получение установленного фильтра
     * \return Фильтр
     */
    char* GetFilter() const ;

    /*!
     * \brief Получение флага чуствительности к регистру
     * \return Значение флага
     */
    bool GetCaseSensitive() const;

    /*!
     * \brief Проверка строки на принадлежность фильтру
     * \param text Проверяемая строка
     * \param regex Объект с фильтром для сравнения
     * \param altTerminator Символ нультерминации
     * \return true Если совпадение найдено
     */
    static bool Match(char * __restrict text, CRegex& regex, char altTerminator = '\0');
private:
    /*!
     * \brief Проверка символа на '?'
     * \param symbol Проверяемый символ
     * \return Результат сравнения
     */
    static bool IsUndefined( char symbol );

    /*!
     * \brief Проверка на равенство двух символов, а также на то, что rSymbol не является '?'
     * \param lSymbol Левый проверяемый символ, эталон при сравнении
     * \param rSymbol Проверяемый символ
     * \return true если lSymbol == rSymbol или rSymbol является '?'
     */
    static bool IsEqual( char lSymbol, char rSymbol );

private:
    static const int MAX_FILTER_SIZE = 1024;

    //! Полученный фильтр
    char* m_searchFilter;

    //! Флаг, показывающий нужно ли учитывать регистр при поиске
    bool m_caseSensitive;
};
