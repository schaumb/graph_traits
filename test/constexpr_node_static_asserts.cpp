
#include <bxlx/graph/traits/node.hpp>
#include <bitset>

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


