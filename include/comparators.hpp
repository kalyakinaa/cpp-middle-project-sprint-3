#pragma once

#include "book.hpp"

#include <string_view>

namespace bookdb::comp {

struct LessByAuthor {
    bool operator()(const Book &lhs, const Book &rhs) const {
        if (lhs.author != rhs.author)
            return lhs.author < rhs.author;
        return lhs.title < rhs.title;
    }
};

struct LessByTitle {
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.title < rhs.title; }
};

struct LessByYear {
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.year < rhs.year; }
};

struct LessByGenre {
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.genre < rhs.genre; }
};

struct LessByRating {
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.rating < rhs.rating; }
};

struct LessByReadCount {
    bool operator()(const Book &lhs, const Book &rhs) const { return lhs.read_count < rhs.read_count; }
};

struct LessByPopularity {
    bool operator()(const Book &lhs, const Book &rhs) const {
        return lhs.rating * lhs.read_count < rhs.rating * rhs.read_count;
    }
};

}  // namespace bookdb::comp
