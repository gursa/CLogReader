#pragma once

#include <windows.h>

/*!
 * \brief Отображение текста ошибки по коду ::GetLastError()
 * \param methodName Имя метода, где произошла ошибка
 * \param functionName Имя функции, вызвавшей ошибку
 */
void ErrorMsg(const char *methodName, const char *functionName);

/*!
    \brief Класс, реализуюший примитивы работы с файлом
    \author Михаил Макаров
    \date Декабрь 2019 года

    Работа начинается с вызова метода Open для открытия файла посредством CreateFile.
*/
class CFile
{
public:
    /*!
     * \brief Конструктор инициализирует переменные
     */
    CFile();

    /*!
     * \brief Деструктор корректно освобождает ресурсы посредством вызова Close
     */
    ~CFile();

    /*!
     * \brief Открытие файла и получение его размера
     * \param fileName Имя файла
     * \return true в случае успешного завершения операции
     */
    bool Open(const char *fileName);

    /*!
     * \brief Корректно освобождает ресурсы
     * \return true в случае успешного завершения операции
     */
    bool Close();

    /*!
     * \brief Возвращает декскриптор на открытый файл
     * \return Дескриптор
     */
    HANDLE FileHandle() const;

    /*!
     * \brief Возвращает размер файла
     * \return Размер файла
     */
    LARGE_INTEGER FileSize() const;
private:
    //! Дескриптор открытого файла
    HANDLE m_fileHandle;

    //! Размер файла
    LARGE_INTEGER m_fileSize;
};
