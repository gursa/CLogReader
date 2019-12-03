#include <cstdio>
#include <cstddef>
#include "regex.h"

//! Максимально количество токенов
#define MAX_TOKENS_COUNT 100
//! Размер буфера для символов из всех выражений в квадратных скобках
#define MAX_CLASSES_BUFFER_LEN 40

regex::CRegex::CRegex()
    : m_tokens(NULL)
{
}

bool regex::CRegex::compile(const char *pattern)
{
    static regex_utility::regex_t tokens_compiled[MAX_TOKENS_COUNT];
    static unsigned char class_buffer[MAX_CLASSES_BUFFER_LEN];
    int class_buffer_index = 1;

    // Текущий символ в паттерне
    char c;
    // Индекс в паттерне
    int i = 0;
    // Индекс текущего токена
    int j = 0;

    while (pattern[i] != '\0' && (j+1 < MAX_TOKENS_COUNT))
    {
        c = pattern[i];
        switch (c)
        {
        // Спец. символы
        case '^':
            tokens_compiled[j].type = regex_utility::regex_t::typeBegin;
            break;
        case '$':
            tokens_compiled[j].type = regex_utility::regex_t::typeEnd;
            break;
        case '.':
            tokens_compiled[j].type = regex_utility::regex_t::typeDot;
            break;
        case '*':
            tokens_compiled[j].type = regex_utility::regex_t::typeAsterisk;
            break;
        case '+':
            tokens_compiled[j].type = regex_utility::regex_t::typePlus;
            break;
        case '?':
            tokens_compiled[j].type = regex_utility::regex_t::typeQuestion;
            break;
        // Экранированные символы-классы (\s \w ...)
        case '\\':
            make_escaped_character_classes(pattern, tokens_compiled, i, j);
            break;
        // Символы-классы (то что заключено в [])
        case '[':
            if(!make_character_classes(pattern, tokens_compiled, i, j, class_buffer, class_buffer_index))
                return false;
            break;
        // Все остальное
        default:
            tokens_compiled[j].type = regex_utility::regex_t::typeChar;
            tokens_compiled[j].data.symbol = c;
            break;
        }
        i++;
        j++;
    }
    //!* Последнему токену присваеваем тип regex_utility::regex_t::typeUnused - свидетельство окончание паттерна
    tokens_compiled[j].type = regex_utility::regex_t::typeUnused;

    m_tokens = tokens_compiled;
    return true;
}

int regex::CRegex::search(const char *text)
{
    //! Поиск будем производить только если у нас есть токены и текст
    if ((m_tokens != NULL) || (text != NULL))
    {
        //! Если мы работаем с жесткой привязкой к началу строки паттерна
        if (m_tokens[0].type == regex_utility::regex_t::typeBegin)
        {
            return ((matchPattern(&m_tokens[1], text)) ? 0 : -1);
        }
        else //! Если начальная позиция вхождения нам не важна
        {
            //! На всякий случай инициализируем индекс, с которого начинается вхождение найденного паттерна, ошибочным значением -1
            int found = -1;
            //! Крутим цикл до тех пор пока не встретим символ нуль-терминации
            do
            {
                found++;

                if (matchPattern(m_tokens, text))
                {
                    //! Если первый же символ в строке нулевой у нас проблема, выходим с ошибкой
                    if (text[0] == '\0')
                        return -1;
                    //! Если же все ок  - возвращаем индекс, откуда начинается вхождение
                    return found;
                }
            }
            while (*text++ != '\0');
        }
    }
    return -1;
}

void regex::CRegex::make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, int &i, int &j)
{
    //! Проверяем, не был ли символ '\\' последним в строке. Если это так, то работаем
    if (pattern[i+1] != '\0')
    {
        //! Сам начальный символ escape-последовательности '\\' нам не интересен, сдвигаем индекс по паттерну
        i++;
        //! И продолжаем анализ ...
        switch (pattern[i])
        {
        //! Спец. символы
        case 'd':
            tokens_compiled[j].type = regex_utility::regex_t::typeDigit;
            break;
        case 'D':
            tokens_compiled[j].type = regex_utility::regex_t::typeNotDigit;
            break;
        case 'w':
            tokens_compiled[j].type = regex_utility::regex_t::typeAlpha;
            break;
        case 'W':
            tokens_compiled[j].type = regex_utility::regex_t::typeNotAlpha;
            break;
        case 's':
            tokens_compiled[j].type = regex_utility::regex_t::typeWhitespace;
            break;
        case 'S':
            tokens_compiled[j].type = regex_utility::regex_t::typeNotWhitespace;
            break;
        //! Остальные пойдут как просто текст
        default:
            tokens_compiled[j].type = regex_utility::regex_t::typeChar;
            tokens_compiled[j].data.symbol = pattern[i];
            break;
        }
    }
}

bool regex::CRegex::make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, int &i, int &j, unsigned char *class_buffer, int &class_buffer_index)
{
    if((pattern == NULL) || (tokens_compiled == NULL) || (class_buffer == NULL))
        return false;

    //! Запоминаем индекс с началом содержимого внутри кв. скобок
    int buf_begin = class_buffer_index;

    tokens_compiled[j].type = regex_utility::regex_t::typeClass;

    //! Копируем содержимое внутри скобок. Закрывающую ] коипровать не будем.
    while ((pattern[++i] != ']') && (pattern[i]   != '\0'))
    {
        //! Если вдруг встретили спецсимвол, то опускаем его.
        if (pattern[i] == '\\')
        {
            //! Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
            if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN - 1)
            {
                fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a special character is %d, current index is %d\n", MAX_CLASSES_BUFFER_LEN - 2, class_buffer_index);
                fprintf(stderr, "pattern = '%s'\n", pattern);
                return false;
            }
            //! Если переполнения нет, то копируем символ
            class_buffer[class_buffer_index++] = pattern[i++];
        }
        else if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN) //! Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
        {
            fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a character is %d, current index is %d\n", MAX_CLASSES_BUFFER_LEN-1, class_buffer_index);
            fprintf(stderr, "pattern = '%s'\n", pattern);
            return false;
        }
        //! Если переполнения нет, то копируем символ
        class_buffer[class_buffer_index++] = pattern[i];
    }
    //! Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
    //! Отлавливаем случаи подобные [01234567890123456789012345678901234567][
    if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN)
    {
        fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a character is %d, current index is %d\n", MAX_CLASSES_BUFFER_LEN-1, class_buffer_index);
        fprintf(stderr, "pattern = '%s'\n", pattern);
        return false;
    }
    //! Нуль-терминируем
    class_buffer[class_buffer_index++] = 0;
    tokens_compiled[j].data.class_ptr = &class_buffer[buf_begin];

    return true;
}
