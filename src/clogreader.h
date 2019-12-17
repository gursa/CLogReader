#pragma once

#include <cstdio>
#include <windows.h>

//! Размер буфера для символов из всех выражений в квадратных скобках
#define MAX_CLASSES_BUFFER_LEN 100

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
    /*!
     * \brief Получение проекции мапируемого файла
     * \return В случае успеха, возвращается true, в противном случае false
     *
     * Вызывает MapViewOfFile с соответствующими параметрами
     * \warning Обязательно вызвать UnmappingRegion для освобождения занимаемых ресурсов
     */
    bool MappingNextRegion();

    /*!
     * \brief Освобождение ресурсов после мапирования
     * \return В случае успеха, возвращается true, в обратном случае false
     *
     * Вызывает UnmapViewOfFile
     * \warning Является парной функцией к MappingNextRegion
     */
    bool UnmappingRegion();

    /*!
     * \brief Добавление символа из мапированного региона в буфер
     * \param buffer Буфер для добавления
     * \param bufferSize Размер буфера для добавления
     * \param bufferIndex Индекс в буфере добавляемого символа
     * \return true в случае успеха
     */
    bool AddSymbolToBuffer(char *buffer, const int bufferSize, int &bufferIndex);

    /*!
     * \brief Получение строки из мапированного региона
     * \param buffer Буфер, полученная строка
     * \param bufferSize Размер буфера
     * \return true в случае успеха
     */
    bool GetString(char *buffer, const int bufferSize);

    /*!
     * \brief Отображение текста ошибки по коду ::GetLastError()
     * \param functionName Имя функции, вызвавшей ошибку
     */
    void ErrorMsg(const char *functionName);

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

    /*!
     * \brief Проверка строки на принадлежность фильтру
     * \param text Проверяемая строка
     * \param filter Фильтр сравнения
     * \param сaseSensitive Флаг, показывающий, будем ли учитытвать регистр
     * \param altTerminator Символ нультерминации
     * \return true Если совпадение найдено
     */
    static bool Match(char *   __restrict text, char *  __restrict  filter, bool сaseSensitive = true, char altTerminator = '\0');

private:    
    //! Вспомогательная структура, используется для получении информации о гранулярности
    SYSTEM_INFO m_systemInfo;

    //! Дескриптор объекта проекции файла
    HANDLE m_fileMapping;

    //! Размер файла
    LARGE_INTEGER m_fileSize;

	//! Количество необработанных байт в файле
	LARGE_INTEGER m_fileTail;

    //!  Смещение в файле при мапировании
    LARGE_INTEGER m_fileOffset;

    //! Смапированный регион
    char* m_mapingRegion;

    //! Смещение в смапированном регионе
    DWORD m_regionOffset;

    //! Размер смапированного региона
    DWORD m_regionSize;

    char m_fileLine[2*65*1024];

    static const int MAX_FILTER_SIZE = 1024;

    //! Полученный фильтр
    char* m_searchFilter;

    //! Флаг, показывающий нужно ли учитывать регистр при поиске
    bool m_caseSensitive;
};
