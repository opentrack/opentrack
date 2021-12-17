#define OTR_GENERATE_SIGNAL3(type) void sig_##type::operator()(const type& x) const { notify(x); }
#include "qt-signal.hpp"
namespace _qt_sig_impl {

sig_void::sig_void(QObject* parent) : QObject(parent) {}

}
