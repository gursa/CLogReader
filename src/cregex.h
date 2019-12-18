#pragma once


class CRegex
{
public:
    /*!
     * \brief Конструктор
     * \param isCaseSensitive Флаг, отвечающий за чувствительность к регистру при поиске
     */
    CRegex(bool isCaseSensitive = true);
    ~CRegex();

    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедет производится поиск
     * \return false - ошибка
     *
     * Производит предобработку и установку шаблона поиска.
     */
    bool SetFilter(const char *filter);
    void FreeFilter();

    char* GetFilter() const ;
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
