#include "kernel.h"


int main()
{
    impl::Matrix<cv::Mat_<uchar>> mat(cv::imread("..//img1.jpg", 0));
     
    impl::Matrix<cv::Mat_<uchar>> mat2(cv::imread("..//img1.jpg", 0));

    int k = 5;


    std::vector<double> v(k*k, 1.0/(k*k));



    auto f = [&](auto&& w) -> uchar
    {
        return std::accumulate(w.begin(), w.end(), 0.0) / (k * k);
        //return std::inner_product(w.begin(), w.end(), v.begin(), 0.0);
    };



    Kernel<decltype(f)> kernel(f, k, k, 1);


    //auto res = kernel(mat);
    auto res2 = kernel(mat2);



    ASADAS


    //cv::namedWindow("w");
    //cv::imshow("w", res);
    cv::namedWindow("t");
    cv::imshow("t", res2);
    cv::waitKey(0);



    return 0;
}
