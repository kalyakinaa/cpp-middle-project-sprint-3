#pragma once

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"

namespace bookdb {

[[nodiscard]] constexpr auto YearBetween(int start, int end) {
    return [start, end](const Book &book) { return book.year >= start && book.year <= end; };
}

[[nodiscard]] constexpr auto RatingAbove(double min_rating) {
    return [min_rating](const Book &book) { return book.rating >= min_rating; };
}

[[nodiscard]] constexpr auto GenreIs(Genre genre) {
    return [genre](const Book &book) { return book.genre == genre; };
}

template <BookPredicate... Preds>
[[nodiscard]] constexpr auto all_of(Preds... preds) {
    return [... ps = std::move(preds)](const Book &book) { return (ps(book) && ...); };
}

template <BookPredicate... Preds>
[[nodiscard]] constexpr auto any_of(Preds... preds) {
    return [... ps = std::move(preds)](const Book &book) { return (ps(book) || ...); };
}

template <BookIterator It, BookSentinel<It> Sent, BookPredicate Pred>
[[nodiscard]] auto filterBooks(It first, Sent last, Pred pred) {
    std::vector<std::reference_wrapper<const Book>> result;
    std::copy_if(first, last, std::back_inserter(result), [&pred](const Book &book) { return pred(book); });
    return result;
}

}  // namespace bookdb
