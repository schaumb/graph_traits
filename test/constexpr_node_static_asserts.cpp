#include <utility>

namespace std {
    template<size_t Int, template<class, class> class Pair, class T1, class T2>
    constexpr std::conditional_t<Int == 0, T1&, T2&>
    get(Pair<T1, T2>& p) noexcept
    {
        if constexpr (Int == 0)
            return p.first;
        else
            return p.second;
    }
    template<size_t Int, template<class, class> class Pair, class T1, class T2>
    constexpr std::conditional_t<Int == 0, const T1&, const T2&>
    get(const Pair<T1, T2>& p) noexcept
    {
        if constexpr (Int == 0)
            return p.first;
        else
            return p.second;
    }
}

#include <bxlx/graph/traits/node.hpp>
#include <bitset>
#include <cassert>
#include <deque>
#include <optional>
#include <map>

namespace node = bxlx::traits::node;

template<class, class T, class ... Ts>
constexpr static bool has_node_fun = false;
template<class T, class ...Ts>
constexpr static bool has_node_fun<std::void_t<
    decltype(node::has_node(std::declval<const T&>(), std::declval<Ts>()...))
>, T, Ts...> = true;

template<class T, class ...Ts>
constexpr static bool has_node_fun_v = has_node_fun<void, T, Ts...>;

static_assert(!has_node_fun_v<int>);
static_assert(!has_node_fun_v<int, int>);
static_assert(!has_node_fun_v<std::vector<std::vector<std::size_t>>, std::size_t, std::equal_to<>>);
static_assert(!has_node_fun_v<std::initializer_list<std::pair<int, int>>, int, std::hash<int>>);
static_assert(!has_node_fun_v<std::initializer_list<std::pair<int, int>>, int, std::equal_to<std::string_view>>);


static_assert(!node::has_node(std::array<std::initializer_list<int>, 0>{}, 0));
static_assert(node::has_node(std::array<std::initializer_list<int>, 5>{}, 0));
static_assert(!node::has_node(std::array<std::initializer_list<int>, 10>{}, -1));

static_assert(!node::has_node(std::array<std::initializer_list<std::size_t>, 2>{}, 4));
static_assert(node::has_node(std::array<std::initializer_list<std::size_t>, 5>{}, 4));
static_assert(!node::has_node(std::array<std::initializer_list<std::size_t>, 2>{}, -1));

static_assert(!node::has_node(std::initializer_list<std::initializer_list<int>>{}, 1));
static_assert(node::has_node(std::initializer_list<std::initializer_list<int>>{{}, {}}, 1));
static_assert(!node::has_node(std::initializer_list<std::initializer_list<int>>{}, -1));

static_assert(!node::has_node(std::initializer_list<std::initializer_list<std::size_t>>{}, 1));
static_assert(node::has_node(std::initializer_list<std::initializer_list<std::size_t>>{{}, {}}, 1));
static_assert(!node::has_node(std::initializer_list<std::initializer_list<std::size_t>>{}, -1));

static_assert(!node::has_node(std::initializer_list<std::pair<int, int>>{}, 0));
static_assert(!node::has_node(std::initializer_list<std::pair<int, int>>{{1, 2}}, 0));
static_assert(node::has_node(std::initializer_list<std::pair<int, int>>{{1, 2}, {0, 4}}, 2));
static_assert(node::has_node(std::initializer_list<std::pair<int, int>>{{1, 9}}, 7, [](int lhs, int rhs) { return lhs % 2 == rhs % 2; }));

static_assert(!node::has_node(std::array<std::bitset<5>, 5>{}, 6));
static_assert(node::has_node(std::array<std::bitset<5>, 5>{}, 0));
static_assert(!node::has_node(std::array<std::bitset<5>, 5>{}, -1));

static_assert(!node::has_node(std::array<std::initializer_list<unsigned>, 5>{}, 6));
static_assert(node::has_node(std::array<std::initializer_list<unsigned>, 5>{}, 0));
static_assert(!node::has_node(std::array<std::initializer_list<unsigned>, 5>{}, -1));

static_assert(!node::has_node(std::initializer_list<std::bitset<5>>{}, 6));
static_assert(node::has_node(std::initializer_list<std::bitset<5>>{}, 0));
static_assert(!node::has_node(std::initializer_list<std::bitset<5>>{}, -1));


