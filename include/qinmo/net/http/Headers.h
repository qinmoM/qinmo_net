#pragma once

#include "qinmo/tool/StringView.h"
#include <vector>
#include <unordered_map>

namespace qinmo::net::http
{

class Response;

/// @brief Contain-Only, skip validity check
class Headers
{
public:
    using HeadersType = std::vector<std::pair<std::string, std::string>>;


    static bool isTokenChar(char c);
    static bool isValidValue(unsigned char c);

public:
    /// @brief check whether name exists
    bool has(qinmo::StringView name) const;

    /// @return return false if name does not exists or is not a single name
    bool getSingle(qinmo::StringView name, qinmo::StringView& value) const;
    /// @brief obtain all values of this name
    std::vector<qinmo::StringView> getAll(qinmo::StringView name) const;


    /// @brief insert a field
    void add(qinmo::StringView name, qinmo::StringView value);

private:
    HeadersType headers_;
    std::unordered_map<std::string, std::vector<std::size_t>> position_;

};
} // qinmo::net::http