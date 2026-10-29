#include <gtest/gtest.h>

#include <algorithm>
#include <set>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "book.hpp"
#include "book_database.hpp"
#include "comparators.hpp"
#include "concepts.hpp"
#include "filters.hpp"
#include "heterogeneous_lookup.hpp"
#include "statsistics.hpp"

using namespace bookdb;

namespace {

Book mk(std::string_view author, std::string title, int year, Genre genre, double rating, int read_count) {
    return Book{title, author, year, genre, rating, read_count};
}

BookDatabase<> MakeMainDb() {
    BookDatabase<> db;
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);
    db.EmplaceBack("The Great Gatsby", "F. Scott Fitzgerald", 1925, Genre::Fiction, 4.5, 120);
    db.EmplaceBack("To Kill a Mockingbird", "Harper Lee", 1960, Genre::Fiction, 4.8, 156);
    db.EmplaceBack("Pride and Prejudice", "Jane Austen", 1813, Genre::Fiction, 4.7, 178);
    db.EmplaceBack("The Catcher in the Rye", "J.D. Salinger", 1951, Genre::Fiction, 4.3, 112);
    db.EmplaceBack("Brave New World", "Aldous Huxley", 1932, Genre::SciFi, 4.5, 98);
    db.EmplaceBack("Jane Eyre", "Charlotte Brontë", 1847, Genre::Fiction, 4.6, 110);
    db.EmplaceBack("The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203);
    db.EmplaceBack("Lord of the Flies", "William Golding", 1954, Genre::Fiction, 4.2, 89);
    return db;
}

}  // namespace

TEST(BookTest, GenreFromString) {
    EXPECT_EQ(GenreFromString("Fiction"), Genre::Fiction);
    EXPECT_EQ(GenreFromString("NonFiction"), Genre::NonFiction);
    EXPECT_EQ(GenreFromString("SciFi"), Genre::SciFi);
    EXPECT_EQ(GenreFromString("Biography"), Genre::Biography);
    EXPECT_EQ(GenreFromString("Mystery"), Genre::Mystery);
    EXPECT_EQ(GenreFromString("Unknown"), Genre::Unknown);
    EXPECT_EQ(GenreFromString("garbage"), Genre::Unknown);
    EXPECT_EQ(GenreFromString(""), Genre::Unknown);
}

TEST(BookTest, ConstructorsEnumAndString) {
    Book byEnum{"1984", "Orwell", 1949, Genre::SciFi, 4.0, 190};

    EXPECT_EQ(byEnum.author, "Orwell");
    EXPECT_EQ(byEnum.title, "1984");
    EXPECT_EQ(byEnum.year, 1949);
    EXPECT_EQ(byEnum.genre, Genre::SciFi);
    EXPECT_DOUBLE_EQ(byEnum.rating, 4.0);
    EXPECT_EQ(byEnum.read_count, 190);

    Book byString{"1984", "Orwell", 1949, std::string_view{"SciFi"}, 4.0, 190};

    EXPECT_EQ(byString.genre, Genre::SciFi);
    EXPECT_EQ(byString.title, byEnum.title);
}

