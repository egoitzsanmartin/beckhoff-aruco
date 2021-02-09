#include "UDP_Server.h"

sockaddr_in init_UDP(SOCKET in) {

	WSADATA data;

	WORD version = MAKEWORD(2, 2);

	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		std::cout << "Can't start Winsock! " << wsOk;
	}

	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY; 
	serverHint.sin_family = AF_INET;
	serverHint.sin_port = htons(27277);
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		std::cout << "Can't bind socket! " << WSAGetLastError() << std::endl;
	} //Gives error for no reason

	sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_port = htons(27277);
	inet_pton(AF_INET, "192.168.102.1", &client.sin_addr);

	return client;
}

void getRobotPose(std::ofstream* allOutfile)
	{
		SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);
		sockaddr_in client;
		client = init_UDP(in);

		int clientLength = sizeof(client);
		char buf[1024];

		ZeroMemory(&client, clientLength); // Clear the client structure
		ZeroMemory(buf, 1024); // Clear the receive buffer

		// Wait for message
		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR)
		{
			std::cout << "Error receiving from client " << WSAGetLastError() << std::endl;
		}

		// Display message and client info
		char clientIp[256]; // Create enough space to convert the address byte array
		ZeroMemory(clientIp, 256); // to string of characters

		// Convert from byte array to chars
		inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);

		// Display the message / who sent it
		std::cout << "Message recv from " << clientIp << " : " << buf << std::endl;
		writeRobotPoseInFile(buf, allOutfile);

		closesocket(in);
}
