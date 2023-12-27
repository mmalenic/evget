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

#include <spdlog/spdlog.h>

#include "database\sqlite\SQLiteConnection.h"

Database::Result<void> Database::SQLiteConnection::connect(std::string database, ConnectOptions options) {
    try {
        switch (options) {
            case ConnectOptions::READ_ONLY:
                this->database = {database, ::SQLite::OPEN_READONLY};
                break;
            case ConnectOptions::READ_WRITE:
                this->database = {database, ::SQLite::OPEN_READWRITE};
                break;
            case ConnectOptions::READ_WRITE_CREATE:
                this->database = {database, ::SQLite::OPEN_READWRITE | ::SQLite::OPEN_CREATE};
                break;
        }

        spdlog::info("connected to SQLite database: {}", database);
        return {};
    } catch (std::exception& e) {
        auto what = e.what();
        spdlog::error("error connecting to SQLite database: {}", what);
        return Err{{.errorType = ErrorType::ConnectError, .message = what}};
    }
}
