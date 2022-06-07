#include <gtest/gtest.h>
#include <traact/util/Logging.h>

int main(int argc, char **argv) {

    traact::util::initLogging(spdlog::level::trace, "");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}