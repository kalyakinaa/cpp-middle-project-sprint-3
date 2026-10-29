#pragma once

#include <concepts>
#include <initializer_list>
#include <span>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "book.hpp"
#include "concepts.hpp"
#include "heterogeneous_lookup.hpp"

namespace bookdb {

template <BookContainerLike BookContainer = std::vector<Book>>
class BookDatabase {
public:
    using container_type = BookContainer;
    using value_type = typename BookContainer::value_type;
    using size_type = typename BookContainer::size_type;
    using difference_type = typename BookContainer::difference_type;
    using reference = typename BookContainer::reference;
    using const_reference = typename BookContainer::const_reference;
    using pointer = typename BookContainer::pointer;
    using const_pointer = typename BookContainer::const_pointer;
    using iterator = typename BookContainer::iterator;
    using const_iterator = typename BookContainer::const_iterator;
    using reverse_iterator = typename BookContainer::reverse_iterator;
    using const_reverse_iterator = typename BookContainer::const_reverse_iterator;

    using AuthorContainer = std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual>;

    BookDatabase() = default;

    BookDatabase(std::initializer_list<Book> init) {
        for (const Book &book : init) {
            PushBack(book);
        }
    }

    void Clear() {
        books_.clear();
        authors_.clear();
    }

    iterator begin() noexcept { return books_.begin(); }
    iterator end() noexcept { return books_.end(); }

    const_iterator begin() const noexcept { return books_.begin(); }
    const_iterator end() const noexcept { return books_.end(); }

    const_iterator cbegin() const noexcept { return books_.cbegin(); }
    const_iterator cend() const noexcept { return books_.cend(); }

    reverse_iterator rbegin() noexcept { return books_.rbegin(); }
    reverse_iterator rend() noexcept { return books_.rend(); }

    const_reverse_iterator rbegin() const noexcept { return books_.rbegin(); }
    const_reverse_iterator rend() const noexcept { return books_.rend(); }

    const_reverse_iterator crbegin() const noexcept { return books_.crbegin(); }
    const_reverse_iterator crend() const noexcept { return books_.crend(); }

    bool empty() const noexcept { return books_.empty(); }
    size_type size() const noexcept { return books_.size(); }
    size_type capacity() const noexcept { return books_.capacity(); }

    reference front() { return books_.front(); }
    const_reference front() const { return books_.front(); }

    reference back() { return books_.back(); }
    const_reference back() const { return books_.back(); }

    reference operator[](size_type i) { return books_[i]; }
    const_reference operator[](size_type i) const { return books_[i]; }

    reference at(size_type i) { return books_.at(i); }
    const_reference at(size_type i) const { return books_.at(i); }

    void PushBack(const Book &book) {
        auto [it, inserted] = authors_.insert(std::string(book.author));
        Book copy = book;
        copy.author = *it;
        books_.push_back(std::move(copy));
    }

    void PushBack(Book &&book) {
        auto [it, inserted] = authors_.insert(std::string(book.author));
        book.author = *it;
        books_.push_back(std::move(book));
    }

    template <typename... Args>
        requires std::constructible_from<Book, Args...>
    void EmplaceBack(Args &&...args) {
        auto &book = books_.emplace_back(std::forward<Args>(args)...);
        auto [it, inserted] = authors_.insert(std::string(book.author));
        book.author = *it;
    }

    [[nodiscard]] std::span<const Book> GetBooks() const { return books_; }
    [[nodiscard]] std::span<Book> GetBooks() { return books_; }

    [[nodiscard]] const AuthorContainer &GetAuthors() const { return authors_; }
    [[nodiscard]] const AuthorContainer &GetAuthors() { return authors_; }

private:
    BookContainer books_;
    AuthorContainer authors_;
};

}  // namespace bookdb

namespace std {
template <>
struct formatter<bookdb::BookDatabase<std::vector<bookdb::Book>>> {
    template <typename FormatContext>
    auto format(const bookdb::BookDatabase<std::vector<bookdb::Book>> &db, FormatContext &fc) const {
        format_to(fc.out(), "BookDatabase (size = {}): ", db.size());

        format_to(fc.out(), "Books:\n");
        for (const auto &book : db.GetBooks()) {
            format_to(fc.out(), "- {}\n", book);
        }

        format_to(fc.out(), "Authors:\n");
        for (const auto &author : db.GetAuthors()) {
            format_to(fc.out(), "- {}\n", author);
        }
        return fc.out();
    }

    constexpr auto parse(format_parse_context &ctx) { return ctx.begin(); }
};
}  // namespace std
