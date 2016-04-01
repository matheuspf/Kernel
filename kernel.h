#ifndef KERNEL_H
#define KERNEL_H
#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <boost/thread.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "../Benchmark.h"
//#include "../ThreadMat/ThreadMatrix.h"

using namespace std;




namespace impl
{

struct nullType {};


template <class>
struct printType;



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




template <class M>
struct Window
{
public:

    //---------------------------------------------------------------//

    struct Iterator : public std::forward_iterator_tag
    {
        //Iterator (const Iterator& it) : Iterator(it.mat, it.rows, it.cols, it.i, it.j) {}

        //Iterator (M& mat, int rows, int cols, int i = 0, int j = 0) : mat(mat), rows(rows), cols(cols), i(i), j(j) {}

        Iterator (Window<M>& w) : w(w), i(w.center.x()), j(w.center.y()) {}

        Iterator (Window<M>& w, int i, int j) : w(w), i(i), j(j) {}


        typename std::decay_t<M>::ValueType& operator* ()
        {
            return w(i, j);
        }

        Iterator& operator++ ()
        {
            j = (j + 1) % w.cols;
            i += !j;

            return *this;
        }

        bool operator == (const Iterator& it)
        {
            return (i == it.i) && (j == it.j);
        }

        bool operator != (const Iterator& it)
        {
            return !operator==(it);
        }


        Window<M>& w;

        /*M& mat;

        int rows;
        int cols;*/

        int i;
        int j;
    };



    using value_type = typename std::decay_t<M>::ValueType&;

    using pointer       = typename std::decay_t<M>::ValueType*;
    using const_pointer = const pointer;

    using reference       = typename std::decay_t<M>::ValueType&;
    using const_reference = const reference;

    using iterator       = Iterator;
    using const_iterator = const iterator;


    iterator begin ()
    {
        return iterator(*this, 0, 0);
    }

    const_iterator begin () const
    {
        return iterator(*this, 0, 0);
    }

    iterator end ()
    {
        return iterator(*this, rows, 0);
    }

    const_iterator end () const
    {
        return iterator(*this, rows, 0);
    }



    //---------------------------------------------------------------//



    Window (M& mat_, int rows_, int cols_) :
            mat(mat_), rows(rows_), cols(cols_), imgRows(mat_.rows), imgCols(mat_.cols),
            imgPos(std::make_pair(0, 0)), center(std::make_pair(rows_ / 2, cols_ / 2)) {}



    inline decltype(auto) operator () (int i, int j)
    {
        return this->mat(i - this->center.x() + this->imgPos.x(), j - this->center.y() + this->imgPos.y());
    }

    template <typename T, typename U>
    inline decltype(auto) operator () (const point<T, U>& p)
    {
        return operator()(p.x(), p.y());
    }


    template <class V>
    auto convolve (V&& v)
    {
        auto r = decltype(v[0][0] * this->operator()(0, 0)){};

        for(int i = 0; i < rows; ++i)
            for(int j = 0; j < cols ; ++j)
                r += v[i][j] * this->operator()(i, j);

        return r;
    }



    M& mat;

    int rows;
    int cols;

    int imgRows;
    int imgCols;

    point<int, int> imgPos;
    point<int, int> center;

};


template <class M>
struct RegularWindow : public Window<M>
{
    using Window<M>::Window;


    /*inline decltype(auto) operator () (int i, int j)
    {
        return this->mat(i - this->center.x() + this->imgPos.x(), j - this->center.y() + this->imgPos.y());
    }

    template <typename T, typename U>
    inline decltype(auto) operator () (const point<T, U>& p)
    {
        return operator()(p.x(), p.y());
    }*/

};


template <class M>
struct CircularWindow : public Window<M>
{
    using Window<M>::Window;
};



template <class M>
struct ReplicateBorder : public Window<M>
{
    using Window<M>::Window;


    inline decltype(auto) operator () (int i, int j)
    {
        point<int, int> pos = std::make_pair(i - this->center.x() + this->imgPos.x(),
                                             j - this->center.y() + this->imgPos.y());

        return this->mat(pos.x() < 0 ? 0 : pos.x() >= this->mat.rows ? this->mat.rows - 1 : pos.x(),
                         pos.y() < 0 ? 0 : pos.y() >= this->mat.cols ? this->mat.cols - 1 : pos.y());
    }

    template <typename T, typename U>
    inline decltype(auto) operator () (const point<T, U>& p)
    {
        return operator()(p.x(), p.y());
    }
};

}




template <class FunctionType, template <class> class BorderType = impl::ReplicateBorder, bool Circular = false>
class Kernel
{
public:

    using Function = FunctionType;

