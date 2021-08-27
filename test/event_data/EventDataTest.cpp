// MIT License
//
// Copyright (c) 2021 Marko Malenic
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <gtest/gtest.h>
#include <EventData/EventData.h>
#include "UnsupportedOperationException.h"

using namespace std;

TEST(EventDataTest, CreationAndIteration) { // NOLINT(cert-err58-cpp)
    EventData eventData{};
    eventData.addEntry(mouseclick, text, "");
    eventData.finish();

    auto n = 0;
    for (auto i{eventData.begin()}; i != eventData.end(); i++, n++) {
        ASSERT_EQ(mouseclick, i->getEventDeviceFormat());
        ASSERT_EQ(text, i->getDataType());
        ASSERT_EQ("", i->getEntry());
    }
    ASSERT_EQ(n, 1);
}

TEST(EventDataTest, ThrowsAfterFinish) { // NOLINT(cert-err58-cpp)
    EventData eventData{};
    eventData.addEntry(mouseclick, text, "");
    eventData.finish();

    ASSERT_THROW(eventData.addEntry(mouseclick, text, ""), UnsupportedOperationException);
}
