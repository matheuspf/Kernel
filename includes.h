#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <boost/thread.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../Benchmark.h"
#include "wrappers.h"


namespace knl       // Main namespace
{

namespace impl      // implers
{

struct NullType {};     // Empty class for convenience

template <class> struct PrintType;       // "Prints" type of variable in compile time.



// This function decides between two variables on compile time depending on value of parameter 'Condition'
// With optimazations enabled on g++ (-O2) the generated assembly is identical to the assignment of the choosen variable

template <bool Condition, typename T, typename U>
inline decltype(auto) choose (T&& t, U&&, std::enable_if_t<Condition>* = nullptr)
{
    return std::forward<T>(t);
}

template <bool Condition, typename T, typename U>
inline decltype(auto) choose (T&&, U&& u, std::enable_if_t<!Condition>* = nullptr)
{
    return std::forward<U>(u);
}



// Handy class for manipulation of Points in 2d space. Inherits from std::pair<T, U> for convenience.

template <typename T, typename U>
struct Point : public std::pair<T, U>
{
    using base = std::pair<T, U>;

    using base::base;


    // Acessors: 'Point<T, U>::first' does not makes sense. 'Point<T, U>::x()'' does, and have the exact same performance as acessing the variable
    // No way to create 'Point<T, U>::x' without incurring the overhead of creating a new variable

    constexpr inline decltype(auto) x () { return this->first; }
    constexpr inline const decltype(auto) x () const { return this->first; }

    constexpr inline decltype(auto) y () { return this->second; }
    constexpr inline const decltype(auto) y () const { return this->second; }


    template <typename V, typename W>
    inline auto operator + (const std::pair<V, W>& p) const
    {
        return Point(std::make_pair(this->first + p.first, this->second + p.second));
    }

    template <typename V, typename W>
    inline auto operator - (const std::pair<V, W>& p) const
    {
        return Point(std::make_pair(this->first - p.first, this->second - p.second));
    }


    template <typename V, typename W>
    inline decltype(auto) operator += (const std::pair<V, W>& p)
    {
        this->first  += p.first;
        this->second += p.second;

        return *this;
    }

    template <typename V, typename W>
    inline decltype(auto) operator -= (const std::pair<V, W>& p)
    {
        this->first  -= p.first;
        this->second -= p.second;

        return *this;
    }
};

}   //namespace impl

}   //namespace knl
