#include <aruco/abi.h>

#if ARUCO_OPENTRACK_FORK_ABI != 2
#   error "wrong ABI"
#endif

int main(void) { return 0; }