    template <class M>
    using Window = std::conditional_t<Circular, impl::CircularWindow<M>, impl::RegularWindow<M>>;

    template <class M>
    using Border = BorderType<M>;


    //using returnType = std::remove_reference_t<std::result_of_t<FunctionType(Window<impl::Matrix<cv::Mat_<cv::Vec3b>>>)>>;



    Kernel (const Function& f, int rows, int cols = -1, int numThreads = 1)  :
            f(f), rows(rows), cols(cols == -1 ? rows : cols), numThreads(numThreads) {}



    template <class... Ms>
    inline auto operator () (Ms&&... mats)
    {
        return delegate<std::decay_t<std::result_of_t<Function(Window<std::decay_t<Ms>>...)>>, Ms...>(std::forward<Ms>(mats)...);
    }


    template <typename Return, class M, class... Ms, typename = std::enable_if_t<std::is_same<Return, void>::value>>
    inline decltype(auto) delegate (M&& m, Ms&&... mats)
    {
        execute([&](int i, int j, auto&&... ws)
        {
            [](auto&&...){}((ws.imgPos = std::make_pair(i, j))...);
            this->f(std::forward<decltype(ws)>(ws)...);

        }, m.rows, m.cols, std::forward<M>(m), std::forward<Ms>(mats)...);


        return impl::choose<bool(sizeof...(Ms))>(std::tie(std::forward<M>(m), std::forward<Ms>(mats)...), std::forward<M>(m));
    }

    template <typename Return, class M, class... Ms, typename = std::enable_if_t<!std::is_same<Return, void>::value>>
    inline auto delegate (M&& m, Ms&&... mats)
    {
        auto res = std::decay_t<M>::create(Return(), m.rows, m.cols);


        execute([&](int i, int j, auto&&... ws)
        {
            [](auto&&...){}((ws.imgPos = std::make_pair(i, j))...);
            res(i, j) = this->f(std::forward<decltype(ws)>(ws)...);

        }, m.rows, m.cols, std::forward<M>(m), std::forward<Ms>(mats)...);


        return res;
    }



    template <class Apply, class... Ms>
    inline void execute (Apply apply, int matRows, int matCols, Ms&&... mats)
    {
        //if(numThreads > 1)
          //  boost::thread([&]{ applyBorder([&](int i, int j){ return apply(i, j, Border<std::decay_t<M>>(mat, rows, cols)); }, mat.rows, mat.cols); }).join();

        //else
            applyBorder([&](int i, int j){ return apply(i, j, Border<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols);

        applyWindow([&](int i, int j){ return apply(i, j, Window<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols);
    }




    template <class Apply>
    void applyWindow (Apply apply, int matRows, int matCols)
    {
        std::vector<boost::thread> threads(numThreads - 1);

        int blockSize = (matRows - 2 * rows) / numThreads, block = rows;   //// --->  "2"  trocar pra "rows / 2"


        static auto loop = [&](int first, int last)
        {
            for(int i = first; i < last; ++i)
                for(int j = cols; j < (matCols - cols); ++j)
                    apply(i, j);
        };


        for(int i = 0; i < numThreads - 1; ++i, block += blockSize)
            threads[i] = boost::thread(loop, block, block + blockSize);

        loop(block, matRows - rows);


        for(auto& t : threads)
            t.join();



        /*for(int i = rows; i < (matRows - rows); ++i)
            for(int j = cols; j < (matCols - cols); ++j)
                apply(i, j);*/
    }


    template <class Apply>
    void applyBorder (Apply apply, int matRows, int matCols)
    {
        static auto inner = [&](int i, int first, int last)
        {
            for(int j = first; j < last; ++j)
                apply(i, j);
        };

        static auto outter = [&](int first, int last, int innerFirst, int innerLast)
        {
            for(int i = first; i < last; ++i)
                inner(i, innerFirst, innerLast);
        };

        outter(0, rows, 0, matCols);
        outter(matRows - rows, matRows, 0, matCols);
        outter(rows, matRows - rows, 0, cols);
        outter(rows, matRows - rows, matCols - cols, matCols);
    }


private:


    Function f;

    int rows;
    int cols;
    int numThreads;

};




template <class T>
struct Traits : Traits<decltype(&T::operator())> {};


template <class C, class R, class... A>
struct Traits<R(C::*)(A...)>
{
    using ret = R;
};

template <class C, class R, class... A>
struct Traits<R(C::*)(A...) const>
{
    using ret = R;
};



template <class FunctionType, template <class> class BorderType = impl::ReplicateBorder, bool Circular = false, typename... Args>
inline auto makeKernel (FunctionType f, Args&&... args)
{
    return Kernel<FunctionType, BorderType, Circular>(f, args...);
}



#endif // KERNEL_H
