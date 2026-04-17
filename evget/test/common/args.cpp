#include "common/args.h"

#include <string>
#include <utility>
#include <vector>

test::Args::Args(std::vector<std::string> args) : storage_{std::move(args)} {}

int test::Args::Argc() const {
    return static_cast<int>(storage_.size());
}

char** test::Args::Argv() {
    pointers_.clear();
    pointers_.reserve(storage_.size());
    for (auto& arg : storage_) {
        pointers_.push_back(arg.data());
    }
    return pointers_.data();
}
