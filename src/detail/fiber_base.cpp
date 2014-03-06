
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <boost/fiber/detail/fiber_base.hpp>

#include <exception>

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

fiber_base::fiber_base() :
    fss_data_(),
    state_( READY),
    priority_( 0),
    use_count_( 0),
    flags_( 0)
{}

fiber_base::~fiber_base()
{
    BOOST_ASSERT( is_terminated() );
    BOOST_ASSERT( waiting_.empty() );
}

void
fiber_base::set_terminated() BOOST_NOEXCEPT
{
    state_t previous = state_.exchange( TERMINATED);
    BOOST_ASSERT( RUNNING == previous);
}

void
fiber_base::set_ready() BOOST_NOEXCEPT
{
    state_t previous = state_.exchange( READY);
    BOOST_ASSERT( WAITING == previous || RUNNING == previous || READY == previous);
}

void
fiber_base::set_running() BOOST_NOEXCEPT
{
    state_t previous = state_.exchange( RUNNING);
    BOOST_ASSERT( READY == previous);
}

void
fiber_base::set_waiting() BOOST_NOEXCEPT
{
    state_t previous = state_.exchange( WAITING);
    BOOST_ASSERT( RUNNING == previous);
}

void
fiber_base::interruption_blocked( bool blck) BOOST_NOEXCEPT
{
    if ( blck)
        flags_ |= flag_interruption_blocked;
    else
        flags_ &= ~flag_interruption_blocked;
}

void
fiber_base::request_interruption( bool req) BOOST_NOEXCEPT
{
    if ( req)
        flags_ |= flag_interruption_requested;
    else
        flags_ &= ~flag_interruption_requested;
}

void
fiber_base::thread_affinity( bool req) BOOST_NOEXCEPT
{
    if ( req)
        flags_ |= flag_thread_affinity;
    else
        flags_ &= ~flag_thread_affinity;
}

void *
fiber_base::get_fss_data( void const* vp) const
{
    uintptr_t key( reinterpret_cast< uintptr_t >( vp) );
    fss_data_t::const_iterator i( fss_data_.find( key) );

    return fss_data_.end() != i ? i->second.vp : 0;
}

void
fiber_base::set_fss_data(
    void const* vp,
    fss_cleanup_function::ptr_t const& cleanup_fn,
    void * data, bool cleanup_existing)
{
    BOOST_ASSERT( cleanup_fn);

    uintptr_t key( reinterpret_cast< uintptr_t >( vp) );
    fss_data_t::iterator i( fss_data_.find( key) );

    if ( fss_data_.end() != i)
    {
        if( cleanup_existing) i->second.do_cleanup();
        if ( data)
            fss_data_.insert(
                    i,
                    std::make_pair(
                        key,
                        fss_data( data, cleanup_fn) ) );
        else fss_data_.erase( i);
    }
    else
        fss_data_.insert(
            std::make_pair(
                key,
                fss_data( data, cleanup_fn) ) );
}

}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif
