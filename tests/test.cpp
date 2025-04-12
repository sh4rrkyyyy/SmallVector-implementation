#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <limits>
#include <random>
#include <algorithm>
#include <vector>
#include <string>
#include <small_vector.h>

TEST_CASE("base_class") {
    static_assert(sizeof(VectorBase<int>) == 24);
    static_assert(sizeof(VectorBase<char>) == 24);
    SmallVector<int, 3> raw;
    VectorBase<int>& data = raw;
    REQUIRE(data.is_small());
    REQUIRE(data.size() == 0);
    REQUIRE(data.capacity() == 3);
    data.push_back(3);
    data.push_back(2);
    data.push_back(1);
    data[1] = -1;
    REQUIRE(data[1] == -1);
    {
        const auto& tmp = data;
        REQUIRE(tmp[2] == 1);
        REQUIRE(tmp.is_small());
    }
    data.pop_back();
    data.push_back(7);
    REQUIRE(data.is_small());
    REQUIRE(data.size() == 3);
    REQUIRE(data.capacity() == 3);
    data.push_back(8);
    REQUIRE(!data.is_small());
    REQUIRE(data.size() == 4);
    REQUIRE(data.capacity() == 6);
    REQUIRE(data[2] == 7);
}

TEST_CASE("ctr") {
    static_assert(sizeof(SmallVector<int, 4>) == 40);
    using Vector = SmallVector<std::string, 2>;
    std::vector<std::string> tmp{"d", "e", "f"};
    Vector a(2, "a");
    Vector b;
    Vector c{"c", "cc"};
    Vector d(tmp.begin(), tmp.end());

    REQUIRE(a.is_small());
    REQUIRE(a[1] == "a");
    REQUIRE(b.is_small());
    REQUIRE(c.is_small());
    REQUIRE(!d.is_small());
    REQUIRE(d.capacity() == 3);

    a = a;
    b = d;
    REQUIRE(a[0] == "a");
    REQUIRE(!b.is_small());
    REQUIRE(b.size() == 3);
    a = std::move(d);
    REQUIRE(a[2] == "f");
    REQUIRE(!a.is_small());

    Vector e(c);
    REQUIRE(e.is_small());
    REQUIRE(e[1] == "cc");
    Vector f(std::move(b));
    REQUIRE(f.size() == 3);
    REQUIRE(f[2] == "f");

    f = std::move(c);
    REQUIRE(f.is_small());
    REQUIRE(f[0] == "c");
}

TEST_CASE("shrink") {
    const int iter = 1 << 17;
    SmallVector<int, 8> data;
    for (int i = 0; i < iter; ++i) {
        REQUIRE(data.is_small() == (i <= 8));
        data.push_back(i);
        REQUIRE(data.size() == size_t(i + 1));
        if (i >= 8 && (i & (i - 1)) == 0) {
            REQUIRE(data.capacity() == (data.size() - 1) * 2);
        }
    }
    REQUIRE(!data.reset_small());
    for (int i = 0; i < iter - 8; ++i) {
        data.pop_back();
    }
    REQUIRE(data[data.size() - 1] == 7);
    REQUIRE(data.reset_small());
    REQUIRE(data.is_small());
    REQUIRE(data.reset_small());
}
