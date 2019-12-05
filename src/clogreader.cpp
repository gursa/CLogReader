#include "clogreader.h"

CLogReader::CLogReader()
    : m_tokens(NULL), m_file(NULL), m_strnum(0)
{    
}

CLogReader::~CLogReader()
{
    Close();

    if(m_tokens != NULL)
        free(m_tokens);
}

bool CLogReader::Open(const char *filename)
{
    if(filename == NULL) {
        fprintf(stderr, "%s(): Error in args! filename = %p", __FUNCTION__, filename);
        return false;
    }

    m_file = fopen(filename, "r");
    if (!m_file) {
        fprintf(stderr, "Unable to open file %s", filename);
        return false;
    }

    return true;
}

void CLogReader::Close()
{
    if(m_file != NULL)
        fclose(m_file);
}

bool CLogReader::SetFilter(const char *filter)
{
    if(filter == NULL) {
        fprintf(stderr, "%s(): Error in args! filter = %p", __FUNCTION__, filter);
        return false;
    }

    size_t length = strlen(filter);
    char *pattern = (char*) calloc(length+3, sizeof (char));
    if(pattern == NULL) {
        fprintf(stderr, "%s(): Cann't allocate memory for pattern!", __FUNCTION__);
        return false;
    }

    bool rc = prepare_pattern(filter, length, pattern);

    free(pattern);

    return rc;
}

bool CLogReader::GetNextLine(char *buf, const int bufsize)
{
    if((buf == NULL) || (bufsize <= 0)) {
        fprintf(stderr, "%s(): Error in args! buf = %p, bufsize = %d", __FUNCTION__, buf, bufsize);
        return false;
    }

    memset(buf, '\0', bufsize);

    if(m_file == NULL) {
        fprintf(stderr, "%s(): The file is not open!", __FUNCTION__);
        return false;
    }

    int found = -1;

    while(!feof(m_file)) {
        if(fgets(buf, bufsize, m_file) == NULL) {
            if(ferror(m_file)) {
                fprintf(stderr, "%s(): File read error caught!", __FUNCTION__);
                clearerr(m_file);
                Close();
                return false;
            }

        }
        m_strnum++;
        found = -1;
        if(search(buf, found)) {
            fprintf(stdout, "Match %06zu\n", m_strnum);
            return true;
        }

    }

    fprintf(stderr, "%s(): Caught the end of the file!\n", __FUNCTION__);
    return false;
}

bool CLogReader::prepare_pattern(const char *filter, const size_t filter_length, char *pattern)
{
    if((filter == NULL) || (filter_length == 0) || (pattern == NULL)) {
        fprintf(stderr, "%s(): Error in args! filter = %p, filter_length = %zu, pattern = %p", __FUNCTION__, (void*)filter, filter_length, (void*)pattern);
        return false;
    }

    //Предыдущий символ в фильтре
    char c_old = '*';
    // Текущий символ в фильтре
    char symbol;
    // Индекс в фильтре
    size_t i = 0;
    // Индекс в подготовленном паттерне
    size_t j = 0;

    while (i < filter_length)
    {
        symbol = filter[i];
        if(symbol == '*'){
            if(c_old == '*') {
                i++;
                continue;
            }
            pattern[j] = symbol;
        } else {
            if(regex_utility::isAlphaNumeric(symbol) && (i == 0)) {
                pattern[j] = '^';
                j++;
            }
            pattern[j] = symbol;
        }


        i++;
        j++;
        c_old = symbol;
    }


    if(regex_utility::isAlphaNumeric(pattern[j-1]))
        pattern[j] = '$';

    return compile_pattern(pattern);
}

