#define MIXIN_TRAIT_TESTS

#ifdef MIXIN_TRAIT_TESTS
#   include "mixin-traits.hpp"

// the `impl' class provides a cast template through the CRTP pattern.
// mixins don't do direct inheritance on themselves,
// that's what mixin_traits::depends is for.

namespace mixins::traits_detail {

struct A {};
struct B {};
struct C {};
struct D {};

template<> struct mixin_traits<A>
{
    using depends = tuple<>;
};

template<> struct mixin_traits<B>
{
    using depends = tuple<A>;
};

template<> struct mixin_traits<C>
{
    using depends = tuple<A>;
};

template<> struct mixin_traits<D>
{
    using depends = tuple<C>;
};

extern void test1();

void test1()
{
    struct U : B, A {};
    struct V : D {};
    struct W : C, A {};
    struct Q : virtual W, virtual D {};

//#define SHOULD_NOT_COMPILE
#ifdef SHOULD_NOT_COMPILE
    (void)impl<Q, W>{};     // W not a mixin
    (void)impl<V, A>{};     // A
    (void)impl<V, D>{};     // D => C => A
    (void)impl<V, D>{};     // D => C => A
    (void)impl<W, C, B>{};  // B
#else
    (void)impl<U, B>{};
    (void)impl<W, C>{};
    (void)impl<Q, D, A>{};
#endif
}

} // ns mixins::traits_detail

#endif
