
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "boost/fiber/mutex.hpp"

#include <algorithm>

#include <boost/assert.hpp>

#include "boost/fiber/interruption.hpp"
#include "boost/fiber/operations.hpp"

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {

bool
mutex::lock_if_unlocked_()
{
    if ( UNLOCKED != state_) return false;
    
    state_ = LOCKED;
    BOOST_ASSERT( ! owner_);
    owner_ = this_fiber::get_id();
    return true;
}

mutex::mutex() :
    splk_(),
	state_( UNLOCKED),
    owner_(),
    waiting_()
{}

mutex::~mutex()
{
    BOOST_ASSERT( ! owner_);
    BOOST_ASSERT( waiting_.empty() );
}

void
mutex::lock()
{
    detail::fiber_base * n( fm_active() );
    if ( 0 != n)
    {
        for (;;)
        {
            unique_lock< detail::spinlock > lk( splk_);

            if ( lock_if_unlocked_() ) return;

            // store this fiber in order to be notified later
            BOOST_ASSERT( waiting_.end() == std::find( waiting_.begin(), waiting_.end(), n) );
            waiting_.push_back( n);

            // suspend this fiber
            fm_wait( lk);
        }
    }
    else
    {
        // notification for main-fiber
        detail::main_fiber mf;
        n = & mf;

        for (;;)
        {
            unique_lock< detail::spinlock > lk( splk_);

            if ( lock_if_unlocked_() ) return;

            // store this fiber in order to be notified later
            BOOST_ASSERT( waiting_.end() == std::find( waiting_.begin(), waiting_.end(), n) );
            waiting_.push_back( n);
            lk.unlock();

            // wait until main-fiber gets notified
            while ( ! n->is_ready() )
                // run scheduler
                fm_run();
        }
    }
}

bool
mutex::try_lock()
{
    unique_lock< detail::spinlock > lk( splk_);

    if ( lock_if_unlocked_() ) return true;

    lk.unlock();
    // let other fiber release the lock
    this_fiber::yield();
    return false;
}

void
mutex::unlock()
{
    BOOST_ASSERT( LOCKED == state_);
    BOOST_ASSERT( this_fiber::get_id() == owner_);

    unique_lock< detail::spinlock > lk( splk_);
    detail::fiber_base * n = 0;
    if ( ! waiting_.empty() )
    {
        n = waiting_.front();
        waiting_.pop_front();
    }
    owner_ = detail::worker_fiber::id();
	state_ = UNLOCKED;
    lk.unlock();
    if ( n) n->set_ready(false);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