template<class T, class U = T>
constexpr bool is_equal(T&& t, U&& u) {
    auto b_t = std::begin(t); auto e_t = std::end(t); auto b_u = std::begin(u); auto e_u = std::end(u);
    while (b_t != e_t && b_u != e_u)
        if (*b_t++ != *b_u++)
            return false;
    return b_t == e_t && b_u == e_u;
}

constexpr std::array<std::array<std::pair<int, float>, 2>, 3> list{{
    {{{1, 0.1}, {2, 0.2}}},
    {{{0, 1}}},
    {{{1, 6.1}, {2, 1.1}}}
}};

constexpr std::pair<std::array<int, 5>, std::initializer_list<std::pair<int, int>>> c_edge_list {
    {{2, 3, 5}},
    {}
};

static_assert(node::get_node(&list, 2) != nullptr);
static_assert(node::get_node(&list, 3) == nullptr);
static_assert(is_equal(*node::get_node(&list, 1), {{{0, 1}}}));
static_assert(is_equal(node::get_node(list, 0), {{{1, 0.1}, {2, 0.2}}}));

static_assert(is_equal(node::get_node(std::initializer_list<std::initializer_list<bool>>{{true, false}, {false, false}}, 0), {true, false}));

static_assert(node::get_node_property(std::pair<std::initializer_list<int>, std::initializer_list<std::pair<int, int>>>{{0, 4, 2}, {}}, 1) == 4);

static_assert(node::get_node_property(std::initializer_list<std::pair<std::initializer_list<int>, std::string_view>>{
    {}, {}, {{}, "hello"}
}, 2) == "hello");

static_assert(*node::get_node_property(&c_edge_list, 2) == 5);

template<class T, std::size_t Ix>
struct constexpr_vector {
    T arr[Ix]{};

    std::size_t s{};

    [[nodiscard]] constexpr T* begin() { return arr; }
    [[nodiscard]] constexpr const T* begin() const { return arr; }

    [[nodiscard]] constexpr T* end() { return arr + s; }
    [[nodiscard]] constexpr const T* end() const { return arr + s; }

    [[nodiscard]] constexpr std::size_t size() const { return s; }

    template<class ...Args>
    constexpr T& emplace_back(Args&& ...args) {
        if (s == Ix)
            throw std::bad_alloc();
        return arr[s++] = T(std::forward<Args>(args)...);
    }

    constexpr void resize(std::size_t new_s) {
        if (new_s > Ix)
            throw std::bad_alloc();
        s = new_s;
    }
};

template<class U, class V>
struct constexpr_pair {
    U first;
    V second;

    constexpr constexpr_pair() noexcept : first{}, second{} {}
    constexpr constexpr_pair(const U& f, const V& s) : first(f), second(s) {}
    template<class Tup1, class Tup2>
    constexpr constexpr_pair(std::piecewise_construct_t, Tup1&& f, Tup2&& s) : first(std::make_from_tuple<U>(f)), second(std::make_from_tuple<V>(s)) {}
};
template <class T1, class T2>
struct std::tuple_size<constexpr_pair<T1, T2>>
    : std::integral_constant<std::size_t, 2> { };

template< std::size_t I, class T1, class T2 >
struct std::tuple_element<I, constexpr_pair<T1, T2>> {
    using type = std::conditional_t<I == 0, T1, T2>;
};


template<class K, class V, std::size_t Ix>
struct constexpr_map {
    using T = constexpr_pair<K, V>;
    T arr[Ix]{};

    std::size_t s{};

    [[nodiscard]] constexpr T* begin() { return arr; }
    [[nodiscard]] constexpr const T* begin() const { return arr; }

    [[nodiscard]] constexpr T* end() { return arr + s; }
    [[nodiscard]] constexpr const T* end() const { return arr + s; }

    [[nodiscard]] constexpr std::size_t size() const { return s; }

    [[nodiscard]] constexpr const T* find(const K& k) const {
        for (auto& m : *this)
            if (m == k)
                return &m;

        return end();
    }

    [[nodiscard]] constexpr T* find(const K& k) {
        for (auto& m : *this)
            if (m == k)
                return &m;

        return end();
    }
};

