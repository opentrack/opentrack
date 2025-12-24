#include "compat/library-path.hpp"
#include "ftnoir_tracker_neuralnet.h"
#include <QLibrary>
#include <cstdlib>
#include <onnxruntime_cxx_api.h>

namespace neuralnet_tracker_ns
{

#if defined OPENTRACK_HAS_ONNXRUNTIME_CPU_DISPATCH && defined _WIN32 && (defined __x86_64__ || defined _M_X64 || defined __i386__ || defined _M_IX86)
#define OPENTRACK_USE_ONNXRUNTIME_CPU_DISPATCH
bool is_avx_supported()
{
    int cpuInfo[4];

    // Step 1: Check CPUID leaf 1 for AVX bit and OSXSAVE
#if defined(_MSC_VER)
    __cpuid(cpuInfo, 1);
#else
    __cpuid(1, cpuInfo[0], cpuInfo[1], cpuInfo[2], cpuInfo[3]);
#endif

    bool avxSupported = (cpuInfo[2] & (1 << 28)) != 0;     // AVX bit
    bool osxsaveSupported = (cpuInfo[2] & (1 << 27)) != 0; // OSXSAVE bit

    if (!avxSupported || !osxsaveSupported)
        return false;

    // Step 2: Check if OS has enabled AVX via XGETBV
    unsigned int xcr0 = 0;
#ifdef _MSC_VER
    xcr0 = static_cast<unsigned int>(_xgetbv(0));
#else
    unsigned int eax, edx;
    __asm__ volatile("xgetbv" : "=a"(eax), "=d"(edx) : "c"(0));
    xcr0 = eax;
#endif

    // Check if XMM and YMM states are enabled by OS
    return (xcr0 & 0x6) == 0x6;
}
#endif

void NeuralNetTracker::maybe_load_onnxruntime_dynamically()
{
#ifdef OPENTRACK_USE_ONNXRUNTIME_CPU_DISPATCH
    QLibrary lib;
    lib.setLoadHints(QLibrary::PreventUnloadHint);
    if (is_avx_supported())
        lib.setFileName(OPENTRACK_BASE_PATH
                        + OPENTRACK_LIBRARY_PATH "onnxruntime-avx"
                                                 "." OPENTRACK_LIBRARY_EXTENSION);
    else
        lib.setFileName(OPENTRACK_BASE_PATH
                        + OPENTRACK_LIBRARY_PATH "onnxruntime-noavx"
                                                 "." OPENTRACK_LIBRARY_EXTENSION);
    qDebug() << "tracker/nn: loading onnxruntime library" << lib.fileName();
    if (!lib.load())
    {
        qDebug().nospace() << "tracker/nn: can't load onnxruntime library "
                           << ": " << lib.errorString() << ". now crashing.";
        std::abort();
    }

    void* fn_OrtGetApiBase = lib.resolve("OrtGetApiBase");
    if (!fn_OrtGetApiBase)
    {
        qDebug().nospace() << "tracker/nn: can't find OrtGetApiBase in onnxruntime: " << lib.errorString() << ". now crashing.";
        std::abort();
    }
    using OrtGetApiBase_t = const OrtApiBase*(ORT_API_CALL*)(void);
    const auto* ort_base = reinterpret_cast<OrtGetApiBase_t>(fn_OrtGetApiBase)();
    if (!ort_base)
    {
        qDebug().nospace() << "tracker/nn: can't find ort API base in onnxruntime: " << lib.errorString() << ". now crashing.";
        std::abort();
    }
    const auto* ort_api = ort_base->GetApi(ORT_API_VERSION);
    if (!ort_api)
    {
        qDebug().nospace() << "tracker/nn: can't find ort API in onnxruntime: " << lib.errorString() << ". now crashing.";
        std::abort();
    }

    Ort::Global<void>::api_ = ort_api; // see ORT_API_MANUAL_INIT in the onnx c++ header.
#endif
}

} // namespace neuralnet_tracker_ns
