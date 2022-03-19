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

#include "evgetcore/Event/TableData.h"

EvgetCore::Event::TableData::TableData(EvgetCore::Event::TableData::TableDataBuilder& builder) : genericData{std::move(builder._genericData)}, systemData{std::move(builder._systemData)} {
}

EvgetCore::Event::TableData::TableDataBuilder& EvgetCore::Event::TableData::TableDataBuilder::genericData(std::unique_ptr<AbstractData> genericData) {
    _genericData = std::move(genericData);
    return *this;
}

EvgetCore::Event::TableData::TableDataBuilder& EvgetCore::Event::TableData::TableDataBuilder::systemData(std::unique_ptr<AbstractData> systemData) {
    _systemData = std::move(systemData);
    return *this;
}

std::unique_ptr<EvgetCore::Event::TableData> EvgetCore::Event::TableData::TableDataBuilder::build() {
    return std::make_unique<EvgetCore::Event::TableData>(*this);
}

const EvgetCore::Event::AbstractData* EvgetCore::Event::TableData::getGenericData() const {
    return genericData.get();
}

const EvgetCore::Event::AbstractData* EvgetCore::Event::TableData::getSystemData() const {
    return systemData.get();
}
