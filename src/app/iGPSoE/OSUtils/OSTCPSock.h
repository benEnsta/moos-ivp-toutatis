/***************************************************************************************************************:')

OSTCPSock.h

TCP sockets handling.
You must call InitNet() before using any network function.

Fabrice Le Bars

Created : 2006-2007

Version status : Not finished

***************************************************************************************************************:)*/

// Prevent Visual Studio Intellisense from defining _WIN32 and _MSC_VER when we use 
// Visual Studio to edit Linux or Borland C++ code.
#ifdef __linux__
#	undef _WIN32
#endif // __linux__
#if defined(__GNUC__) || defined(__BORLANDC__)
#	undef _MSC_VER
#endif // defined(__GNUC__) || defined(__BORLANDC__)

#ifndef OSTCPSOCK_H
#define OSTCPSOCK_H

#include "OSNet.h"

/*
Debug macros specific to OSTCPSock.
*/
#ifdef _DEBUG_MESSAGES_OSUTILS
#	define _DEBUG_MESSAGES_OSTCPSOCK
#endif // _DEBUG_MESSAGES_OSUTILS

#ifdef _DEBUG_WARNINGS_OSUTILS
#	define _DEBUG_WARNINGS_OSTCPSOCK
#endif // _DEBUG_WARNINGS_OSUTILS

#ifdef _DEBUG_ERRORS_OSUTILS
#	define _DEBUG_ERRORS_OSTCPSOCK
#endif // _DEBUG_ERRORS_OSUTILS

#ifdef _DEBUG_MESSAGES_OSTCPSOCK
#	define PRINT_DEBUG_MESSAGE_OSTCPSOCK(params) PRINT_DEBUG_MESSAGE(params)
#else
#	define PRINT_DEBUG_MESSAGE_OSTCPSOCK(params)
#endif // _DEBUG_MESSAGES_OSTCPSOCK

#ifdef _DEBUG_WARNINGS_OSTCPSOCK
#	define PRINT_DEBUG_WARNING_OSTCPSOCK(params) PRINT_DEBUG_WARNING(params)
#else
#	define PRINT_DEBUG_WARNING_OSTCPSOCK(params)
#endif // _DEBUG_WARNINGS_OSTCPSOCK

#ifdef _DEBUG_ERRORS_OSTCPSOCK
#	define PRINT_DEBUG_ERROR_OSTCPSOCK(params) PRINT_DEBUG_ERROR(params)
#else
#	define PRINT_DEBUG_ERROR_OSTCPSOCK(params)
#endif // _DEBUG_ERRORS_OSTCPSOCK

//#define DEFAULT_ADDR	"127.0.0.1"
//#define DEFAULT_PORT	"27254"

#ifdef _MSC_VER
// Disable some Visual Studio warnings that happen in some Winsock macros
// related to fd_set.
#pragma warning(disable : 4127) 
#endif // _MSC_VER

/*
Create a TCP socket at a given address and a given port.
Used at the beginning to create a server or a client application.
InitNet() must be called before using any network function.

SOCKET* pSocket : (OUT) pointer to the socket created
char* address : (IN) address used
char* port : (IN) port used
struct addrinfo** addrinf : (OUT) structure containing information about the connection used

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int CreateSocketTCP(SOCKET* pSocket, char* address, char* port, struct addrinfo** addrinf)
{
	struct addrinfo hints;
	int iResult;

	*addrinf = NULL;
	*pSocket = INVALID_SOCKET;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(address, port, &hints, addrinf);
	if (iResult != 0)
	{
		return EXIT_FAILURE;
	}

	// Create a socket.
	*pSocket = socket((*addrinf)->ai_family, (*addrinf)->ai_socktype, (*addrinf)->ai_protocol);
	if (*pSocket == INVALID_SOCKET)
	{
		freeaddrinfo(*addrinf);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

inline int DestroySocketTCP(SOCKET* pSocket, struct addrinfo** addrinf)
{
	freeaddrinfo(*addrinf);
	closesocket(*pSocket);

	return EXIT_SUCCESS;
}

/*
Disconnect a client by closing its socket.
Used by a server application when it has finished with a client.

SOCKET* pClientSocket : (INOUT) Pointer to the client socket.

Return : EXIT_SUCCESS or EXIT_FAILURE if there is an error.
*/
inline int DisconnectTCP(SOCKET* pSocket)
{
	closesocket(*pSocket);

	return EXIT_SUCCESS;
}

