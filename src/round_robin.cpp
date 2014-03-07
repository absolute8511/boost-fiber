
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/round_robin.hpp>

#include <boost/assert.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {

void
round_robin::awakened( detail::fiber_base::ptr_t const& f)
{ rqueue_.push_back( f); }

detail::fiber_base::ptr_t
round_robin::pick_next()
{
    detail::fiber_base::ptr_t victim;
    if ( ! rqueue_.empty() )
    {
        victim.swap( rqueue_.front() );
        rqueue_.pop_front();
    }
    return victim;
}

void
round_robin::priority( detail::fiber_base::ptr_t const& f, int prio) BOOST_NOEXCEPT
{
    BOOST_ASSERT( f);

    // set only priority to fiber
    // round-robin does not respect priorities
    f->priority( prio);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
