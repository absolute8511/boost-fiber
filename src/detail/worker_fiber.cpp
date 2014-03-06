
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/detail/worker_fiber.hpp>

#include <exception>

#include <boost/bind.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/locks.hpp>

#include <boost/fiber/detail/scheduler.hpp>
#include <boost/fiber/exceptions.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

worker_fiber::worker_fiber( attributes const& attr, StackAllocator const& stack_alloc) :
    fiber_base(),
    callee( 0),
    caller(
        boost::bind( & worker_fiber::trampoline_, this, _1),
        attrs,
        stack_alloc),
    except_(),
    splk_(),
    waiting_()
{}

void
worker_fiber::trampoline_( typename coro_t::yield_type & yield)
{
    BOOST_ASSERT( yield);
    BOOST_ASSERT( ! is_terminated() );

    callee_ = & yield;
    set_running();
    suspend();

    try
    {
        BOOST_ASSERT( is_running() );
        run();
        BOOST_ASSERT( is_running() );
    }
    catch ( coro::detail::forced_unwind const&)
    {
        set_terminated();
        release();
        throw;
    }
    catch ( fiber_interrupted const&)
    { except_ = current_exception(); }
    catch (...)
    { std::terminate(); }

    set_terminated();
    release();
    suspend();

    BOOST_ASSERT_MSG( false, "fiber already terminated");
}

void
worker_fiber::release()
{
    BOOST_ASSERT( is_terminated() );

    std::vector< ptr_t > waiting;

    // get all waiting fibers
    splk_.lock();
    waiting.swap( waiting_);
    splk_.unlock();

    // notify all waiting fibers
    BOOST_FOREACH( fiber_base::ptr_t p, waiting)
    { p->set_ready(); }

    // release all fiber-specific-pointers
    BOOST_FOREACH( fss_data_t::value_type & data, fss_data_)
    { data.second.do_cleanup(); }
}

bool
worker_fiber::join( fiber_base::ptr_t const& p)
{
    unique_lock< spinlock > lk( splk_);
    if ( is_terminated() ) return false;
    waiting_.push_back( p);
    return true;
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
