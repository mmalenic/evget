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

#include "evgetcore/Storage/SQLite.h"
#include "schema/initialize.h"
#include "queries/insert_key.h"
#include "queries/insert_key_modifier.h"
#include "queries/insert_mouse_move.h"
#include "queries/insert_mouse_move_modifier.h"
#include "queries/insert_mouse_click.h"
#include "queries/insert_mouse_click_modifier.h"
#include "queries/insert_mouse_scroll.h"
#include "queries/insert_mouse_scroll_modifier.h"


#include <SQLiteCpp/SQLiteCpp.h>

#include <utility>

EvgetCore::Storage::SQLite::SQLite(const std::string& database)
    : database{database},
initialize{this->database, Database::detail::initialize},
insertKey{this->database, Database::detail::insert_key},
insertKeyModifier{this->database, Database::detail::insert_key_modifier},
insertMouseClick{this->database, Database::detail::insert_mouse_click},
      insertMouseClickModifier{this->database, Database::detail::insert_mouse_click_modifier},
      insertMouseMove{this->database, Database::detail::insert_mouse_move},
      insertMouseMoveModifier{this->database, Database::detail::insert_mouse_move_modifier},
      insertMouseScroll{this->database, Database::detail::insert_mouse_scroll},
      insertMouseScrollModifier{this->database, Database::detail::insert_mouse_scroll_modifier} {
}

void EvgetCore::Storage::SQLite::store(Event::Data event) {
}

EvgetCore::Storage::Result<void> EvgetCore::Storage::SQLite::init() {
    return {};
}
