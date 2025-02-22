
#include "RoboCatPCH.h"

#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <conio.h>

#if _WIN32

bool TCPServerSendMessages(TCPSocketPtr conn, SocketAddress address)
{
	char message[4096];
	char buffer[4096];

	int32_t bytesRead = conn->Receive(buffer, 4096);

	std::string msgReceived(buffer, bytesRead);
	LOG("Received message: %s", msgReceived.c_str());

	printf("%s", "Enter your message: ");
	scanf("%s", &message);

	if (kbhit() != 0) //this was supposed to try and display a user is typing message
	{
		std::string msg("a user is typing...");
		conn->Send(msg.c_str(), msg.length());
	}

	if (strcmp(message, "&") == 0)
	{
		std::string msg("the peer has disconnected");
		conn->Send(msg.c_str(), msg.length());
		return false;
	}

	conn->Send(message, 4096);

	LOG("%s", "Sent message to peer");	
	
	return true;	
}

bool TCPClientSendMessages(TCPSocketPtr connSocket, SocketAddress address)
{
	char message[4096];

	printf("%s", "Please enter a message to send:");

	scanf("%s", &message);

	if (kbhit() != 0) //this was supposed to try and display a user is typing message
	{
		std::string msg("a user is typing...");
		connSocket->Send(msg.c_str(), msg.length());
	}

	if (strcmp(message, "&") == 0)
	{
		std::string msg("the peer has disconnected");
		connSocket->Send(msg.c_str(), msg.length());
		return false;
	}

	connSocket->Send(message, 4096);

	LOG("%s", "Sent message to peer");

	char buffer[4096];
	int32_t bytesRead = connSocket->Receive(buffer, 4096);

	std::string msgReceived(buffer, bytesRead);
	LOG("Received message: %s", msgReceived.c_str());
	
	return true;
	
}

void DoTCPServer()
{
	// Open a TCP socket
	TCPSocketPtr listenSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);

	if (listenSocket == nullptr)
	{
		SocketUtil::ReportError("Creating listen socket");
		ExitProcess(1);
	}

	LOG("%s", "Created listen socket");

	//SocketAddress a2(INADDR_LOOPBACK, 8080);
	// Listen only for connections on this machine
	//SocketAddressPtr a = SocketAddressFactory::CreateIPv4FromString("0.0.0.0:8080");
	SocketAddress a(INADDR_LOOPBACK, 8080);

	if (&a == nullptr)
	{
		SocketUtil::ReportError("Creating server address");
		ExitProcess(1);
	}

	LOG("%s", "Create socket address");

	// "Bind" that socket to the address we want to listen on
	if (listenSocket->Bind(a) != NO_ERROR)
	{
		SocketUtil::ReportError("Binding socket");
		ExitProcess(1);
	}

	LOG("%s", "Bound socket");

	// Call "Listen" to have the OS listen for connections on that socket
	if (listenSocket->Listen() != NO_ERROR)
	{
		SocketUtil::ReportError("Listening");
		ExitProcess(1);
	}

	LOG("%s", "Socket listening");

	// Call "Accept" which *blocks* until we get a request to connect,
	// and then it accepts that connection!


	/// listenSocket <----- packets from other hosts wanting to connect
	/// listenSocket -----> accepts connection, spawns:
	///   connectionSocket <----> otherHost1
	///   connectionSock2  <----> otherHost2

	SocketAddress connAddr(INADDR_LOOPBACK, 8081);
	TCPSocketPtr conn;

	// This code isn't blocking anymore -- it'll run to the end of the program.
	//while (conn != nullptr)
	{
		conn = listenSocket->Accept(connAddr);
	}

	bool messageOnGoing = true;
	messageOnGoing = TCPServerSendMessages(conn, connAddr);

	while (messageOnGoing == true)
	{
		messageOnGoing = TCPServerSendMessages(conn, connAddr);
	}
}

void DoTCPClient()
{
	//std::thread t(DoTCPServer);
	//t.join();
	 
	// Open a TCP socket
	TCPSocketPtr connSocket = SocketUtil::CreateTCPSocket(SocketAddressFamily::INET);

	if (connSocket == nullptr)
	{
		SocketUtil::ReportError("Creating listen socket");
		ExitProcess(1);
	}

	LOG("%s", "Created client socket");

	//SocketAddress a2(INADDR_LOOPBACK, 8080);
	// Listen only for connections on this machine
	//SocketAddressPtr a = SocketAddressFactory::CreateIPv4FromString(INADDR_ANY);
	SocketAddress a(INADDR_LOOPBACK, 8081);

	if (&a == nullptr)
	{
		SocketUtil::ReportError("Creating server address");
		ExitProcess(1);
	}

	LOG("%s", "Create socket address");

	// "Bind" that socket to the address we want to listen on
	if (connSocket->Bind(a) != NO_ERROR)
	{
		SocketUtil::ReportError("Binding socket");
		ExitProcess(1);
	}

	LOG("%s", "Bound socket");

	//SocketAddressPtr servAddress = SocketAddressFactory::CreateIPv4FromString(INADDR_ANY);
	SocketAddress servAddress(INADDR_LOOPBACK, 8080);

	if (&servAddress == nullptr)
	{
		SocketUtil::ReportError("Creating server address");
		ExitProcess(1);
	}

	if (connSocket->Connect(servAddress) != NO_ERROR)
	{
		SocketUtil::ReportError("Connecting to server");
		ExitProcess(1);
	}

	LOG("%s", "Connected to server!");
	std::string msg("someone has connected");
	connSocket->Send(msg.c_str(), msg.length());

	bool messageOnGoing = true;
	messageOnGoing = TCPClientSendMessages(connSocket, servAddress);

	
	while (messageOnGoing == true)
	{
		messageOnGoing = TCPClientSendMessages(connSocket, servAddress);
	}
}

int main(int argc, const char** argv)
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
#else
const char** __argv;
int __argc;
int main(int argc, const char** argv)
{
	__argc = argc;
	__argv = argv;
#endif

	SocketUtil::StaticInit();

	OutputWindow win;
	std::thread t;
	std::thread t2;

	bool isServer = StringUtils::GetCommandLineArg(1) == "server";

	if (isServer)
	{
		DoTCPServer();
	}

	if (!isServer)
	{
		DoTCPClient();
	}
	

	//std::thread t([&win]()
	//			  {
	//				  int msgNo = 1;
	//				  /*while (true)
	//				  {
	//					  std::this_thread::sleep_for(std::chrono::milliseconds(250));
	//					  std::string msgIn("~~~auto message~~~");
	//					  std::stringstream ss(msgIn);
	//					  ss << msgNo;
	//					  win.Write(ss.str());
	//					  msgNo++;
	//				  }*/
	//				  
	//				  
	//			  });

	//while (true)
	//{
	//	std::string input;
	//	std::getline(std::cin, input);
	//	win.WriteFromStdin(input);
	//}

	SocketUtil::CleanUp();

	return 0;
}
