#include <catch2/catch_session.hpp>
#include "Common.h"

int main(int argc, const char* argv[] ) {
    Registry::Create();
    int result = Catch::Session().run(argc, argv);
    Registry::Destroy();
    return result;
}