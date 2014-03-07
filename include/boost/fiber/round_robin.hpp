//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_ROUND_ROBIN_H
#define BOOST_FIBERS_ROUND_ROBIN_H

#include <deque>

#include <boost/config.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/fiber_manager.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

namespace boost {
namespace fibers {

class round_robin : public sched_algorithm
{
public:
    virtual void awakened( detail::fiber_base::ptr_t const& fib);
    virtual detail::fiber_base::ptr_t pick_next();
    virtual void priority( detail::fiber_base::ptr_t const& fib, int prio) BOOST_NOEXCEPT;

private:
    typedef std::deque< detail::fiber_base::ptr_t > rqueue_t;

    rqueue_t        rqueue_;
};

}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_ROUND_ROBIN_H
