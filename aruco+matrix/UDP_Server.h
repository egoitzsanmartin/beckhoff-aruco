#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

sockaddr_in init_UDP(SOCKET in);
void getRobotPose(std::ofstream* allOutfile);