template<class K, class V, std::size_t Ix, class Comp = std::equal_to<K>>
struct constexpr_modifiable_map {
    using T = constexpr_pair<K, V>;
    T arr[Ix]{};

    std::size_t s{};

    [[nodiscard]] constexpr T* begin() { return arr; }
    [[nodiscard]] constexpr const T* begin() const { return arr; }

    [[nodiscard]] constexpr T* end() { return arr + s; }
    [[nodiscard]] constexpr const T* end() const { return arr + s; }

    [[nodiscard]] constexpr std::size_t size() const { return s; }

    [[nodiscard]] constexpr const T* find(const K& k) const {
        for (auto& m : *this)
            if (Comp{}(m.first, k))
                return &m;

        return end();
    }

    [[nodiscard]] constexpr T* find(const K& k) {
        for (auto& m : *this)
            if (Comp{}(m.first, k))
                return &m;

        return end();
    }

    template<class ...Args>
    [[nodiscard]] constexpr constexpr_pair<T*, bool> try_emplace(const K& k, Args&& ...args) {
        if (auto p = find(k); p != end())
            return {p, false};

        if (s == Ix)
            throw std::bad_alloc();

        return {std::addressof(arr[s++] = T{k, V(std::forward<Args>(args)...)}), true};
    }
};


static_assert(bxlx::detail2::classify<constexpr_vector<int, 20>> == bxlx::detail2::type_classification::random_access_range);
static_assert(bxlx::detail2::compile_time_size_v<constexpr_vector<int, 20>> == 0);
static_assert(bxlx::detail2::classify<constexpr_map<int, int, 20>> == bxlx::detail2::type_classification::map_like_container);
static_assert(bxlx::detail2::classify<constexpr_modifiable_map<int, int, 20>> == bxlx::detail2::type_classification::map_like_container);


template<class, class T, class ...Args>
constexpr static bool has_add_node_impl = false;
template<class T, class ...Args>
constexpr static bool has_add_node_impl<std::void_t<
    decltype(node::add_node(std::declval<T&>(), std::declval<Args>()...))
>, T, Args...> = true;

template<class T, class ...Args>
constexpr static bool has_add_node_v = has_add_node_impl<void, T, Args...>;


static_assert(!has_add_node_v<int>);
static_assert(!has_add_node_v<int, std::size_t>);
static_assert(!has_add_node_v<std::array<std::vector<int>, 4>>);
static_assert(!has_add_node_v<std::vector<std::tuple<int, int>>>);
static_assert(!has_add_node_v<std::initializer_list<std::vector<int>>>);
static_assert(!has_add_node_v<std::vector<std::initializer_list<bool>>>);

static_assert(has_add_node_v<std::vector<std::initializer_list<int>>>);
static_assert(has_add_node_v<std::vector<std::vector<int>>>);
static_assert(has_add_node_v<std::vector<std::vector<std::pair<int, float>>>>);
static_assert(has_add_node_v<std::vector<std::array<int, 10>>>);
static_assert(has_add_node_v<std::vector<std::array<std::pair<int, float>, 10>>>);

static_assert(has_add_node_v<std::vector<std::vector<bool>>>);
static_assert(has_add_node_v<std::vector<std::deque<bool>>>);
static_assert(has_add_node_v<std::vector<std::array<bool, 10>>>);
static_assert(has_add_node_v<std::vector<std::array<std::optional<float>, 10>>>);
static_assert(has_add_node_v<std::vector<std::vector<std::optional<float>>>>);
static_assert(has_add_node_v<std::vector<std::bitset<10>>>);


static_assert(has_add_node_v<std::pair<std::vector<int>, std::vector<std::tuple<int, int>>>>);
static_assert(has_add_node_v<std::pair<std::vector<int>, std::vector<std::tuple<int, int>>>, int>);
static_assert(!has_add_node_v<std::pair<std::vector<int>, std::vector<std::tuple<int, int>>>, int, int>);
static_assert(!has_add_node_v<std::pair<std::vector<int>, std::vector<std::tuple<int, int>>>, std::initializer_list<int>>);

static_assert(has_add_node_v<std::vector<std::pair<std::vector<int>, std::reference_wrapper<int>>>, int&>);
static_assert(!has_add_node_v<std::vector<std::pair<std::vector<int>, std::reference_wrapper<int>>>>);