/*
Shutdowns the connection at a given socket.
Be careful because it should fail if the socket was not connected.

SOCKET *sock : (IN) pointer to the socket
int how : (IN) SD_RECEIVE if it stops receiving, SD_SEND if it stops sending, SD_BOTH for both

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int ShutdownTCP(SOCKET* pSocket, int how)
{
	int iResult;

	// shutdown the connection
	iResult = shutdown(*pSocket, how);
	if (iResult == SOCKET_ERROR)
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("ShutdownTCP error (%s) : %s"
			"(*pSocket=%#x, how=%d)\n", 
			strtime_m(), 
			"shutdown failed. ", 
			*pSocket, how));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Sets options for a given socket. Set reuseaddr to 1 if you don't want to wait 1 min before being able to reuse
the socket address after having closed it, or 0 if you want to be sure that there is no data from the last 
connection on this socket, and wait 1 min.

SOCKET sock : (IN) socket
BOOL bReuseaddr : (IN) FALSE to wait 1 min and be sure that there is no data 
from the last connection, TRUE otherwise
int timeout : (IN) timeout in milliseconds for send and recv

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int SetSockOptTCP(SOCKET sock, BOOL bReuseaddr, UINT timeout)
{
#ifdef _WIN32
#else
	struct timeval tv;
#endif
	int iOptVal = 0;
	int iOptLen = 0;

	iOptLen = sizeof(int);
	if (bReuseaddr)
	{
		iOptVal = 1; // TRUE
	}
	else	
	{
		iOptVal = 0; // FALSE
	}
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&iOptVal, iOptLen) == SOCKET_ERROR) {
		return EXIT_FAILURE;
	}

	// Setting timeouts for send and recv
#ifdef _WIN32
	iOptVal = (int)timeout; // In ms
	iOptLen = sizeof(int);
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&iOptVal, iOptLen) == SOCKET_ERROR) {
		PRINT_DEBUG_ERROR_OSTCPSOCK(("SetSockOptTCP error (%s) : %s"
			"(sock=%#x, bReuseaddr=%d, timeout=%u)\n", 
			strtime_m(), 
			"Cannot set timeout for send. ", 
			sock, (int)bReuseaddr, timeout));
		return EXIT_FAILURE;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iOptVal, iOptLen) == SOCKET_ERROR) {
		PRINT_DEBUG_ERROR_OSTCPSOCK(("SetSockOptTCP error (%s) : %s"
			"(sock=%#x, bReuseaddr=%d, timeout=%u)\n", 
			strtime_m(), 
			"Cannot set timeout for recv. ", 
			sock, (int)bReuseaddr, timeout));
		return EXIT_FAILURE;
	}
#else
	tv.tv_sec = (int)timeout/1000;
	tv.tv_usec = ((int)timeout%1000)*1000;
	iOptLen = sizeof(struct timeval);
	if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, iOptLen) == SOCKET_ERROR)
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("SetSockOptTCP error (%s) : %s"
			"(sock=%#x, bReuseaddr=%d, timeout=%u)\n", 
			strtime_m(), 
			"Cannot set timeout for send. ", 
			sock, (int)bReuseaddr, timeout));
		return EXIT_FAILURE;
	}
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, iOptLen) == SOCKET_ERROR)
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("SetSockOptTCP error (%s) : %s"
			"(sock=%#x, bReuseaddr=%d, timeout=%u)\n", 
			strtime_m(), 
			"Cannot set timeout for recv. ", 
			sock, (int)bReuseaddr, timeout));
		return EXIT_FAILURE;
	}
#endif // _WIN32

	return EXIT_SUCCESS;
}

/*
Bind a given socket.
Used by a server application to be able to accept incoming client connections.

SOCKET *ListenSocket : (IN) pointer to the server socket
struct addrinfo **addrinf : (IN,OUT) structure containing information about the connection used

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int BindTCP(SOCKET Socket, struct addrinfo **addrinf)
{
	int iResult;

	// Setup the listening socket
	iResult = bind(Socket, (*addrinf)->ai_addr, (int)((*addrinf)->ai_addrlen));
	if (iResult == SOCKET_ERROR)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Listen at a given socket.
Used by a server application to be able to accept incoming client connections.

SOCKET *ListenSocket : (IN) pointer to the server socket

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int ListenTCP(SOCKET Socket)
{
	int iResult;

	iResult = listen(Socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("ListenTCP error (%s) : %s"
			"(Socket=%#x)\n", 
			strtime_m(), 
			"listen failed. ", 
			Socket));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Binds and listens at a given socket.
Used by a server application to be able to accept incoming client connections.

SOCKET *ListenSocket : (IN) pointer to the server socket
struct addrinfo **addrinf : (IN,OUT) structure containing information about the connection used

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int BindListenTCP(SOCKET Socket, struct addrinfo** addrinf)	{

	int iResult;

	// Setup the listening socket
	iResult = bind(Socket, (*addrinf)->ai_addr, (int)((*addrinf)->ai_addrlen));
	if (iResult == SOCKET_ERROR)
	{
		return EXIT_FAILURE;
	}

	iResult = listen(Socket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Accepts a client connection by returning a client socket.

SOCKET *ListenSocket : (IN) pointer to the server socket
SOCKET *ClientSocket : (OUT) pointer to the client socket created

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int AcceptTCP(SOCKET ServerSocket, SOCKET* pClientSocket)
{
	// Accept a client socket
	*pClientSocket = accept(ServerSocket, NULL, NULL);
	if (*pClientSocket == INVALID_SOCKET)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Connects to a server.
Used by a client application to connect to a server.

SOCKET *ConnectSocket : (IN) pointer to a socket created by the client
struct addrinfo **addrinf : (IN,OUT) structure containing information about the connection used

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int ConnectTCP(SOCKET ClientSocket, struct addrinfo** addrinf)
{
	int iResult;

	// Connect to server.
	iResult = connect(ClientSocket, (*addrinf)->ai_addr, (int)(*addrinf)->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		//		fprintf(stderr, "Unable to connect to server!\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Sends data to a given socket.

SOCKET sock : (IN) socket
char *sendbuf : (IN) data to send
int sendbuflen : (IN) number of bytes to send

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int SendTCP(SOCKET sock, char* sendbuf, UINT sendbuflen)
{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	int i = 0;
#endif // _DEBUG_MESSAGES_OSTCPSOCK
	int Bytes = 0;

	Bytes = send(sock, sendbuf, sendbuflen, 0);
	if (Bytes >= 0)
	{
		if (Bytes == 0)
		{
			PRINT_DEBUG_ERROR_OSTCPSOCK(("SendTCP error (%s) : %s"
				"(sock=%#x, sendbuf=%#x, sendbuflen=%u)\n", 
				strtime_m(), 
				szOSUtilsErrMsgs[EXIT_TIMEOUT], 
				sock, sendbuf, sendbuflen));
			return EXIT_TIMEOUT;
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Bytes sent : %d\n", Bytes));
			for (i = 0; i < Bytes; i++)
			{
				//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (uint8)sendbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("SendTCP error (%s) : %s"
			"(sock=%#x, sendbuf=%#x, sendbuflen=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			sock, sendbuf, sendbuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Receives data at a given socket.

SOCKET sock : (IN) socket
char *recvbuf : (OUT) buffer which will contain the data received
int recvbuflen : (IN) number of bytes to receive

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int RecvTCP(SOCKET sock, char* recvbuf, UINT recvbuflen)
{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	int i = 0;
#endif // _DEBUG_MESSAGES_OSTCPSOCK
	int Bytes = 0;

	Bytes = recv(sock, recvbuf, recvbuflen, 0);
	if (Bytes >= 0)
	{
		if (Bytes == 0)
		{
			PRINT_DEBUG_ERROR_OSTCPSOCK(("RecvTCP error (%s) : %s"
				"(sock=%#x, recvbuf=%#x, recvbuflen=%u)\n", 
				strtime_m(), 
				szOSUtilsErrMsgs[EXIT_TIMEOUT], 
				sock, recvbuf, recvbuflen));
			return EXIT_TIMEOUT;
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Bytes received : %d\n", Bytes));
			for (i = 0; i < Bytes; i++)
			{
				//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (uint8)recvbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
		}
	}
	else
	{
		PRINT_DEBUG_ERROR_OSTCPSOCK(("RecvTCP error (%s) : %s"
			"(sock=%#x, recvbuf=%#x, recvbuflen=%u)\n", 
			strtime_m(), 
			GetLastErrorMsg(), 
			sock, recvbuf, recvbuflen));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/*
Sends data to a given socket. Retries automatically if all the bytes were not sent.
Fails when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) socket
char *sendbuf : (IN) data to send
int sendbuflen : (IN) number of bytes to send

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int SendAllTCP(SOCKET sock, char* sendbuf, UINT sendbuflen)
{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	UINT i = 0;
#endif // _DEBUG_MESSAGES_OSTCPSOCK
	UINT BytesSent = 0;
	int Bytes = 0;

	while (BytesSent < sendbuflen)
	{
		Bytes = send(sock, sendbuf + BytesSent, sendbuflen - BytesSent, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
				for (i = 0; i < BytesSent; i++)
				{
					//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)sendbuf[i]));
				}
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
				PRINT_DEBUG_ERROR_OSTCPSOCK(("SendAllTCP error (%s) : %s"
					"(sock=%#x, sendbuf=%#x, sendbuflen=%u)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					sock, sendbuf, sendbuflen));
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Total bytes sent : %u\n", BytesSent));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Bytes sent : %d\n", Bytes));
			}
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
			for (i = 0; i < BytesSent; i++)
			{
				//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)sendbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
			PRINT_DEBUG_ERROR_OSTCPSOCK(("SendAllTCP error (%s) : %s"
				"(sock=%#x, sendbuf=%#x, sendbuflen=%u)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				sock, sendbuf, sendbuflen));
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Total bytes sent : %u\n", BytesSent));
			return EXIT_FAILURE;
		}

		BytesSent += Bytes;
	}

#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	for (i = 0; i < BytesSent; i++)
	{
		//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)sendbuf[i]));
	}
	PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK

	return EXIT_SUCCESS;
}

/*
Receives data at a given socket. Retries automatically if all the bytes were not received.
Fails when a timeout occurs if it is enabled on the socket.

SOCKET sock : (IN) socket
char *recvbuf : (OUT) buffer which will contain the data received
int recvbuflen : (IN) number of bytes to receive

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int RecvAllTCP(SOCKET sock, char* recvbuf, UINT recvbuflen)
{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	UINT i = 0;
#endif // _DEBUG_MESSAGES_OSTCPSOCK
	UINT BytesReceived = 0;
	int Bytes = 0;

	while (BytesReceived < recvbuflen)
	{
		Bytes = recv(sock, recvbuf + BytesReceived, recvbuflen - BytesReceived, 0);
		if (Bytes >= 0)
		{
			if (Bytes == 0)
			{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
				for (i = 0; i < BytesReceived; i++)
				{
					//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)recvbuf[i]));
				}
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
				PRINT_DEBUG_ERROR_OSTCPSOCK(("RecvAllTCP error (%s) : %s"
					"(sock=%#x, recvbuf=%#x, recvbuflen=%u)\n", 
					strtime_m(), 
					szOSUtilsErrMsgs[EXIT_TIMEOUT], 
					sock, recvbuf, recvbuflen));
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Total bytes received : %u\n", BytesReceived));
				return EXIT_TIMEOUT;
			}
			else
			{
				PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Bytes received : %d\n", Bytes));
			}
		}
		else
		{
#ifdef _DEBUG_MESSAGES_OSTCPSOCK
			for (i = 0; i < BytesReceived; i++)
			{
				//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)recvbuf[i]));
			}
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK
			PRINT_DEBUG_ERROR_OSTCPSOCK(("RecvAllTCP error (%s) : %s"
				"(sock=%#x, recvbuf=%#x, recvbuflen=%u)\n", 
				strtime_m(), 
				GetLastErrorMsg(), 
				sock, recvbuf, recvbuflen));
			PRINT_DEBUG_MESSAGE_OSTCPSOCK(("Total bytes received : %u\n", BytesReceived));
			return EXIT_FAILURE;
		}

		BytesReceived += Bytes;
	}

#ifdef _DEBUG_MESSAGES_OSTCPSOCK
	for (i = 0; i < BytesReceived; i++)
	{
		//PRINT_DEBUG_MESSAGE_OSTCPSOCK(("%.2x ", (int)recvbuf[i]));
	}
	PRINT_DEBUG_MESSAGE_OSTCPSOCK(("\n"));
#endif // _DEBUG_MESSAGES_OSTCPSOCK

	return EXIT_SUCCESS;
}

/*
Waits for data to read on a given socket.

SOCKET sock : (IN) socket
timeval timeout : (IN) max time to wait before returning

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error or no data to read from the socket after the timeout
*/
inline int WaitForDataTCP(SOCKET sock, struct timeval timeout)
{
	fd_set sock_set;
	int iResult = SOCKET_ERROR;

	// Initialize a fd_set and add the socket to it
	FD_ZERO(&sock_set); 
	FD_SET(sock, &sock_set);

	// Waits for the readability of the socket in the fd_set, with a timeout
	iResult = select((int)sock+1, &sock_set, NULL, NULL, &timeout);

	// Removes the socket from the set
	FD_CLR(sock, &sock_set); 

	if (iResult == SOCKET_ERROR)
	{
		return EXIT_FAILURE;
	}

	if (iResult == 0)
	{ 
		// The timeout on select() occured
		return EXIT_TIMEOUT;
	}

	return EXIT_SUCCESS;
}

