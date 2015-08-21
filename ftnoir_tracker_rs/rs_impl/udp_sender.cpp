/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs_impl.h"
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <commctrl.h>
#include <cstdlib>

#define UDP_PORT 4242

int main(){
    double pose[6] = { 0., 0., 0., 0., 0., 0. };
    struct sockaddr_in socketInfo;
    SOCKET s = 0;
    WSADATA wsa;

    int retStatus = rs_tracker_impl_start();
    if (retStatus != 0){
        exit(retStatus);
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) //init winsocketapi
        exit(EXIT_FAILURE);

    memset(&socketInfo, 0, sizeof(socketInfo));
    socketInfo.sin_family = AF_INET;
    socketInfo.sin_port = htons(UDP_PORT);
    InetPton(AF_INET, L"127.0.0.1", &socketInfo.sin_addr.S_un.S_addr);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR) //create UDP socket
        exit(EXIT_FAILURE);

    for(;;) {
        retStatus = rs_tracker_impl_update_pose(pose);
        if(retStatus == 0){ //no error
            if (sendto(s, (char*)&pose, sizeof(pose), 0, (struct sockaddr *) &socketInfo, sizeof(socketInfo)) == SOCKET_ERROR) //send new data
                exit(EXIT_FAILURE);
        }

        if(retStatus != 0 && retStatus != -303)// -303=timeout and 0 are ok, else we've got to stop.
            break;
    }

    closesocket(s);
    WSACleanup();

    return retStatus;
}
