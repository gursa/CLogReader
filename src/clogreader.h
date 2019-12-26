#pragma once

#include <windows.h>

/// Тип узла
enum type_t {
    typeText = 0, ///< Текст
    typeAsterics, ///< Спецсимвол "*" - последовательность любых символов неограниченной длины
    typeQuestion, ///< Спецсимвол "?" - один любой символ
};

struct CPatternNode
{
    //! Тип узла
    type_t type;
    //! Содержимое узла
    char* data;
    //! Размер содержимого
    size_t dataSize;
    //! Признак, должен ли паттерн быть жестко привязан к началу строки
    bool isLeftAnchor;
    //! Признак, должен ли паттерн быть жестко привязан к концу строки
    bool isRightAnchor;
    //! Указатель на след. ноду
    CPatternNode *nextNode;
};

class CLogReader
{
public:
    /*!
     * \brief Конструктор
     */
    CLogReader();

    /*!
     * \brief Деструктор
     *
     * Вызывает метод Close() для корректного освобождения ресурсов.
     */
    ~CLogReader();

    /*!
     * \brief Открывает файл для чтения
     * \param fileName Имя файла
     * \return false - ошибка
     *
     * Производит открытие текствого файла. При filename == NULL или ошибке открытия возвращает false, в противном случае true.
     */
    bool Open( const char* fileName );

    /*!
     * \brief Освобождение ресурсов
     */
    void Close();

    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедет производится поиск
     * \return false - ошибка
     *
     * Производит предобработку и установку шаблона поиска.
     */
    bool SetFilter( const char* filter );

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
    bool GetNextLine( char* buf, const int bufsize );

private:
    /*!
     * \brief Проверка на равенство двух символов, а также на то, что rSymbol не является '?'
     * \param lSymbol Левый проверяемый символ, эталон при сравнении
     * \param rSymbol Проверяемый символ
     * \return true если lSymbol == rSymbol или rSymbol является '?'
     */
    static bool IsEqual( char lSymbol, char rSymbol );

    /*!
     * \brief Добавление узла в скомпилированный паттерн поиска (односвязанный список)
     * \param type Тип узла ('*', '?' или текст)
     * \param data Содержимое узла
     * \param text_length Размер содержимого
     * \param is_start Признак, должен ли паттерн быть жестко привязан к началу строки
     * \param is_end Признак, должен ли паттерн быть жестко привязан к концу строки
     */
    void AddNode(type_t type, const char *data, const size_t dataSize, const bool isLeftAnchor, const bool isRightAnchor);

    /*!
     * \brief Функция проверки принадлежности строки поисковому паттерну
     * \param string Проверяемая строка
     * \param compiledPattern Указатель на начало односвязного списка с компилированным паттерном поиска
     * \return true если строка удовлетворяет критериям поиска
     */
    static bool Match(char *string, struct CPatternNode *compiledPattern);

private:
    //! Гранулярность
    DWORD m_systemGran;

    //! Дескриптор открытого файла
    LARGE_INTEGER m_fileSize;

    //! Дескриптор объекта проекции файла
    HANDLE m_fileMapping;

    //! Полученный фильтр
    char* m_searchFilter;

    //! Буфер по которому производится поиск
    char* m_buffer;

    //! Смещение в файле при мапировании
    LONGLONG m_fileOffset;

    //! Смапированный регион
    LPVOID m_mapingRegion;

    //! Первый элемент скомпилированного паттерна поиска
    CPatternNode* m_startPattern;

    //! Последний элемент скомпилированного паттерна поиска
    CPatternNode* m_endPattern;
};

