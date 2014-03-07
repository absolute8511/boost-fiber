
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_WORKER_FIBER_H
#define BOOST_FIBERS_DETAIL_WORKER_FIBER_H

#include <cstddef>
#include <vector>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/coroutine/symmetric_coroutine.hpp>
#include <boost/exception_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/fiber/attributes.hpp>
#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/fiber/stack_allocator.hpp>

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
    typedef intrusive_ptr< worker_fiber >     ptr_t;

    worker_fiber(attributes const& attrs, stack_allocator const& stack_alloc);

    virtual ~worker_fiber()
    { BOOST_ASSERT( waiting_.empty() ); }

    bool join( fiber_base::ptr_t const&);

    bool has_exception() const BOOST_NOEXCEPT
    { return except_; }

    void rethrow() const
    {
        BOOST_ASSERT( has_exception() );

        rethrow_exception( except_);
    }

    void resume( fiber_base * other)
    {
        BOOST_ASSERT( other);

        other->yield_to( this);
    }

protected:
    friend class main_fiber;

    typedef coro::symmetric_coroutine<
        void, stack_allocator
    >                                   coro_t;

    typename coro_t::yield_type     *   callee;
    typename coro_t::call_type          caller;

    virtual void run() = 0;

private:
    exception_ptr                       except_;
    spinlock                            splk_;
    std::vector< fiber_base::ptr_t >    waiting_;

    void trampoline_( typename coro_t::yield_type & yield);
    
    void set_main_running_( main_fiber *);

    void release();

    void yield_to( worker_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( other->caller);
        BOOST_ASSERT( callee);
        BOOST_ASSERT( * callee);

        // resume other worker-fiber
        ( * callee)( other->caller);

        // set by the scheduler-algorithm
        BOOST_ASSERT( is_running() );
    }

    void yield_to( main_fiber * other)
    {
        BOOST_ASSERT( other);
        BOOST_ASSERT( callee);
        BOOST_ASSERT( * callee);

        set_main_running_( other);

        // yield to main-fiber is equivalent to jump back to main-fiber
        // the main-fiber (per scheduler only one) is at the start of
        // this chain of worker-fibers (symmetric coroutines)
        // set main-fiber to running state
        ( * callee)();

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
