#include <gtest/gtest.h>
#include <traact/util/Logging.h>

int main(int argc, char **argv) {

    traact::util::initLogging(spdlog::level::warn, "");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}