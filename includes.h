#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <boost/thread.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../Benchmark.h"

namespace impl
{

struct nullType {};


template <class>
struct PrintType;



template <bool Cond, typename T, typename U>
inline decltype(auto) choose (T&& t, U&&, std::enable_if_t<Cond>* = nullptr)
{
    return std::forward<T>(t);
}

template <bool Cond, typename T, typename U>
inline decltype(auto) choose (T&&, U&& u, std::enable_if_t<!Cond>* = nullptr)
{
    return std::forward<U>(u);
}



template <typename T, typename U>
struct point : public std::pair<T, U>
{

    using base = std::pair<T, U>;

    using base::base;


    constexpr inline decltype(auto) x ()
    {
        return this->first;
    }

    constexpr inline const decltype(auto) x () const
    {
        return this->first;
    }


    constexpr inline decltype(auto) y ()
    {
        return this->second;
    }

    constexpr inline const decltype(auto) y () const
    {
        return this->second;
    }


    template <typename V, typename W>
    inline auto operator + (const std::pair<V, W>& p) const
    {
        return point(std::make_pair(this->first + p.first, this->second + p.second));
    }

    template <typename V, typename W>
    inline auto operator - (const std::pair<V, W>& p) const
    {
        return point(std::make_pair(this->first - p.first, this->second - p.second));
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



template <class>
class Matrix;


template <typename T>
struct Matrix<cv::Mat_<T>> : public cv::Mat_<T>
{
public:

    using cv::Mat_<T>::Mat_;

    using base = cv::Mat_<T>;

    using ValueType = T;


    inline decltype(auto) operator () (int i, int j)
    {
        return (base::operator[](i))[j];
    }

    template <typename U, typename V>
    inline decltype(auto) operator () (const point<U, V>& p)
    {
        return operator()(p.x(), p.y());
    }


    template <typename U>
    static inline auto create (U&&, int rows, int cols)
    {
        return Matrix<cv::Mat_<U>>(rows, cols);
    }

};

}
