#pragma once

#include <concepts>
#include <iterator>
#include <ranges>
#include <utility>

#include "book.hpp"

namespace bookdb {

template <typename T>
concept BookContainerLike =
    std::ranges::sized_range<T> && std::ranges::range<T> &&
    std::same_as<std::remove_cvref_t<std::ranges::range_value_t<T>>, Book> && requires(T c, const T &cc, Book b) {
        { cc.empty() } -> std::convertible_to<bool>;
        { cc.size() } -> std::convertible_to<std::size_t>;
        { c.push_back(std::move(b)) } -> std::same_as<void>;
    };

template <typename T>
concept BookIterator = std::input_iterator<T> && std::same_as<std::remove_cvref_t<std::iter_value_t<T>>, Book>;

template <typename S, typename I>
concept BookSentinel = BookIterator<I> && std::sentinel_for<S, I>;

template <typename P>
concept BookPredicate = std::predicate<P, const Book &>;

template <typename C>
concept BookComparator = std::predicate<C, const Book &, const Book &>;

}  // namespace bookdb