TEST(BookTest, Formatter) {
    Book b{"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190};
    const std::string s = std::format("{}", b);

    EXPECT_NE(s.find("1984"), std::string::npos);
    EXPECT_NE(s.find("George Orwell"), std::string::npos);
    EXPECT_NE(s.find("SciFi"), std::string::npos);
}

TEST(ComparatorsTest, AllComparators) {
    Book b1{"1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190};
    Book b2{"Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143};
    Book b3{"The Hobbit", "J.R.R. Tolkien", 1937, Genre::Fiction, 4.9, 203};

    EXPECT_TRUE(comp::LessByAuthor{}(b1, b2));
    EXPECT_TRUE(comp::LessByAuthor{}(b1, b3));
    EXPECT_FALSE(comp::LessByAuthor{}(b3, b1));

    EXPECT_TRUE(comp::LessByTitle{}(b1, b2));

    EXPECT_TRUE(comp::LessByYear{}(b3, b1));

    EXPECT_TRUE(comp::LessByGenre{}(b2, b1));
    EXPECT_FALSE(comp::LessByGenre{}(b1, b2));

    EXPECT_TRUE(comp::LessByRating{}(b1, b2));

    EXPECT_TRUE(comp::LessByReadCount{}(b2, b1));

    EXPECT_TRUE(comp::LessByPopularity{}(b2, b1));
}

TEST(HeterogeneousLookupTest, TransparentSet) {
    std::set<std::string, TransparentStringLess> s{"alpha", "beta", "gamma"};

    EXPECT_NE(s.find(std::string_view{"beta"}), s.end());
    EXPECT_NE(s.find("alpha"), s.end());
    EXPECT_EQ(s.find(std::string_view{"delta"}), s.end());
}

TEST(HeterogeneousLookupTest, TransparentUnorderedSet) {
    std::unordered_set<std::string, TransparentStringHash, TransparentStringEqual> s{"alpha", "beta", "gamma"};

    EXPECT_NE(s.find(std::string_view{"beta"}), s.end());
    EXPECT_NE(s.find("alpha"), s.end());
    EXPECT_EQ(s.find(std::string_view{"delta"}), s.end());

    TransparentStringHash h;
    EXPECT_EQ(h(std::string{"hello"}), h(std::string_view{"hello"}));
    EXPECT_EQ(h("hello"), h(std::string_view{"hello"}));
}

TEST(BookDatabaseTest, PushBackAndAccess) {
    auto db = MakeMainDb();

    EXPECT_EQ(db.size(), 10);
    EXPECT_EQ(db.front().title, "1984");
    EXPECT_EQ(db.back().title, "Lord of the Flies");
    EXPECT_EQ(db[0].author, "George Orwell");
    EXPECT_EQ(db.at(1).title, "Animal Farm");

    std::size_t count = 0;
    for (const auto &book : db) {
        EXPECT_FALSE(book.title.empty());
        ++count;
    }
    EXPECT_EQ(count, 10);

    EXPECT_EQ(db.GetAuthors().size(), 9);
    EXPECT_NE(db.GetAuthors().find("George Orwell"), db.GetAuthors().end());
    EXPECT_NE(db.GetAuthors().find("Harper Lee"), db.GetAuthors().end());
}

TEST(BookDatabaseTest, EmplaceBackAndInitializerList) {
    BookDatabase<> db;
    db.EmplaceBack("1984", "George Orwell", 1949, Genre::SciFi, 4.0, 190);
    db.EmplaceBack("Animal Farm", "George Orwell", 1945, Genre::Fiction, 4.4, 143);

    EXPECT_EQ(db.size(), 2);
    EXPECT_EQ(db[0].title, "1984");
    EXPECT_EQ(db[1].genre, Genre::Fiction);

    BookDatabase<> db2{
        mk("George Orwell", "1984", 1949, Genre::SciFi, 4.0, 190),
        mk("George Orwell", "Animal Farm", 1945, Genre::Fiction, 4.4, 143),
        mk("Harper Lee", "To Kill a Mockingbird", 1960, Genre::Fiction, 4.8, 156),
    };

    EXPECT_EQ(db2.size(), 3);
    EXPECT_EQ(db2.GetAuthors().size(), 2);
}

TEST(BookDatabaseTest, SortAndClearAndSpan) {
    auto db = MakeMainDb();

    std::sort(db.begin(), db.end(), comp::LessByAuthor{});
    EXPECT_EQ(db[0].author, "Aldous Huxley");
    for (std::size_t i = 1; i < db.size(); ++i) {
        EXPECT_TRUE(db[i - 1].author <= db[i].author);
    }

    std::sort(db.begin(), db.end(), comp::LessByRating{});

    EXPECT_DOUBLE_EQ(db[0].rating, 4.0);
    EXPECT_DOUBLE_EQ(db[9].rating, 4.9);

    auto span = db.GetBooks();

    EXPECT_EQ(span.size(), 10);
    EXPECT_EQ(span.data(), &db[0]);

    const auto &cdb = db;
    std::span<const Book> cspan = cdb.GetBooks();

    EXPECT_EQ(cspan.size(), 10);

    db.Clear();

    EXPECT_TRUE(db.empty());
    EXPECT_EQ(db.size(), 0);
    EXPECT_TRUE(db.GetAuthors().empty());
}

TEST(BookDatabaseTest, Formatter) {
    auto db = MakeMainDb();
    const std::string s = std::format("{}", db);

    EXPECT_NE(s.find("BookDatabase"), std::string::npos);
    EXPECT_NE(s.find("1984"), std::string::npos);
    EXPECT_NE(s.find("George Orwell"), std::string::npos);
    EXPECT_NE(s.find("The Hobbit"), std::string::npos);
}

TEST(StatisticsTest, AuthorHistogram) {
    auto db = MakeMainDb();
    auto h = buildAuthorHistogramFlat(db);

    EXPECT_EQ(h.size(), 9);
    EXPECT_EQ(h.at("George Orwell"), 2);
    EXPECT_EQ(h.at("Harper Lee"), 1);
    EXPECT_EQ(h.at("J.R.R. Tolkien"), 1);
    EXPECT_EQ(h.at("Jane Austen"), 1);
}

TEST(StatisticsTest, GenreRatings) {
    auto db = MakeMainDb();
    auto r = calculateGenreRatings(db.begin(), db.end());

    EXPECT_DOUBLE_EQ(r.at(Genre::SciFi), 4.25);
    EXPECT_DOUBLE_EQ(r.at(Genre::Fiction), 4.55);
}

TEST(StatisticsTest, AverageRating) {
    auto db = MakeMainDb();
    EXPECT_DOUBLE_EQ(calculateAverageRating(db), 4.49);

    BookDatabase<> empty;
    EXPECT_DOUBLE_EQ(calculateAverageRating(empty), 0.0);
}

TEST(StatisticsTest, SampleRandomBooks) {
    auto db = MakeMainDb();
    auto s = sampleRandomBooks(db, 3);
    EXPECT_EQ(s.size(), 3);
    for (const Book &b : s) {
        auto it = std::find_if(db.begin(), db.end(), [&](const Book &x) { return &x == &b; });
        EXPECT_NE(it, db.end());
    }

    auto all = sampleRandomBooks(db, 100);
    EXPECT_EQ(all.size(), 10);

    auto none = sampleRandomBooks(db, 0);
    EXPECT_TRUE(none.empty());
}

TEST(StatisticsTest, TopNByRating) {
    auto db = MakeMainDb();
    auto top = getTopNBy(db, 3, comp::LessByRating{});

    ASSERT_EQ(top.size(), 3);
    EXPECT_DOUBLE_EQ(top[0].get().rating, 4.9);
    EXPECT_DOUBLE_EQ(top[1].get().rating, 4.8);
    EXPECT_DOUBLE_EQ(top[2].get().rating, 4.7);

    EXPECT_EQ(top[0].get().title, "The Hobbit");
    EXPECT_EQ(top[1].get().title, "To Kill a Mockingbird");
    EXPECT_EQ(top[2].get().title, "Pride and Prejudice");

    EXPECT_EQ(db.size(), 10);
    EXPECT_EQ(db[0].title, "The Hobbit");

    auto allTop = getTopNBy(db, 100, comp::LessByRating{});

    EXPECT_EQ(allTop.size(), 10);
    EXPECT_DOUBLE_EQ(allTop[0].get().rating, 4.9);
}

TEST(FiltersTest, SinglePredicates) {
    Book b = mk("George Orwell", "1984", 1949, Genre::SciFi, 4.0, 190);

    EXPECT_TRUE(YearBetween(1900, 1999)(b));
    EXPECT_FALSE(YearBetween(1960, 1999)(b));
    EXPECT_TRUE(RatingAbove(4.0)(b));
    EXPECT_FALSE(RatingAbove(4.1)(b));
    EXPECT_TRUE(GenreIs(Genre::SciFi)(b));
    EXPECT_FALSE(GenreIs(Genre::Fiction)(b));
}

TEST(FiltersTest, AllOfAnyOf) {
    Book b = mk("Harper Lee", "To Kill a Mockingbird", 1960, Genre::Fiction, 4.8, 156);

    EXPECT_TRUE(all_of(YearBetween(1900, 1999), RatingAbove(4.5))(b));
    EXPECT_FALSE(all_of(YearBetween(1900, 1999), RatingAbove(4.9))(b));

    EXPECT_TRUE(any_of(GenreIs(Genre::SciFi), RatingAbove(4.5))(b));
    EXPECT_FALSE(any_of(GenreIs(Genre::SciFi), RatingAbove(4.9))(b));

    EXPECT_TRUE(all_of()(b));
    EXPECT_FALSE(any_of()(b));
}

TEST(FiltersTest, FilterBooks) {
    auto db = MakeMainDb();

    auto filtered = filterBooks(db.begin(), db.end(), all_of(YearBetween(1900, 1999), RatingAbove(4.5)));

    ASSERT_EQ(filtered.size(), 4);

    std::vector<std::string> titles;
    for (const auto &b : filtered) {
        titles.push_back(b.get().title);
    }
    std::sort(titles.begin(), titles.end());
    EXPECT_EQ(titles,
              (std::vector<std::string>{"Brave New World", "The Great Gatsby", "The Hobbit", "To Kill a Mockingbird"}));

    auto none = filterBooks(db.begin(), db.end(), RatingAbove(10.0));
    EXPECT_TRUE(none.empty());
}

TEST(StatisticsTest, EmptyDatabase) {
    BookDatabase<> db;
    auto h = buildAuthorHistogramFlat(db);
    EXPECT_TRUE(h.empty());

    auto r = calculateGenreRatings(db.begin(), db.end());
    EXPECT_TRUE(r.empty());

    auto s = sampleRandomBooks(db, 5);
    EXPECT_TRUE(s.empty());

    auto top = getTopNBy(db, 5, comp::LessByRating{});
    EXPECT_TRUE(top.empty());
}

TEST(FiltersTest, EmptyRange) {
    BookDatabase<> db;
    auto filtered = filterBooks(db.begin(), db.end(), YearBetween(1900, 2000));

    EXPECT_TRUE(filtered.empty());
}

TEST(BookDatabaseTest, TypeAliases) {
    using DB = BookDatabase<>;
    static_assert(std::same_as<DB::value_type, Book>);
    static_assert(std::same_as<DB::container_type, std::vector<Book>>);
    static_assert(std::same_as<DB::iterator, std::vector<Book>::iterator>);
    static_assert(std::same_as<DB::const_iterator, std::vector<Book>::const_iterator>);
    static_assert(std::same_as<DB::reference, Book &>);
    static_assert(std::same_as<DB::const_reference, const Book &>);
    static_assert(std::same_as<DB::size_type, std::vector<Book>::size_type>);

    SUCCEED();
}

TEST(ConceptsTest, AllConcepts) {
    // BookContainerLike
    static_assert(BookContainerLike<std::vector<Book>>);
    static_assert(BookContainerLike<std::vector<Book> &>);
    static_assert(!BookContainerLike<std::vector<int>>);
    static_assert(!BookContainerLike<int>);

    // BookIterator
    static_assert(BookIterator<std::vector<Book>::iterator>);
    static_assert(BookIterator<std::vector<Book>::const_iterator>);
    static_assert(!BookIterator<std::vector<int>::iterator>);

    // BookPredicate / BookComparator
    auto pred = [](const Book &) { return true; };
    auto cmp = [](const Book &, const Book &) { return false; };
    static_assert(BookPredicate<decltype(pred)>);
    static_assert(BookComparator<decltype(cmp)>);
    static_assert(!BookPredicate<decltype(cmp)>);

    SUCCEED();
}
