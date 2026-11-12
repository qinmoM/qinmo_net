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

    if (position_.end() == position_.find(n))
        return false;

    return true;
}


bool Headers::getSingle(qinmo::StringView name, qinmo::StringView& value) const
{
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    auto it = position_.find(n);
    if (position_.end() == it || it->second.size() > 1)
        return false;

    auto& str = headers_.at(it->second.front()).second;
    value = qinmo::StringView(str.data(), str.size());
    return true;
}

std::vector<qinmo::StringView> Headers::getAll(qinmo::StringView name) const
{
    std::string n(name.data(), name.size());
    for (char& c : n)
        c = qinmo::toLower(c);

    auto it = position_.find(n);
    std::vector<qinmo::StringView> result;

    if (position_.end() == it)
        return result;

    result.reserve(it->second.size());
    for (std::size_t index : it->second)
        result.emplace_back(headers_[index].second.data(), headers_[index].second.size());

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

    position_[n].push_back(headers_.size());
    headers_.emplace_back(n, v);
}

} // namespace qinmo::net::http
