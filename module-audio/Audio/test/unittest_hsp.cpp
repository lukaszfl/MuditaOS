#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "MockEndpoint.hpp"

#include <Audio/StreamFactory.hpp>

#include <chrono>

using namespace std::chrono_literals;
using Span = ::audio::Stream::Span;
using ::testing::Return;

// TODO: get constants from production code
constexpr auto routerConstraint = 2ms;
constexpr auto cellularTraits   = ::audio::Endpoint::Traits{};
constexpr auto btTraits         = ::audio::Endpoint::Traits{.blockSizeConstraint = 64U, .timeConstraint = 16ms};
constexpr auto cellularFormat   = ::audio::AudioFormat(16000, 16, 1);
constexpr auto btFormat         = ::audio::AudioFormat(8000, 16, 1);

void cellularWrite(Span &span)
{
    for (unsigned int i = 0; i < span.dataSize; i++) {
        span.data[i] = i & 0xFF;
    }
}

TEST(HSP, InitInput)
{
    ::audio::StreamFactory factory(routerConstraint);
    ::testing::audio::MockSink sink;
    ::testing::audio::MockSource source;

    EXPECT_CALL(source, getTraits).WillRepeatedly(Return(cellularTraits));
    EXPECT_CALL(source, getSourceFormat).WillRepeatedly(Return(cellularFormat));

    EXPECT_CALL(sink, getTraits).WillRepeatedly(Return(btTraits));
    EXPECT_CALL(sink, getSinkFormat).WillRepeatedly(Return(btFormat));

    auto stream = factory.makeStream(source, sink);

    Span span;
    unsigned int blocks = 0;

    // fill the stream
    do {
        stream->commit();
        if (!stream->reserve(span)) {
            break;
        }
        blocks++;
        ASSERT_EQ(span.dataSize, 128);
        cellularWrite(span);
    } while (true);
    ASSERT_EQ(span.dataSize, 128);

    // read data
    do {
        if (!stream->peek(span)) {
            break;
        }
        ASSERT_EQ(span.dataSize, 64);
        stream->consume();
        blocks--;

    } while (true);
    ASSERT_EQ(span.dataSize, 64);

    ASSERT_EQ(blocks, 0);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
