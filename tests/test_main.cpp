#include <gtest/gtest.h>
#include <traact/util/Logging.h>

int main(int argc, char **argv) {

    traact::util::init_logging(spdlog::level::trace, false, "");

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

}