#include "kernel.h"
#include "../Wrapper/Matrix.h"



/*  g++ -I"C:\opencv\build\include" -I"C:\MinGW\include" -L"C:\opencv\mingw64\lib" main.cpp -g C:\MinGW\lib\libboost_thread.a C:\MinGW\lib\libboost_system.a  -lopencv_core249 -lopencv_highgui249 -std=c++14 */


using namespace std;
using namespace cv;




int main()
{
    wrp::Matrix<cv::Mat_<uchar>> mat(cv::imread("..//img1.jpg", 0));


    /*int k = 2;


    auto f = [&](auto&& w) -> uchar
    {
        return std::accumulate(w.begin(), w.end(), 0.0) / pow(2 * k + 1, 2);
        //return std::inner_product(w.begin(), w.end(), v.begin(), 0.0);
    };


    knl::Matrix<cv::Mat_<uchar>> res(mat.rows(), mat.cols());

    knl::Kernel<decltype(f)>(f, k, k)(res, mat);*/


    constexpr int width = 20;
    constexpr int rows = width * 2 + 1;


    auto dilation = [](auto&& strElem, auto thresh)
    {
        return [&](auto&& wIn, auto&& wOut)
        {
            if(thresh(wIn(wIn.center)))
            {
                for(int i = 0; i < wIn.rows; ++i)
                    for(int j = 0; j < wIn.cols; ++j)
                        wOut(i, j) = 255 * (thresh(wIn(i, j)) && strElem[i][j]);
            }
        };
    };



    array<array<bool, rows>, rows> strElem;

    for(auto& x : strElem) x.fill(1);


    auto thresh = [](auto&& x){ return x < 100; };


    wrp::Matrix<cv::Mat_<uchar>> res(mat.rows(), mat.cols());


    knl::Kernel<decltype(dilation(strElem, thresh))> kernel(dilation(strElem, thresh), width, width);

    kernel(mat, res);



    namedWindow("w");
    imshow("w", Mat_<uchar>(prt));
    waitKey(0);


    return 0;
}
