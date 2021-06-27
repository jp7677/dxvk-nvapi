#define CATCH_CONFIG_MAIN
#include "../inc/catch.hpp"
#include "../inc/catch2/trompeloeil.hpp"

extern template struct trompeloeil::reporter<trompeloeil::specialized>;
