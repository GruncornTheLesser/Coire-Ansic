#pragma once
#include "fwd.h"

namespace ecs {
    class entity  {
    public:
        unsigned long int value;
        constexpr entity() : value(-1) { }
        entity(unsigned long int v) : value(v) { }
        constexpr operator unsigned long int() { return value; }
    };

    entity tombstone;
}