static_assert(has_add_node_v<std::vector<std::pair<std::bitset<10>, float>>>);
static_assert(has_add_node_v<std::vector<std::pair<std::array<bool, 10>, float>>, double>);

static_assert(has_add_node_v<std::vector<std::pair<std::deque<bool>, float>>>);
static_assert(has_add_node_v<std::vector<std::pair<std::vector<std::optional<int>>, float>>, float>);
static_assert(has_add_node_v<std::vector<std::pair<std::vector<bool>, float>>>);

static_assert(has_add_node_v<std::vector<std::pair<std::array<int, 3>, float>>>);
static_assert(has_add_node_v<std::vector<std::pair<std::array<std::pair<int, float>, 3>, float>>>);

static_assert(!has_add_node_v<std::map<int, std::vector<int>>>);
static_assert(!has_add_node_v<std::map<int, std::vector<int>>, int, int>);
static_assert(has_add_node_v<std::map<std::string, std::vector<std::string>>, std::string>);
static_assert(has_add_node_v<std::map<int, std::map<int, int>>, int>);
static_assert(has_add_node_v<constexpr_modifiable_map<int, std::map<int, int>, 10>, int>);
static_assert(!has_add_node_v<constexpr_map<int, std::map<int, int>, 10>, int>);

static_assert(has_add_node_v<std::pair<std::unordered_map<std::string_view, int>, std::vector<std::tuple<std::string_view, std::string_view>>>, std::string_view, int>);


static_assert(!has_add_node_v<std::map<std::string_view, std::pair<std::map<std::string_view, int>, std::reference_wrapper<int>>>, int&>);
static_assert(!has_add_node_v<std::map<std::string_view, std::pair<std::map<std::string_view, int>, std::reference_wrapper<int>>>, std::string_view>);

static_assert(has_add_node_v<std::map<std::string_view, std::pair<std::vector<std::string_view>, std::reference_wrapper<int>>>, const char*, int&>);
static_assert(has_add_node_v<std::map<std::string_view, std::pair<std::map<std::string_view, int>, std::reference_wrapper<int>>>, std::string, int&>);

constexpr static auto drop_one = [] (auto&&, auto&& ... args) {
    return std::forward_as_tuple(std::forward<decltype(args)>(args)...);
};

template<class Graph, class GraphTraits = bxlx::graph_traits_t<Graph>>
constexpr static auto check = [] (auto&& ...args) {
    Graph g;
    constexpr std::conditional_t<GraphTraits::user_node_index, bxlx::traits::getter_t<0>, bxlx::traits::identity_t> get{};
    auto i = get(node::add_node(g, std::forward<decltype(args)>(args)...));
    auto oth = get(node::add_node(g, std::forward<decltype(args)>(args)...));

    auto expected_size = i == oth ? 1 : 2;
    if (!node::has_node(g, i))
        return false;

    if (std::size(GraphTraits::get_nodes(g)) != expected_size)
        return false;

    if constexpr (GraphTraits::representation == bxlx::traits::graph_representation::adjacency_list &&
        GraphTraits::out_edge_container_size != 0) {
        for (auto& e : GraphTraits::out_edges(node::get_node(g, i))) {
            if (GraphTraits::edge_target(e) != GraphTraits::invalid)
                return false;

            if constexpr (GraphTraits::has_edge_property) {
                constexpr typename GraphTraits::edge_property_type default_edge{};
                if (GraphTraits::get_edge_property(e) != default_edge)
                    return false;
            }
        }
    }
    if constexpr (GraphTraits::representation == bxlx::traits::graph_representation::adjacency_matrix) {
        auto&& edges = std::as_const(GraphTraits::out_edges(node::get_node(g, i)));

        for (std::size_t ix{}, size = std::size(edges); ix != size; ++ix) {
            if constexpr (bxlx::detail2::has_subscript_operator<node::out_edge_container_t<GraphTraits>>) {
                if (edges[ix])
                    return false;
            } else {
                if (*(std::begin(edges) + ix))
                    return false;
            }
        }
        if constexpr (GraphTraits::out_edge_container_size == 0) {
            for (auto& node : GraphTraits::get_nodes(g)) {
                if (std::size(GraphTraits::out_edges(node)) != expected_size)
                    return false;
            }
        }
    }

    if constexpr (GraphTraits::has_node_property) {
        if constexpr (!GraphTraits::user_node_index) {
            typename GraphTraits::node_property_type value{std::forward<decltype(args)>(args)...};
            if (node::get_node_property(g, i) != value)
                return false;
        } else {
            auto value = std::make_from_tuple<typename GraphTraits::node_property_type>(drop_one(std::forward<decltype(args)>(args)...));
            if (node::get_node_property(g, i) != value)
                return false;
        }
    }

    return true;
};
static_assert(check<constexpr_vector<std::initializer_list<int>, 3>>());
static_assert(check<constexpr_vector<constexpr_vector<int, 3>, 3>>());
static_assert(check<constexpr_vector<constexpr_vector<constexpr_pair<int, bool>, 3>, 3>>());
static_assert(check<constexpr_vector<std::array<int, 3>, 3>>());
static_assert(check<constexpr_vector<std::array<constexpr_pair<int, bool>, 3>, 3>>());

