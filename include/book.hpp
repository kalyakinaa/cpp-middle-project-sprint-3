#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <string_view>

namespace bookdb {

enum class Genre { Fiction, NonFiction, SciFi, Biography, Mystery, Unknown };

constexpr bool operator<(Genre a, Genre b) { return static_cast<int>(a) < static_cast<int>(b); }

[[nodiscard]] constexpr Genre GenreFromString(std::string_view s) {
    if (s == "Fiction")
        return Genre::Fiction;
    if (s == "NonFiction")
        return Genre::NonFiction;
    if (s == "SciFi")
        return Genre::SciFi;
    if (s == "Biography")
        return Genre::Biography;
    if (s == "Mystery")
        return Genre::Mystery;
    return Genre::Unknown;
}

struct Book {
    std::string_view author;
    std::string title;

    int year;
    Genre genre;
    double rating;
    int read_count;

    constexpr Book(std::string_view title_, std::string_view author_, int year_, Genre genre_, double rating_,
                   int read_count_)
        : author(author_), title(title_), year(year_), genre(genre_), rating(rating_), read_count(read_count_) {}

    constexpr Book(std::string_view title_, std::string_view author_, int year_, std::string_view genre_,
                   double rating_, int read_count_)
        : Book(title_, author_, year_, GenreFromString(genre_), rating_, read_count_) {}
};
}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::Genre, char> {
    template <typename FormatContext>
    auto format(const bookdb::Genre g, FormatContext &fc) const {
        std::string genre_str;

        using bookdb::Genre;
        switch (g) {
        case Genre::Fiction:
            genre_str = "Fiction";
            break;
        case Genre::Mystery:
            genre_str = "Mystery";
            break;
        case Genre::NonFiction:
            genre_str = "NonFiction";
            break;
        case Genre::SciFi:
            genre_str = "SciFi";
            break;
        case Genre::Biography:
            genre_str = "Biography";
            break;
        case Genre::Unknown:
            genre_str = "Unknown";
            break;
        default:
            throw logic_error{"Unsupported bookdb::Genre"};
        }
        return format_to(fc.out(), "{}", genre_str);
    }

    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
};

template <>
struct formatter<bookdb::Book, char> {
    template <typename FormatContext>
    auto format(const bookdb::Book &b, FormatContext &fc) const {
        return format_to(fc.out(), "\"{}\" by {} ({}, {}), rating: {}, read: {}", b.title, b.author, b.year, b.genre,
                         b.rating, b.read_count);
    }

    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
};
}  // namespace std
