#ifndef MAT_WRAPPERS_H
#define MAT_WRAPPERS_H
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>



// This file defines wrappers around library matrices or user defined matrices.
// If you want for example to use some other library than opencv, you can specialize the 'Matrix' class for your particular implementation.
// The base 'M' must be inherited


// The typedefs needed are:
//
//  ValueType      ->   inter type of matrix

// The methods needed are:
//
// int rows const ()    ->   number of rows
// int cols const ()    ->   number of cols
//
// ValueType& operator() (int, int)                 ->    reference acessor
// const ValueType& operator() const (int, int)     ->    const reference acessor
//
//  template <typename U> static Matrix<M> (int, int)       ->   returns a new 'Matrix' (with base M) with internal type 'U'


namespace knl
{

namespace impl
{

template <class M>
struct Matrix
{
public:

    using Type = M;

    using BaseType = std::decay_t<M>;

    using ReturnType = std::decay_t<decltype(std::declval<M>()(0, 0))>;


    M mat;




    decltype(auto) operator() (int i, int j)
    {
        return wrp::Wrapper<ReturnType&>(mat.operator()(i, j));
    }

    const decltype(auto) operator() (int i, int j) const
    {
        return wrp::Wrapper<const ReturnType&>(mat.operator()(i, j));
    }
};

}



template <class...>
struct Matrix;


template <class M>
struct Matrix<M> : public Matrix<M, std::decay_t<M>>
{
    using Matrix<M, std::decay_t<M>>::Matrix;
    using ValueType = typename Matrix<M, std::decay_t<M>>::ValueType;
};



template <class M, typename T>
struct Matrix<M, cv::Mat_<T>> : public ::wrp::impl::Wrapper<M>
{
    using Type = M;

    using BaseType = cv::Mat_<T>;

    using ValueType = T;

    using ::wrp::impl::Wrapper<M>::Wrapper;

    using ::wrp::impl::Wrapper<M>::t;

    using ::wrp::impl::Wrapper<M>::operator BaseType&;



    /*Type t;



    Matrix (cv::Mat_<T>& m) : t(m) {}
    Matrix (cv::Mat_<T>&& m) : t(std::move(m)) {}
    Matrix (const cv::Mat_<T>& m) : t(m) {}


    operator cv::Mat_<T>& () { return t; }
    operator const cv::Mat_<T>& () const { return t; }*/


    int rows () { return t.rows; }

    int cols () { return t.cols; }



    decltype(auto) operator() (int i, int j)
    {
        return wrp::Wrapper<ValueType&>(t.operator()(i, j));
    }

    const decltype(auto) operator() (int i, int j) const
    {
        return wrp::Wrapper<const ValueType&>(t.operator()(i, j));
    }

    template <typename U>
    static auto create (U&&, int rows, int cols)
    {
        return Matrix<cv::Mat_<U>>(cv::Mat_<U>(rows, cols));
    }
};


/*template <class M>
class Matrix;



template <typename T>
struct Matrix<cv::Mat_<T>> : public cv::Mat_<T>
{
public:

    using Base = cv::Mat_<T>;

    using Base::Base;

    using ValueType = T;


    decltype(auto) operator() (int i, int j)
    {
        return wrp::Wrapper<ValueType&>(Base::operator()(i, j));
    }

    const decltype(auto) operator() (int i, int j) const
    {
        return wrp::Wrapper<const ValueType&>(Base::operator()(i, j));
    }

    template <typename U>
    static auto create (U&&, int rows, int cols)
    {
        return Matrix<cv::Mat_<U>>(rows, cols);
    }
};*/


}   //namespace knl

#endif
