#pragma once

#include <cfile.h>

/*!
    \brief Класс, реализуюший проецирование файла в память
    \author Михаил Макаров
    \date Декабрь 2019 года

    Работа начинается с вызова метода CreateMappingObj для получения объекта мапированного файла.
    Затем вызывается GetString, который отдает нам строку, ограниченную либо символом переноса строки, либо 2*гранулярность
*/
class CFileMapper : public CFile
{
public:
    /*!
     * \brief Конструктор инициализирует внутренние поля, а также достает информацию о гранулярности
     */
    CFileMapper();

    /*!
     * \brief Деструктор корректно освобождает ресурсы посредством вызова DestroyMappingObj
     */
    ~CFileMapper();

    /*!
     * \brief Открытие файла лога, получение его размера, а также объекта смапированного в память файла
     * \param fileName Имя файла лога
     * \return true в случае успешного завершения операции
     */
    bool CreateMappingObj(const char *fileName);

    /*!
     * \brief Корректное освобождение ресурсов
     * \return true в случае успешного завершения операции
     */
    bool DestroyMappingObj();

    /*!
     * \brief Получение строки из мапированного региона
     * \param buffer Буфер, полученная строка
     * \param bufferSize Размер буфера
     * \return true в случае успеха
     */
    bool GetString(char *buffer, const int bufferSize);

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

private:
    //! Вспомогательная структура, используется для получении информации о гранулярности
    SYSTEM_INFO m_systemInfo;

    //! Дескриптор объекта проекции файла
    HANDLE m_fileMapping;

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
};
