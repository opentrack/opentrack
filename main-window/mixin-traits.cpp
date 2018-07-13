#define MIXIN_TRAIT_TESTS

#ifdef MIXIN_TRAIT_TESTS
#   include "mixin-traits.hpp"

namespace mixins::traits_detail {

struct A {};
struct B : A {};
struct C {};

template<> struct mixin_traits<B>
{
    using depends = tuple<A>;
};

template<> struct mixin_traits<A>
{
    using depends = tuple<B>;
};

template<> struct mixin_traits<C>
{
    using depends = tuple<B, A>;
};

extern void test1();

void test1()
{
    //impl<A> fail1;
    impl<B> ok1;
}

} // ns mixins::traits_detail

#endif
