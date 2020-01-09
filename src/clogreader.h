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
     * \brief Функция проверки принадлежности строки поисковому паттерну
     * \param string Проверяемая строка
     * \return true если строка удовлетворяет критериям поиска
     */
	bool Match(char *string);
	
	/*!
     * \brief Добавление узла в скомпилированный паттерн поиска (односвязанный список)
     * \param type Тип узла ('*', '?' или текст)
     * \param data Содержимое узла
     * \param text_length Размер содержимого
     * \param is_start Признак, должен ли паттерн быть жестко привязан к началу строки
     * \param is_end Признак, должен ли паттерн быть жестко привязан к концу строки
	 * \return true в случае успеха
     */
	bool AddNode(type_t type, const char *data, const size_t dataSize, const bool isLeftAnchor, const bool isRightAnchor);

	/*!
     * \brief Отображение подробностей ошибок, вызванных функциями WinAPI
     * \param methodName Имя метода CLogReader, в котором произошла ошибка
     * \param functionName Имя функции, вызвавшей ошибку
     */
	void ErrorMsg(const char *methodName, const char *functionName);
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

	//! Текущий исследуемый элемент паттерна поиска
	CPatternNode* m_itemPattern;

	//! Указатель на найденную подстроку (ищем по элементу паттерна поиска) в исследуемой сроке
	char *m_substrPtr;
	
	//! Смещение в исследуемой строке
	size_t m_stringOffset;


	LARGE_INTEGER m_mapOffset;
    LARGE_INTEGER m_mapSize;
    DWORD m_bufferPosition;
    int m_stringPosition;
    LONGLONG m_mapDelta;
    LONGLONG m_mapRegionSize;
    LONGLONG m_availableBytes;
};

