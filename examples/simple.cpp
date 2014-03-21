#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/bind.hpp>
#include <boost/intrusive_ptr.hpp>

#include <boost/fiber/all.hpp>

inline
void fn( std::string const& str, int n)
{
	for ( int i = 0; i < n; ++i)
	{
		std::cout << i << ": " << str << std::endl;
		boost::this_fiber::yield();
	}
    std::cout << str << " finished" << std::endl;
}

int main()
{
    try
    {
        boost::fibers::fiber f1( boost::bind( fn, "abc", 1) );
        boost::fibers::fiber f2( boost::bind( fn, "xyz", 2) );

        f1.join();
        f2.join();

        std::cout << "done." << std::endl;

        return EXIT_SUCCESS;
    }
	catch ( std::exception const& e)
	{ std::cerr << "exception: " << e.what() << std::endl; }
	catch (...)
	{ std::cerr << "unhandled exception" << std::endl; }
	return EXIT_FAILURE;
}
