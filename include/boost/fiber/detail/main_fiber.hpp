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
    static ptr_t make_pointer( main_fiber & n);

    main_fiber();

    void deallocate_object()
    {}

    void resume( fiber_base * other)
    {
        BOOST_ASSERT( other);

        other->yield_to( this);
    }

private:
    main_fiber( main_fiber const&);
    main_fiber & operator=( main_fiber const&);

    void yield_to( worker_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( other->caller);

        // start worker-fiber
        other->caller();

        // set by the scheduler-algorithm
        BOOST_ASSERT( is_running() );
    }

    void yield_to( main_fiber *)
    { BOOST_ASSERT_MSG( false, "main-fiber can not resume main-fiber"); }
};

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_MAIN_FIBER_H
