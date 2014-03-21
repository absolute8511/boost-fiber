//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_MAIN_FIBER_H
#define BOOST_FIBERS_DETAIL_MAIN_FIBER_H

#include <boost/atomic.hpp>
#include <boost/config.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/worker_fiber.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

class main_fiber : public fiber_base
{
public:
    main_fiber() :
        fiber_base()
    {
        thread_affinity( true);
        set_running();
    }

    void deallocate() {}

    void resume( fiber_base * other)
    {
        BOOST_ASSERT( other);

        set_running();
        other->yield_to( this);
    }

private:
    void yield_to( worker_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( other->caller_);

        // start worker-fiber
        other->caller_( fiber_base::null_ptr);

        // set the main fiber to running state
        // becaus other worker fiber terminate and
        // thus did jump back to its starting context
        set_running();
    }

    void yield_to( main_fiber *)
    { BOOST_ASSERT_MSG( false, "main-fiber can not resume main-fiber"); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_MAIN_FIBER_H
