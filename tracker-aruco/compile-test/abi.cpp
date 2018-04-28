#include "../../compat/linkage-macros.hpp"

#define ARUCO_ABI aruco_opentrack_fork_abi_1

extern "C" void ARUCO_ABI(void);

int main(void)
{
    ARUCO_ABI();
    return 0;
}