static_assert(check<constexpr_vector<constexpr_vector<bool, 3>, 3>>());
static_assert(check<constexpr_vector<std::array<bool, 3>, 3>>());
static_assert(check<constexpr_vector<std::array<std::optional<float>, 3>, 3>>());
static_assert(check<constexpr_vector<constexpr_vector<std::optional<float>, 3>, 3>>());
static_assert(check<constexpr_vector<std::bitset<10>, 3>>());


static_assert(check<constexpr_pair<constexpr_vector<int, 4>, constexpr_vector<constexpr_pair<int, int>, 4>>>(7));

constexpr const int V = 3;
static_assert(check<constexpr_vector<constexpr_pair<constexpr_vector<int, 2>, const int*>, 4>>(&V));

static_assert(check<constexpr_vector<constexpr_pair<std::bitset<10>, const int*>, 4>>(&V));
static_assert(check<constexpr_vector<constexpr_pair<std::array<bool, 10>, const int*>, 4>>(&V));
static_assert(check<constexpr_vector<constexpr_pair<std::array<std::optional<float>, 10>, const int*>, 4>>(&V));


static_assert(check<constexpr_vector<constexpr_pair<constexpr_vector<bool, 2>, std::nullptr_t>, 4>>(nullptr));
static_assert(check<constexpr_vector<constexpr_pair<constexpr_vector<const int*, 2>, std::nullptr_t>, 4>>(nullptr));

static_assert(check<constexpr_vector<constexpr_pair<std::array<int, 2>, std::nullptr_t>, 4>>(nullptr));
static_assert(check<constexpr_vector<constexpr_pair<std::array<constexpr_pair<int, float>, 2>, std::nullptr_t>, 4>>(nullptr));

static_assert(check<constexpr_modifiable_map<int, constexpr_vector<int, 1>, 2>>(0));
static_assert([] {
    struct {
        std::size_t ix{};
        constexpr operator int() {
            return ix++;
        }
    } changable_index;
    return check<constexpr_modifiable_map<int, constexpr_vector<int, 1>, 2>>(changable_index);
}());

static_assert(check<constexpr_modifiable_map<std::string_view, constexpr_map<std::string_view, int, 1>, 4>>("helo"));
static_assert(check<constexpr_pair<constexpr_modifiable_map<std::string_view, const int*, 4>, constexpr_vector<constexpr_pair<std::string_view, std::string_view>, 2>>>("helo", &V));


static_assert(check<constexpr_modifiable_map<std::string_view, constexpr_pair<constexpr_vector<std::string_view, 1>, const int*>, 3>>("hello", &V));
static_assert(check<constexpr_modifiable_map<std::string_view, constexpr_pair<constexpr_modifiable_map<std::string_view, bool, 1>, const int*>, 3>>("hello", &V));

static_assert([] {
    struct {
        std::size_t ix{};
        std::string_view arr[2] = {"1", "2"};
        constexpr operator std::string_view() {
            return arr[ix++];
        }
    } changable_index;
    return check<constexpr_modifiable_map<std::string_view, constexpr_pair<constexpr_vector<std::string_view, 1>, const int*>, 3>>(changable_index, &V);
}());

