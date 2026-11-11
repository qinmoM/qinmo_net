#pragma once

#include "qinmo/tool/StringView.h"
#include <vector>

namespace qinmo::net::http
{

class Response;
    
class Headers
{
public:
    using HeadersType = std::vector<std::pair<std::string, std::string>>;

public:
    bool has(qinmo::StringView name) const;

    bool getSingle(qinmo::StringView name, qinmo::StringView& value) const;
    std::vector<qinmo::StringView> getAll(qinmo::StringView name) const;


    void add(qinmo::StringView name, qinmo::StringView value);

private:
    HeadersType headers_;
};
} // qinmo::net::http