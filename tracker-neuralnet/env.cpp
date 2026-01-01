#include "compat/library-path.hpp"
#include "ftnoir_tracker_neuralnet.h"
#include <QLibrary>
#include <cstdlib>
#include <onnxruntime_cxx_api.h>

namespace neuralnet_tracker_ns
{

void NeuralNetTracker::maybe_load_onnxruntime_dynamically()
{
#if defined _WIN32 || (defined __unix__ || defined __unix) && !defined __APPLE__
    QLibrary lib;
    lib.setLoadHints(QLibrary::PreventUnloadHint);
    lib.setFileName(
#ifdef _WIN32
        OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH
#endif
        "onnxruntime" "." OPENTRACK_LIBRARY_EXTENSION);
    qDebug() << "tracker/nn: loading onnxruntime library" << lib.fileName();
    if (!lib.load())
    {
        qDebug().nospace() << "tracker/nn: can't load onnxruntime library "
                           << ": " << lib.errorString() << ". now crashing.";
        std::abort();
    }

    auto* fn_OrtGetApiBase = (void*)lib.resolve("OrtGetApiBase");
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
