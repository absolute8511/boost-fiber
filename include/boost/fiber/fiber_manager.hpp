//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_FIBER_MANAGER_H
#define BOOST_FIBERS_FIBER_MANAGER_H

#include <vector>
#include <queue>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/thread/lock_types.hpp> 
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>
#include <boost/fiber/detail/main_fiber.hpp>
#include <boost/fiber/detail/spinlock.hpp>
#include <boost/fiber/detail/worker_fiber.hpp>
#include <boost/fiber/fiber.hpp>

#include <boost/scoped_ptr.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4251 4275)
# endif

namespace boost {
namespace fibers {

struct sched_algorithm
{
    virtual ~sched_algorithm() {}
    virtual void awakened( detail::fiber_base::ptr_t const&) = 0;
    virtual detail::fiber_base::ptr_t pick_next() = 0;
    virtual void priority( detail::fiber_base::ptr_t const&, int) BOOST_NOEXCEPT = 0;
};

struct fiber_manager : private noncopyable
{
    struct schedulable
    {
        detail::fiber_base::ptr_t   f;
        clock_type::time_point      tp;

        schedulable( detail::fiber_base::ptr_t const& f_,
                     clock_type::time_point const& tp_ =
                        (clock_type::time_point::max)() ) :
            f( f_), tp( tp_)
        { BOOST_ASSERT( f); }

        bool operator>(schedulable const& other) const {
            return tp > other.tp;
        }
    };

    fiber_manager();

    virtual ~fiber_manager();

    void set_sched_algo( sched_algorithm * algo) {
        sched_algo_ = algo;
        def_algo_.reset();
    }

    void spawn( detail::worker_fiber::ptr_t const&);

    void priority( detail::fiber_base::ptr_t const& f, int prio) BOOST_NOEXCEPT
    { sched_algo_->priority( f, prio); }

    void join( detail::worker_fiber::ptr_t const&);

    detail::fiber_base::ptr_t active() BOOST_NOEXCEPT
    { return active_fiber_; }

    void wait( unique_lock< detail::spinlock > &);
    bool wait_until( clock_type::time_point const&,
                     unique_lock< detail::spinlock > &);
    template< typename Rep, typename Period >
    bool wait_for( chrono::duration< Rep, Period > const& timeout_duration,
                   unique_lock< detail::spinlock > & lk)
    { return wait_until( clock_type::now() + timeout_duration, lk); }

    void yield();

    void run();

    clock_type::time_point next_wakeup()
    {
        if ( wqueue_.empty() )
            return (clock_type::time_point::max)();
        else
            return wqueue_.top().tp;
    }

    void migrate( detail::worker_fiber::ptr_t const&);

private:
    void resume_fiber( detail::fiber_base::ptr_t const&);

    class wqueue_t : public std::priority_queue<
        schedulable,
        std::vector< schedulable >,
        std::greater< schedulable > 
    >
    {
    public:
        typedef std::vector< schedulable >::iterator    iterator;

        iterator begin()
        { return c.begin(); }

        iterator end()
        { return c.end(); }
    };

    scoped_ptr< sched_algorithm >   def_algo_;
    sched_algorithm             *   sched_algo_;

    detail::main_fiber              main_fiber_;
    detail::fiber_base::ptr_t       active_fiber_;
    wqueue_t                        wqueue_;
};

}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_FIBER_MANAGER_H
