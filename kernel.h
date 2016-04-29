#ifndef KERNEL_H
#define KERNEL_H
#include "KernelUtils.h"





namespace knl
{

template <class>
struct Window;

template <class>
struct RegularWindow;

template <class>
struct CircularWindow;

template <class M>
struct ReplicateBorder;


//--------------------------------------------------------------------------------


template <class F, template <class> class BorderType = ReplicateBorder, bool Circular = false>
class Kernel
{
public:

    using Function = F;

    template <class M>
    using Window = std::conditional_t<Circular, CircularWindow<M>, RegularWindow<M>>;

    template <class M>
    using Border = BorderType<M>;



    Kernel (const Function& f, int width, int height = -1, int numThreads = 1)  :
            f(f), width(width), height(height == -1 ? width : height),
            rows(2 * width + 1), cols(2 * height + 1), numThreads(numThreads) {}



    template <class... Mats>
    inline decltype(auto) operator () (Mats&&... mats)
    {
        return delegate<Mats...>(nullptr, std::forward<Mats>(mats)...);
    }


private:


    template <class M, class... Mats>
    inline decltype(auto) delegate (std::enable_if_t<std::is_same<void, std::decay_t<std::result_of_t<Function(Window<std::decay_t<M>>, Window<std::decay_t<Mats>>...)>>>::value>*,
                                    M&& m, Mats&&... mats)
    {
        execute([&](int i, int j, auto&&... ws)
        {
            [](auto&&...){}((ws.imgPos = std::make_pair(i, j))...);
            this->f(std::forward<decltype(ws)>(ws)...);

        }, m.rows(), m.cols(), std::forward<M>(m), std::forward<Mats>(mats)...);


        //return impl::choose<sizeof...(Mats) ? 1 : 0>(m, std::tie(m, mats...));
    }


    template <class M, class... Mats>
    inline decltype(auto) delegate (std::enable_if_t<!std::is_same<void, std::decay_t<std::result_of_t<Function(Window<std::decay_t<Mats>>...)>>>::value>*,
                                    M& m, Mats&&... mats)
    {
        execute([&](int i, int j, auto&&... ws)
        {
            [](auto&&...){}((ws.imgPos = std::make_pair(i, j))...);
            m(i, j) = this->f(std::forward<decltype(ws)>(ws)...);

        }, m.rows(), m.cols(), std::forward<Mats>(mats)...);


        return m;
    }


    template <class M, class... Mats>
    inline decltype(auto) delegate (std::enable_if_t<!std::is_same<void, std::decay_t<std::result_of_t<Function(Window<std::decay_t<Mats>>...)>>>::value>*,
                                    M&& m, Mats&&... mats)
    {
        std::decay_t<M> ret(std::move(m));

        execute([&](int i, int j, auto&&... ws)
        {
            [](auto&&...){}((ws.imgPos = std::make_pair(i, j))...);
            ret(i, j) = this->f(std::forward<decltype(ws)>(ws)...);

        }, m.rows(), m.cols(), std::forward<Mats>(mats)...);


        return ret;
    }



//------------------------------------------------------------------------------------------------------------



    template <class Apply, class... Ms>
    inline void execute (Apply apply, int matRows, int matCols, Ms&&... mats)
    {
        //if(numThreads > 1)
            //boost::thread(this, &Kernel::applyBorder, [&](int i, int j){ return apply(i, j, Border<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols).join();
            //boost::thread([&]{ applyBorder([&](int i, int j){ return apply(i, j, Border<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols); }).join();

        //else
            applyBorder([&](int i, int j){ return apply(i, j, Border<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols);

        applyWindow([&](int i, int j){ return apply(i, j, Window<std::decay_t<Ms>>(mats, rows, cols)...); }, matRows, matCols);
    }




    template <class Apply>
    void applyWindow (Apply apply, int matRows, int matCols)
    {
        std::vector<boost::thread> threads(numThreads - 1);

        int blockSize = (matRows - 2 * rows) / numThreads;
        int block = rows;   //// --->  "2"  trocar pra "rows / 2"


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


    int width;
    int height;

    int rows;
    int cols;

    int numThreads;

};



template <class F, template <class> class BorderType = ReplicateBorder, bool Circular = false, typename... Args>
inline auto makeKernel (F f, Args&&... args)
{
    return Kernel<F, BorderType, Circular>(f, std::forward<Args>(args)...);
}



//==========================================================================================================



template <class M>
class Window
{
public:

    using MatType = M;


    //---------------------------------------------------------------//

    struct Iterator : public std::forward_iterator_tag
    {
        Iterator (Window<M>& w) : w(w), i(0), j(0) {}

        Iterator (Window<M>& w, int i, int j) : w(w), i(i), j(j) {}


        decltype(auto) operator* ()
        {
            return w(i, j);
        }

        decltype(auto) operator++ ()
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


        int i;
        int j;
    };



    using value_type = typename std::decay_t<M>::ValueType&;

    using Pointer       = typename std::decay_t<M>::ValueType*;
    using const_Pointer = const Pointer;

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


    using ValueType = typename std::decay_t<M>::ValueType;


    Window (M& mat_, int rows_, int cols_) :
            mat(mat_), rows(rows_), cols(cols_), imgRows(mat_.rows()), imgCols(mat_.cols()),
            imgPos(std::make_pair(0, 0)), center(std::make_pair(rows_ / 2, cols_ / 2)) {}



    inline decltype(auto) operator () (int i, int j)
    {
        return this->mat(i - this->center.x() + this->imgPos.x(), j - this->center.y() + this->imgPos.y());
    }

    template <typename T, typename U>
    inline decltype(auto) operator () (const impl::Point<T, U>& p)
    {
        return this->operator()(p.x(), p.y());
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

    impl::Point<int, int> imgPos;
    impl::Point<int, int> center;

};


//========================================================================================


template <class M>
struct RegularWindow : public Window<M>
{
    using Window<M>::Window;
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
        impl::Point<int, int> pos = std::make_pair(i - this->center.x() + this->imgPos.x(),
                                             j - this->center.y() + this->imgPos.y());

        return this->mat(pos.x() < 0 ? 0 : pos.x() >= this->mat.rows() ? this->mat.rows() - 1 : pos.x(),
                         pos.y() < 0 ? 0 : pos.y() >= this->mat.cols() ? this->mat.cols() - 1 : pos.y());
    }

    template <typename T, typename U>
    inline decltype(auto) operator () (const impl::Point<T, U>& p)
    {
        return operator()(p.x(), p.y());
    }
};


}   // namespace knl



#endif // KERNEL_H
