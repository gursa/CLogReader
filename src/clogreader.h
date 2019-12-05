#ifndef CLOGREADER_H
#define CLOGREADER_H

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <regex.h>

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
     *
     * Инициализирует указатель m_tokens NULL-ом.
     */
    CLogReader();

    /*!
     * \brief Деструктор
     *
     * Очищает выделенную память под токены m_tokens. Вызывает метод Close() для корректного закрытия файла.
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
     * \brief Закрытие текстового файла
     *
     * Закрывает текстиовый файл, если соответствующий дескриптор m_file != NULL.
     */
    void Close();

    /*!
     * \brief Установка фильтра строк
     * \param filter Шаблон по которому бедет производится поиск
     * \return false - ошибка
     *
     * Производит установку шаблона поиска. Для этого вызывает prepare_pattern для подготовки паттерна и последующей компиляции в токены поиска m_tokens.
     * Если filter == NULL или происходит ошибка при выделении памяти под паттерн возвращается false. В противном случае возвращается результат prepare_pattern.
     */
    bool SetFilter(const char *filter);

    /*!
     * \brief Запрос очередной найденной строки
     * \param buf Буфер
     * \param bufsize Максимальная длина
     * \return false - конец файла или ошибка
     *
     * Производит запрос очередной строки, удовлетворяющей токенам m_tokens. Реализовано через построчное чтение файла с помощью fgets и последующего
     * сравнение с токенами посредством функции search. В случае если search вернула совпадение функция завершается с true, в противном случае чтение продолжается.
     * false возвращается если buf == NULL или bufsize == 0 или файловый дескриптор m_file не инициализирован соответствующим fopen в функции Open или происходят ошибки чтения,
     * а также если достигнут конец файла.
     */
    bool GetNextLine(char *buf, const int bufsize);
private:
    /*!
     * \brief Конвертация шаблона из внешнего мира во внутренний паттерн
     * \param filter Шаблон
     * \param filter_length Длина шаблона
     * \param pattern Внутренний паттерн
     * \return false если есть ошибка в аргументах, в противном случае результат compile_pattern
     *
     * Производит убирание задвоенных звездочек ('*'), а также добавляет якоря начала ('^') и конца ('$') шаблона, если шаблон начинается и заканчивается с букв/цифр соответственно.
     * Возвращает false если filter == NULL или filter_length == 0 или pattern == NULL, в противном случае результат compile_pattern
     */
    bool prepare_pattern(const char *filter, const size_t filter_length, char *pattern);

    /*!
     * \brief Компиляция паттерна в токены
     * \param pattern Паттерн по которому будет производится поиск
     * \return true - если компиляция шаблона прошла успешно, false в обратном случае
     * \warning Если в паттерне присутствуют символы-классы (то что заключено между квадратных скобок '[' и ']'), то их длина не должна превышать MAX_CLASSES_BUFFER_LEN символов.
     *
     */
    bool compile_pattern(const char *pattern);

    /*!
     * \brief Конвертирование экраннированного символа (то что следует после '\') в токены
     * \param pattern Паттерн по которому будет производится поиск
     * \param tokens_compiled Токены, в которые производится конвертация
     * \param i Индекс в паттерне pattern, с которого производитсяч анализ
     * \param j Индекс в массиве токенов tokens_compiled, в который будет прообразован символ-класса
     * \return false если есть ошибка в аргументах
     *
     * Находится следующий символ за '\' и анализируется. Возвращается false если pattern == NULL или tokens_compiled == NULL, в противном случае true.
     */
    bool make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j);

    /*!
     * \brief Конвертирование символа-класса (то что заключено между квадратных скобок '[' и ']') в токены
     * \param pattern Паттерн по которому будет производится поиск
     * \param tokens_compiled Токены, в которые производится конвертация
     * \param i Индекс в паттерне pattern, с которого производитсяч анализ
     * \param j Индекс в массиве токенов tokens_compiled, в который будет прообразован символ-класса
     * \param class_buffer Буффер в котором накапливаются все символы, содержащиеся в символ-классе
     * \param class_buffer_index Индекс по class_buffer
     * \return false - Ошибка
     *
     * Находится вхождение между '[' и ']' и присваиваются regex_utility::regex_t::data::class_ptr. В случае успеха, возвращается true, в противном случае возвращается false.
     */
    bool make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j, char *class_buffer, size_t &class_buffer_index);

    /*!
     * \brief Поиск шаблона в строке
     * \param text Текст, в котором производится поиск
     * \param found Индекс найденного в строке шаблона (начиная с 0). Если вхождений не найдено, то будет возвращенно -1.
     * \return false - ошибка или совпадений не найдено
     */
    bool search(const char *text, int &found);
private:
    //! Токены шаблона
    struct regex_utility::regex_t *m_tokens;

    //! Файл, по которому производится поиск
    FILE *m_file;

    //! Номер строки в файле
    size_t m_strnum;
};

#endif // CLOGREADER_H
