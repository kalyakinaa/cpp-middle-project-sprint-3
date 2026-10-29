#pragma once

#include <algorithm>
#include <cstddef>
#include <flat_map>
#include <functional>
#include <iterator>
#include <map>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "book_database.hpp"
#include "comparators.hpp"

namespace bookdb {

template <BookContainerLike T, typename Comparator = TransparentStringLess>
[[nodiscard]] auto buildAuthorHistogramFlat(const BookDatabase<T> &cont, Comparator comp = {}) {
    std::flat_map<std::string, std::size_t, Comparator> histogram(comp);

    for (const Book &book : cont.GetBooks()) {
        auto it = histogram.lower_bound(book.author);
        if (it != histogram.end() && it->first == book.author) {
            ++it->second;
        } else {
            histogram.emplace_hint(it, std::string(book.author), 1);
        }
    }
    return histogram;
}

template <BookIterator It, BookSentinel<It> Sent>
[[nodiscard]] auto calculateGenreRatings(It first, Sent last) {
    std::flat_map<Genre, std::pair<double, std::size_t>> acc;

    for (; first != last; ++first) {
        const Book &book = *first;
        auto [it, inserted] = acc.try_emplace(book.genre, 0.0, 0u);
        it->second.first += book.rating;
        ++it->second.second;
    }

    std::flat_map<Genre, double> result;
    for (const auto &[genre, sc] : acc) {
        const auto &[sum, count] = sc;
        result.emplace(genre, count > 0 ? sum / static_cast<double>(count) : 0.0);
    }
    return result;
}

template <BookContainerLike T>
[[nodiscard]] double calculateAverageRating(const BookDatabase<T> &books) {
    auto span = books.GetBooks();
    if (span.empty()) {
        return 0.0;
    }

    const double sum =
        std::transform_reduce(span.begin(), span.end(), 0.0, std::plus<>(), [](const Book &b) { return b.rating; });
    return sum / static_cast<double>(span.size());
}

template <BookContainerLike T>
[[nodiscard]] auto sampleRandomBooks(const BookDatabase<T> &cont, std::size_t num) {
    std::vector<std::reference_wrapper<const Book>> result;
    auto books = cont.GetBooks();

    const std::size_t take = std::min(num, books.size());
    result.reserve(take);

    static thread_local std::mt19937 rng{std::random_device{}()};

    std::sample(books.begin(), books.end(), std::back_inserter(result), take, rng);

    return result;
}

template <BookContainerLike T, typename Comparator = comp::LessByRating>
[[nodiscard]] auto getTopNBy(BookDatabase<T> &cont, std::size_t n, Comparator comp = {}) {
    std::vector<std::reference_wrapper<const Book>> result;

    auto books = cont.GetBooks();
    const std::size_t take = std::min(n, books.size());
    if (take == 0) {
        return result;
    }

    std::partial_sort(books.begin(), books.begin() + static_cast<std::ptrdiff_t>(take), books.end(),
                      [&comp](const Book &a, const Book &b) { return comp(b, a); });

    result.reserve(take);
    for (std::size_t i = 0; i < take; ++i) {
        result.emplace_back(std::cref(books[i]));
    }
    return result;
}

}  // namespace bookdb

namespace std {

template <typename Key, typename Value, typename Compare, typename KeyCont, typename MappedCont>
struct formatter<std::flat_map<Key, Value, Compare, KeyCont, MappedCont>, char> {
    template <typename FormatContext>
    auto format(const std::flat_map<Key, Value, Compare, KeyCont, MappedCont> &m, FormatContext &fc) const {
        auto out = fc.out();
        out = std::format_to(out, "{{");
        bool first = true;
        for (const auto &[k, v] : m) {
            if (!first)
                out = std::format_to(out, ", ");
            first = false;
            out = std::format_to(out, "{}: {}", k, v);
        }
        return std::format_to(out, "}}");
    }
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
};

template <typename Key, typename Value, typename Compare, typename Alloc>
struct formatter<std::map<Key, Value, Compare, Alloc>, char> {
    template <typename FormatContext>
    auto format(const std::map<Key, Value, Compare, Alloc> &m, FormatContext &fc) const {
        auto out = fc.out();
        out = std::format_to(out, "{{");
        bool first = true;
        for (const auto &[k, v] : m) {
            if (!first)
                out = std::format_to(out, ", ");
            first = false;
            out = std::format_to(out, "{}: {}", k, v);
        }
        return std::format_to(out, "}}");
    }
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
};

}  // namespace std
