#include "regex_utility.h"

int regex_utility::isSpecialSymbol(char symbol)
{
    return ((symbol == 's') || (symbol == 'S') || (symbol == 'w') || (symbol == 'W') || (symbol == 'd') || (symbol == 'D'));
}

int regex_utility::isAlpha(char symbol)
{
    return ((symbol >= 'a') && (symbol <= 'z')) || ((symbol >= 'A') && (symbol <= 'Z'));
}

int regex_utility::isAlphaNumeric(char symbol)
{
    return ((symbol == '_') || isAlpha(symbol) || isDigit(symbol));
}

int regex_utility::matchClass(char symbol, const char *text)
{
    do
    {
        if (matchRange(symbol, text))
        {
            return 1;
        }
        else if (text[0] == '\\')
        {
            // Нашли начало спец. символа. Сдвигаемся по строке и сравниваем следующий символ.
            text += 1;
            if (matchSpecialSymbol(symbol, text))
                return 1;
            else if ((symbol == text[0]) && !isSpecialSymbol(symbol))
                return 1;
        }
        else if (symbol == text[0])
        {
            if (symbol == '-')
                return ((text[-1] == '\0') || (text[1] == '\0'));
            else
                return 1;
        }
    }
    while (*text++ != '\0');

    return 0;
}

int regex_utility::isDigit(char symbol)
{
    return ((symbol >= '0') && (symbol <= '9'));
}

int regex_utility::matchSpecialSymbol(char symbol, const char *text)
{
    switch (text[0])
    {
        case 'd': return  isDigit(symbol);
        case 'D': return !isDigit(symbol);
        case 'w': return  isAlphaNumeric(symbol);
        case 'W': return !isAlphaNumeric(symbol);
        case 's': return  isWhitespace(symbol);
        case 'S': return !isWhitespace(symbol);
        default:  return (symbol == text[0]);
    }
}

int regex_utility::matchOne(regex_t token_item, char symbol)
{
    switch (token_item.type)
    {
    case regex_t::typeDot:
        return 1;
    case regex_t::typeClass:
        return  matchClass(symbol, (const char*)token_item.data.class_ptr);
    case regex_t::typeDigit:
        return  isDigit(symbol);
    case regex_t::typeNotDigit:
        return !isDigit(symbol);
    case regex_t::typeAlpha:
        return  isAlphaNumeric(symbol);
    case regex_t::typeNotAlpha:
        return !isAlphaNumeric(symbol);
    case regex_t::typeWhitespace:
        return  isWhitespace(symbol);
    case regex_t::typeNotWhitespace:
        return !isWhitespace(symbol);
    default:
        return  (token_item.data.symbol == symbol);
    }
}

int regex_utility::matchPattern(regex_t *tokens, const char *text)
{
    do
    {
        if ((tokens[0].type == regex_t::typeUnused) || (tokens[1].type == regex_t::typeQuestion))
            return matchQuestion(tokens[0], &tokens[2], text);
        else if (tokens[1].type == regex_t::typeAsterisk)
            return matchAsterisk(tokens[0], &tokens[2], text);
        else if (tokens[1].type == regex_t::typePlus)
            return matchPlus(tokens[0], &tokens[2], text);
        else if ((tokens[0].type == regex_t::typeEnd) && tokens[1].type == regex_t::typeUnused)
            return (text[0] == '\0');
    }
    while ((text[0] != '\0') && matchOne(*tokens++, *text++));

    return 0;
}

int regex_utility::matchPlus(regex_t token_item, regex_t *tokens, const char *text)
{
    while ((text[0] != '\0') && matchOne(token_item, *text++))
    {
        if (matchPattern(tokens, text))
            return 1;
    }
    return 0;
}

int regex_utility::matchQuestion(regex_t token_item, regex_t *tokens, const char *text)
{
    if (token_item.type == regex_t::typeUnused)
        return 1;
    if (matchPattern(tokens, text))
        return 1;
    if (*text && matchOne(token_item, *text++))
        return matchPattern(tokens, text);
    return 0;
}

int regex_utility::matchRange(char symbol, const char *text)
{
    return ((symbol != '-') &&
            (text[0] != '\0') && (text[0] != '-') && (text[1] == '-') && (text[1] != '\0') && (text[2] != '\0') &&
            ((symbol >= text[0]) && (symbol <= text[2])));
}

int regex_utility::matchAsterisk(regex_t token_item, regex_t *tokens, const char *text)
{
    do
    {
        if (matchPattern(tokens, text))
            return 1;
    }
    while ((text[0] != '\0') && matchOne(token_item, *text++));

    return 0;
}

int regex_utility::isWhitespace(char symbol)
{
    return ((symbol == ' ') || (symbol == '\t') || (symbol == '\n') || (symbol == '\r') || (symbol == '\f') || (symbol == '\v'));
}
