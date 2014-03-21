
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/fiber/detail/worker_fiber.hpp"

#include <exception>

#include <boost/exception_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>

#include "boost/fiber/detail/main_fiber.hpp"
#include "boost/fiber/detail/scheduler.hpp"
#include "boost/fiber/exceptions.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

worker_fiber::worker_fiber( coro_t::yield_type * callee) :
    fiber_base(),
    callee_( callee),
    caller_(),
    except_(),
    splk_(),
    waiting_()
{ BOOST_ASSERT( callee_); }

worker_fiber::~worker_fiber()
{
    BOOST_ASSERT( is_terminated() );
    BOOST_ASSERT( waiting_.empty() );
}

bool
worker_fiber::join( fiber_base * p)
{
    unique_lock< spinlock > lk( splk_);
    if ( is_terminated() ) return false;
    waiting_.push_back( p);
    return true;
}

void
worker_fiber::release()
{
    set_terminated();

    std::vector< fiber_base * > waiting;

    // get all waiting fibers
    splk_.lock();
    waiting.swap( waiting_);
    splk_.unlock();

    // notify all waiting fibers
    BOOST_FOREACH( fiber_base * p, waiting)
    { p->set_ready(); }

    // release all fiber-specific-pointers
    BOOST_FOREACH( fss_data_t::value_type & data, fss_data_)
    { data.second.do_cleanup(); }

    // Note: jump to next fiber
    //suspend();
    scheduler::instance()->run();
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
