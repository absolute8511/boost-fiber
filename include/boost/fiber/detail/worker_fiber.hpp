
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_WORKER_FIBER_H
#define BOOST_FIBERS_DETAIL_WORKER_FIBER_H

#include <cstddef>
#include <map>
#include <vector>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/move/move.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/flags.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/fiber/exceptions.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251)
# endif

namespace boost {
namespace fibers {
namespace detail {

namespace coro = boost::coroutines;

class main_fiber;

class BOOST_FIBERS_DECL worker_fiber : public fiber_base
{
public:
    typedef coro::symmetric_coroutine<
        void *
    >                                          coro_t;

    worker_fiber( coro_t::yield_type *);

    ~worker_fiber();

    bool join( fiber_base *);

    exception_ptr get_exception() const BOOST_NOEXCEPT
    { return except_; }

    void set_exception( exception_ptr except) BOOST_NOEXCEPT
    { except_ = except; }

    void suspend()
    {
        BOOST_ASSERT( callee_);
        BOOST_ASSERT( * callee_);

        ( * callee_)();

        BOOST_ASSERT( is_running() ); // set by the scheduler-algorithm
    }

    void release();

    void deallocate()
    { coro_t::call_type tmp( move( caller_) ); }

private:
    friend class main_fiber;
    template< typename Fn >
    friend struct setup;

    coro_t::yield_type          *   callee_;
    coro_t::call_type               caller_;
    exception_ptr                   except_;
    spinlock                        splk_;
    std::vector< fiber_base * >     waiting_;

    void resume( fiber_base * other)
    {
        BOOST_ASSERT( other);

        other->yield_to( this); 
    }

    void yield_to( worker_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( other->caller_);
        BOOST_ASSERT( callee_);
        BOOST_ASSERT( * callee_);

        // resume other worker-fiber
        ( * callee_)( other->caller_, fiber_base::null_ptr);

        // set by the scheduler-algorithm
        BOOST_ASSERT( is_running() );
    }

    void yield_to( main_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( callee_);
        BOOST_ASSERT( * callee_);

        // yield to main-fiber is equivalent to jump back to main-fiber
        // the main-fiber (per scheduler only one) is at the start of
        // this chain of worker-fibers (symmetric coroutines)
        // set main-fiber to running state
        ( * callee_)();

        // set by the scheduler-algorithm
        BOOST_ASSERT( is_running() );
    }
};

}}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_WORKER_FIBER_H
