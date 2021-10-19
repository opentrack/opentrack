#include "init.hpp"
#include <type_traits>
#include <opencv2/core/base.hpp>
#include <opencv2/core/utility.hpp>

[[noreturn]]
static
int error_handler(int, const char* fn, const char* msg, const char* filename, int line, void*)
{
    fprintf(stderr, "[%s:%d] opencv: %s at %s\n", filename, line, msg, fn);
    fflush(stderr);
    std::abort();
}

void opencv_init()
{
    cv::redirectError(error_handler);
    cv::setBreakOnError(false);
    cv::setNumThreads(1);
#ifdef OTR_HAS_CV_IPP
    cv::ipp::setUseIPP(true);
    cv::ipp::setUseIPP_NotExact(true);
#endif
}
