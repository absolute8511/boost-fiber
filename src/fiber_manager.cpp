
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/fiber_manager.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>

#include <boost/assert.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/scope_exit.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/thread.hpp>

#include <boost/fiber/detail/scheduler.hpp>
#include <boost/fiber/exceptions.hpp>

#include <boost/fiber/round_robin.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {

fiber_manager::fiber_manager() BOOST_NOEXCEPT :
    def_algo_( new round_robin() ),
    sched_algo_( def_algo_.get() ),
    main_fiber_(),
    active_fiber_( & main_fiber_),
    wqueue_()
{}

fiber_manager::~fiber_manager() BOOST_NOEXCEPT
{
    // fibers will be destroyed (stack-unwinding)
    // if last reference goes out-of-scope
    // therefore destructing wqueue_ && rqueue_
    // will destroy the fibers in this scheduler
    // if not referenced on other places
    while ( ! wqueue_.empty() )
        run();
}

void
fiber_manager::spawn( detail::worker_fiber::ptr_t const& f)
{
    BOOST_ASSERT( f);
    BOOST_ASSERT( f->is_ready() );

    // push active fiber to wqueue_
    wqueue_.push( schedulable( f) );
}

void
fiber_manager::run()
{
    for (;;)
    {
        // move all fibers witch are ready (state_ready)
        // from waiting-queue to the runnable-queue
        wqueue_t wqueue;
        for ( wqueue_t::iterator it( wqueue_.begin() ), end( wqueue_.end() ); it != end; ++it)
        {
            detail::fiber_base::ptr_t f( it->f);

            BOOST_ASSERT( ! f->is_running() );
            BOOST_ASSERT( ! f->is_terminated() );

            // set fiber to state_ready if dead-line was reached or interruption was requested
            if ( it->tp <= clock_type::now() || f->interruption_requested() || f->is_ready() )
            {
                f->set_ready();
                sched_algo_->awakened( f);
            }
            else
                wqueue.push( * it);
        }

        swap( wqueue_, wqueue);

        // pop new fiber from ready-queue which is not complete
        // (example: fiber in ready-queue could be canceled by active-fiber)
        detail::fiber_base::ptr_t f( sched_algo_->pick_next() );
        if ( f)
        {
            BOOST_ASSERT_MSG( f->is_ready(), "fiber with invalid state in ready-queue");
            resume_fiber( f);
            return;
        }
        else
        {
            // no fibers ready to run; the thread should sleep
            // until earliest fiber is scheduled to run
            clock_type::time_point wakeup( next_wakeup() );
            this_thread::sleep_until( wakeup);
        }
    }
}

void fiber_manager::resume_fiber( detail::fiber_base::ptr_t const& f)
{
    BOOST_ASSERT( f);
    BOOST_ASSERT( f->is_ready() );

    // store active fiber in local var
    detail::fiber_base::ptr_t tmp( active_fiber_);
    // assign new fiber to active fiber
    active_fiber_ = f;
    // set active fiber to state_running
    active_fiber_->set_running();
    // resume active fiber
    active_fiber_->resume( tmp.get() );
    // reset previous active fiber
    active_fiber_ = tmp;
}

void
fiber_manager::wait( unique_lock< detail::spinlock > & lk)
{ wait_until( clock_type::time_point( (clock_type::duration::max)() ), lk); }

bool
fiber_manager::wait_until( clock_type::time_point const& timeout_time,
                           unique_lock< detail::spinlock > & lk)
{
    clock_type::time_point start( clock_type::now() );

    BOOST_ASSERT( active_fiber_);
    BOOST_ASSERT( active_fiber_->is_running() );

    // set active fiber to state_waiting
    active_fiber_->set_waiting();
    // release lock
    lk.unlock();
    // push active fiber to wqueue_
    wqueue_.push( schedulable( active_fiber_, timeout_time) );
    // store active fiber in local var
    detail::fiber_base::ptr_t tmp( active_fiber_);
    // switch to another fiber
    run();
    // fiber is resumed

    BOOST_ASSERT( tmp == detail::scheduler::instance()->active() );
    BOOST_ASSERT( tmp->is_running() );

    return clock_type::now() < timeout_time;
}

void
fiber_manager::yield()
{
    BOOST_ASSERT( active_fiber_);
    BOOST_ASSERT( active_fiber_->is_running() );

    // set active fiber to state_waiting
    active_fiber_->set_ready();
    // push active fiber to wqueue_
    wqueue_.push( schedulable( active_fiber_) );
    // store active fiber in local var
    detail::fiber_base::ptr_t tmp( active_fiber_);
    // switch to another fiber
    run();
    // fiber is resumed

    BOOST_ASSERT( tmp == detail::scheduler::instance()->active() );
    BOOST_ASSERT( tmp->is_running() );
}

void
fiber_manager::join( detail::worker_fiber::ptr_t const& f)
{
    BOOST_ASSERT( f);
    BOOST_ASSERT( f != active_fiber_);

    // set active fiber to state_waiting
    active_fiber_->set_waiting();
    // push active fiber to wqueue_
    wqueue_.push( schedulable( active_fiber_) );
    // add active fiber to joinig-list of f
    if ( ! f->join( active_fiber_) )
        // f must be already terminated therefore we set
        // active fiber to state_ready
        // FIXME: better state_running and no suspend
        active_fiber_->set_ready();
    // store active fiber in local var
    detail::fiber_base::ptr_t tmp( active_fiber_);
    // switch to another fiber
    run();

    BOOST_ASSERT( tmp == detail::scheduler::instance()->active() );
    BOOST_ASSERT( tmp->is_running() );

    BOOST_ASSERT( f->is_terminated() );
}

void
fiber_manager::migrate(detail::worker_fiber::ptr_t const& f)
{
    BOOST_ASSERT( f);
    BOOST_ASSERT( f->is_ready());

    sched_algo_->awakened( f);
}

}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
