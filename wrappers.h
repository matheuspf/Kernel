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

template <class M>
class Matrix;

template <typename T>
struct Matrix<cv::Mat_<T>> : public cv::Mat_<T>
{
public:

    using Base = cv::Mat_<T>;

    using Base::Base;

    using ValueType = T;


    ValueType& operator() (int i, int j)
    {
        return Base::operator()(i, j);
    }

    const ValueType& operator() (int i, int j) const
    {
        return Base::operator()(i, j);
    }

    template <typename U>
    static auto create (U&&, int rows, int cols)
    {
        return Matrix<cv::Mat_<U>>(rows, cols);
    }
};


}   //namespace knl
