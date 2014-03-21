
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_FIBERS_DETAIL_FIFO_H
#define BOOST_FIBERS_DETAIL_FIFO_H

#include <algorithm>
#include <cstddef>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/utility.hpp>

#include <boost/fiber/detail/config.hpp>
#include <boost/fiber/detail/fiber_base.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_PREFIX
#endif

namespace boost {
namespace fibers {
namespace detail {

class fifo : private noncopyable
{
public:
    fifo() BOOST_NOEXCEPT :
        head_( 0),
        tail_( 0)
    {}

    bool empty() const BOOST_NOEXCEPT
    { return 0 == head_; }

    void push( fiber_base * item) BOOST_NOEXCEPT
    {
        BOOST_ASSERT( 0 != item);
        BOOST_ASSERT( 0 == item->next() );

        if ( empty() )
            head_ = tail_ = item;
        else
        {
            tail_->next( item);
            tail_ = item;
        }
    }

    fiber_base * pop() BOOST_NOEXCEPT
    {
        BOOST_ASSERT( ! empty() );

        fiber_base * item = head_;
        head_ = head_->next();
        if ( 0 == head_) tail_ = 0;
        item->next_reset();
        return item;
    }

    template< typename SchedAlgo, typename Fn >
    void move_to( SchedAlgo * sched_algo, Fn fn)
    {
        BOOST_ASSERT( sched_algo);

        fiber_base * f = head_, * prev = 0;
        while ( 0 != f)
        {
            fiber_base * nxt = f->next();
            if ( fn( f) )
            {
                if ( f == head_)
                {
                    BOOST_ASSERT( 0 == prev);

                    head_ = nxt;
                    if ( 0 == head_)
                        tail_ = 0;
                }
                else
                {
                    BOOST_ASSERT( 0 != prev);

                    if ( 0 == nxt)
                        tail_ = prev;

                    prev->next( nxt); 
                }
                f->next_reset();
                sched_algo->awakened( f);
            }
            else
                prev = f;
            f = nxt;
        }
    }

    void swap( fifo & other)
    {
        std::swap( head_, other.head_);
        std::swap( tail_, other.tail_);
    }

private:
    fiber_base    *  head_;
    fiber_base    *  tail_;
};

}}}

# if defined(BOOST_MSVC)
# pragma warning(pop)
# endif

#ifdef BOOST_HAS_ABI_HEADERS
#  include BOOST_ABI_SUFFIX
#endif

#endif // BOOST_FIBERS_DETAIL_FIFO_H