/*
Eliminates all the data coming from a given socket (if any).

SOCKET sock : (IN) socket

Returns : EXIT_SUCCESS or EXIT_FAILURE if there is an error
*/
inline int FlushTCP(SOCKET sock)
{
	fd_set sock_set;
	struct timeval timeout = {0, 0}; // Timeout of 0 s for select()
	int iResult = SOCKET_ERROR;
	char recvbuf[1024];

	// Initialize a fd_set and add the socket to it
	FD_ZERO(&sock_set); 
	FD_SET(sock, &sock_set);

	// Checks the readability of the socket in the fd_set
	iResult = select((int)sock+1, &sock_set, NULL, NULL, &timeout);

	while (iResult > 0)
	{
		// There is some data on the socket
		if (recv(sock, recvbuf, 1024*sizeof(char), 0) <= 0)
		{
			FD_CLR(sock, &sock_set); 
			return EXIT_FAILURE;
		}
		// Reset timeout (because a previous call to select may have modified it)
		timeout.tv_sec = 0;
		timeout.tv_usec = 0;
		// Checks the readability of the socket in the fd_set
		iResult = select((int)sock+1, &sock_set, NULL, NULL, &timeout);
	}

	// Removes the socket from the set
	FD_CLR(sock, &sock_set); 

	if (iResult == SOCKET_ERROR)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

#ifdef _MSC_VER
// Restore the Visual Studio warnings previously disabled for Winsock macros
// related to fd_set.
#pragma warning(default : 4127) 
#endif // _MSC_VER

#endif // OSTCPSOCK_H