bool CLogReader::compile_pattern(const char *pattern)
{
    if(pattern == NULL) {
        fprintf(stderr, "%s(): Error in args! pattern = %p", __FUNCTION__, (void*)pattern);
        return false;
    }

    size_t patern_length = strlen(pattern);
    regex_utility::regex_t *tokens_compiled = (regex_utility::regex_t*) calloc(patern_length+1, sizeof (regex_utility::regex_t));
    if(tokens_compiled == NULL) {
        fprintf(stderr, "%s(): Cann't allocate memory for tokens!", __FUNCTION__);
        return false;
    }

    static char class_buffer[MAX_CLASSES_BUFFER_LEN];
    size_t class_buffer_index = 1;

    // Текущий символ в паттерне
    char c;
    // Индекс в паттерне
    size_t i = 0;
    // Индекс текущего токена
    size_t j = 0;

    while (pattern[i] != '\0' && (j+1 < patern_length))
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
            if(!make_character_classes(pattern, tokens_compiled, i, j, class_buffer, class_buffer_index)) {
                free(tokens_compiled);
                return false;
            }
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

bool CLogReader::make_escaped_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j)
{
    if((pattern == NULL) || (tokens_compiled == NULL)) {
        fprintf(stderr, "%s(): Error in args! pattern = %p, tokens_compiled = %p", __FUNCTION__, (void*)pattern, (void*)tokens_compiled);
        return false;
    }

    // Проверяем, не был ли символ '\\' последним в строке. Если это так, то работаем
    if (pattern[i+1] != '\0')
    {
        // Сам начальный символ escape-последовательности '\\' нам не интересен, сдвигаем индекс по паттерну
        i++;
        // И продолжаем анализ ...
        switch (pattern[i])
        {
        // Спец. символы
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
        // Остальные пойдут как просто текст
        default:
            tokens_compiled[j].type = regex_utility::regex_t::typeChar;
            tokens_compiled[j].data.symbol = pattern[i];
            break;
        }
    }
    return true;
}

bool CLogReader::make_character_classes(const char *pattern, regex_utility::regex_t *tokens_compiled, size_t &i, size_t &j, char *class_buffer, size_t &class_buffer_index)
{
    if((pattern == NULL) || (tokens_compiled == NULL) || (class_buffer == NULL)) {
        fprintf(stderr, "%s(): Error in args! pattern = %p, tokens_compiled = %p, class_buffer = %p", __FUNCTION__, (void*)pattern, (void*)tokens_compiled, (void*)class_buffer);
        return false;
    }

    // Запоминаем индекс с началом содержимого внутри кв. скобок
    size_t buf_begin = class_buffer_index;

    tokens_compiled[j].type = regex_utility::regex_t::typeClass;

    // Копируем содержимое внутри скобок. Закрывающую ] коипровать не будем.
    while ((pattern[++i] != ']') && (pattern[i]   != '\0'))
    {
        // Если вдруг встретили спецсимвол, то опускаем его.
        if (pattern[i] == '\\')
        {
            // Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
            if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN - 1)
            {
                fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a special character is %d, current index is %zu\n", MAX_CLASSES_BUFFER_LEN - 2, class_buffer_index);
                fprintf(stderr, "pattern = '%s'\n", pattern);
                return false;
            }
            // Если переполнения нет, то копируем символ
            class_buffer[class_buffer_index++] = pattern[i++];
        }
        else if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN) //! Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
        {
            fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a character is %d, current index is %zu\n", MAX_CLASSES_BUFFER_LEN-1, class_buffer_index);
            fprintf(stderr, "pattern = '%s'\n", pattern);
            return false;
        }
        // Если переполнения нет, то копируем символ
        class_buffer[class_buffer_index++] = pattern[i];
    }
    // Если стартовый индекс вышел за пределы допустимого выходим с ошибкой
    // Отлавливаем случаи подобные [01234567890123456789012345678901234567][
    if (class_buffer_index >= MAX_CLASSES_BUFFER_LEN)
    {
        fprintf(stderr, "Error! Overflow of the internal buffer! The maximum index of a character is %d, current index is %zu\n", MAX_CLASSES_BUFFER_LEN-1, class_buffer_index);
        fprintf(stderr, "pattern = '%s'\n", pattern);
        return false;
    }
    // Нуль-терминируем
    class_buffer[class_buffer_index++] = 0;
    tokens_compiled[j].data.class_ptr = &class_buffer[buf_begin];

    return true;
}

bool CLogReader::search(const char *text, int &found)
{
    // Поиск будем производить только если у нас есть токены и текст
    if ((m_tokens != NULL) || (text != NULL))
    {
        // Если мы работаем с жесткой привязкой к началу строки паттерна
        if (m_tokens[0].type == regex_utility::regex_t::typeBegin)
        {
            return ((matchPattern(&m_tokens[1], text)) ? 0 : -1);
        }
        else // Если начальная позиция вхождения нам не важна
        {
            // На всякий случай инициализируем индекс, с которого начинается вхождение найденного паттерна, ошибочным значением -1
            found = -1;
            // Крутим цикл до тех пор пока не встретим символ нуль-терминации
            do
            {
                found++;

                if (matchPattern(m_tokens, text))
                {
                    // Если первый же символ в строке нулевой у нас проблема, выходим с ошибкой
                    if (text[0] == '\0') {
                        found = -1;
                        return false;
                    }
                    // Если же все ОК
                    return true;
                }
            }
            while (*text++ != '\0');
        }
    }
    return false;
}
