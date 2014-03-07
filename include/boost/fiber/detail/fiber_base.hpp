
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_FIBER_BASE_H
#define BOOST_FIBERS_DETAIL_FIBER_BASE_H

#include <cstddef>
#include <iostream>
#include <map>

#include <boost/assert.hpp>
#include <boost/atomic.hpp>
#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/move/move.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/flags.hpp>
#include <boost/fiber/detail/fss.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

# if defined(BOOST_MSVC)
# pragma warning(push)
# pragma warning(disable:4275)
# endif

namespace boost {
namespace fibers {
namespace detail {

class main_fiber;
class worker_fiber;

class BOOST_FIBERS_DECL fiber_base
{
public:
    typedef intrusive_ptr< fiber_base >     ptr_t;

    class id
    {
    private:
        fiber_base::ptr_t  impl_;

    public:
        id() BOOST_NOEXCEPT :
            impl_()
        {}

        explicit id( fiber_base::ptr_t impl) BOOST_NOEXCEPT :
            impl_( impl)
        {}

        bool operator==( id const& other) const BOOST_NOEXCEPT
        { return impl_ == other.impl_; }

        bool operator!=( id const& other) const BOOST_NOEXCEPT
        { return impl_ != other.impl_; }
        
        bool operator<( id const& other) const BOOST_NOEXCEPT
        { return impl_ < other.impl_; }
        
        bool operator>( id const& other) const BOOST_NOEXCEPT
        { return other.impl_ < impl_; }
        
        bool operator<=( id const& other) const BOOST_NOEXCEPT
        { return ! ( * this > other); }
        
        bool operator>=( id const& other) const BOOST_NOEXCEPT
        { return ! ( * this < other); }

        template< typename charT, class traitsT >
        friend std::basic_ostream< charT, traitsT > &
        operator<<( std::basic_ostream< charT, traitsT > & os, id const& other)
        {
            if ( 0 != other.impl_)
                return os << other.impl_;
            else
                return os << "{not-valid}";
        }

        operator bool() const BOOST_NOEXCEPT
        { return 0 != impl_; }

        bool operator!() const BOOST_NOEXCEPT
        { return 0 == impl_; }
    };

    fiber_base();

    virtual ~fiber_base()
    { BOOST_ASSERT( is_terminated() ); }

    bool is_terminated() const BOOST_NOEXCEPT
    { return TERMINATED == state_; }

    bool is_ready() const BOOST_NOEXCEPT
    { return READY == state_; }

    bool is_running() const BOOST_NOEXCEPT
    { return RUNNING == state_; }

    bool is_waiting() const BOOST_NOEXCEPT
    { return WAITING == state_; }

    void set_terminated() BOOST_NOEXCEPT;

    void set_ready() BOOST_NOEXCEPT;

    void set_running() BOOST_NOEXCEPT;

    void set_waiting() BOOST_NOEXCEPT;

    id get_id() const BOOST_NOEXCEPT
    { return id( const_cast< fiber_base * >( this) ); }

    int priority() const BOOST_NOEXCEPT
    { return priority_; }

    void priority( int prio) BOOST_NOEXCEPT
    { priority_ = prio; }

    bool interruption_blocked() const BOOST_NOEXCEPT
    { return 0 != ( flags_.load() & flag_interruption_blocked); }

    void interruption_blocked( bool blck) BOOST_NOEXCEPT;

    bool interruption_requested() const BOOST_NOEXCEPT
    { return 0 != ( flags_.load() & flag_interruption_requested); }

    void request_interruption( bool req) BOOST_NOEXCEPT;

    bool thread_affinity() const BOOST_NOEXCEPT
    { return 0 != ( flags_.load() & flag_thread_affinity); }

    void thread_affinity( bool req) BOOST_NOEXCEPT;

    void * get_fss_data( void const* vp) const;

    void set_fss_data(
        void const* vp,
        fss_cleanup_function::ptr_t const& cleanup_fn,
        void * data,
        bool cleanup_existing);

    virtual void resume( fiber_base * other) = 0;

    friend inline void intrusive_ptr_add_ref( fiber_base * p) BOOST_NOEXCEPT
    { ++p->use_count_; }

    friend inline void intrusive_ptr_release( fiber_base * p)
    { if ( --p->use_count_ == 0) p->deallocate_object(); }

protected:
    enum state_t
    {
        READY = 0,
        RUNNING,
        WAITING,
        TERMINATED
    };

    struct BOOST_FIBERS_DECL fss_data
    {
        void                       *   vp;
        fss_cleanup_function::ptr_t     cleanup_function;

        fss_data() :
            vp( 0), cleanup_function( 0)
        {}

        fss_data(
                void * vp_,
                fss_cleanup_function::ptr_t const& fn) :
            vp( vp_), cleanup_function( fn)
        { BOOST_ASSERT( cleanup_function); }

        void do_cleanup()
        { ( * cleanup_function)( vp); }
    };

    typedef std::map< uintptr_t, fss_data >   fss_data_t;

    fss_data_t              fss_data_;
    atomic< state_t >       state_;
    atomic< int >           priority_;

    virtual void deallocate_object() = 0;
    virtual void yield_to( worker_fiber *) = 0;
    virtual void yield_to( main_fiber *) = 0;

private:
    friend class main_fiber;
    friend class worker_fiber;

    atomic< std::size_t >   use_count_;
    atomic< int >           flags_;

    fiber_base( fiber_base const&);
    fiber_base & operator=( fiber_base const&);
};

}}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_FIBER_BASE_H
