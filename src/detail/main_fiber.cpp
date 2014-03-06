//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/detail/main_fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

fiber_base::ptr_t
main_fiber::make_pointer( main_fiber & n)
{
    ptr_t p( & n);
    intrusive_ptr_add_ref( p.get() );
    return p;
}

main_fiber::main_fiber() :
    fiber_base()
{
    thread_affinity(true);
    set_running();
}

main_fiber::~main_fiber()
{
    BOOST_FOREACH( fss_data_t::value_type & data, fss_data_)
    { data.second.do_cleanup(); }
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
