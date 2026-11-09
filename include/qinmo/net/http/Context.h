#pragma once

#include "qinmo/net/PacketBuffer.h"

namespace qinmo::net::http
{

class Context
{
public:
    bool parse(qinmo::net::PacketBuffer& buffer);
    bool isParsed() const;

    void reset();
};
} // namespace qinmo::net::http
