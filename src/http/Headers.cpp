#include "qinmo/net/http/Headers.h"

namespace qinmo::net::http
{

bool Headers::isTokenChar(char c)
{
    if (qinmo::isDigit(c) || qinmo::isAlpha(c))
        return true;

    switch (c)
    {
        case '!':
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '*':
        case '+':
        case '-':
        case '.':
        case '^':
        case '_':
        case '`':
        case '|':
        case '~':
            return true;
        default:
            return false;
    }
}


bool Headers::has(qinmo::StringView name) const
{
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    for (auto& field : headers_)
    {
        if (field.first == n)
            return true;
    }

    return false;
}


bool Headers::getSingle(qinmo::StringView name, qinmo::StringView& value) const
{
    std::size_t count = 0;
    qinmo::StringView temp;
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    for (auto& field : headers_)
    {
        if (field.first == n)
        {
            if (0 != count)
                return false;

            ++count;
            temp = qinmo::StringView(field.second.data(), field.second.size());
        }
    }

    if (0 == count)
        return false;

    value = temp;
    return true;
}

std::vector<qinmo::StringView> Headers::getAll(qinmo::StringView name) const
{
    std::vector<qinmo::StringView> result;
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    for (auto& field : headers_)
    {
        if (field.first == n)
            result.emplace_back(field.second.data(), field.second.size());
    }

    return result;
}



void Headers::add(qinmo::StringView name, qinmo::StringView value)
{
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    std::string v(value.data(), value.size());
    for (char& c : v)
        c = qinmo::toLower(c);

    headers_.emplace_back(n, v);
}

} // namespace qinmo::net::http
