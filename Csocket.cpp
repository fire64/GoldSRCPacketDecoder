//========= Copyright © 2010, fire64 LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================
#pragma warning(disable:4996)

#include "StdAfx.h"
#include "Csocket.h"
#include "tools.h"

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
Csocket::Csocket( ESocketProtocol protocol)
{

#ifdef _WIN32
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

	if( protocol == eSocketProtocolTCP )
	{
		m_iftcp = true;

		current_sock = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );

	}
	else if( protocol == eSocketProtocolUDP )
	{
		m_iftcp = false;

		current_sock = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); ;
	}
	else if( protocol == eSocketProtocolIP )
	{
		m_iftcp = true;

		current_sock = socket ( AF_INET, SOCK_STREAM, IPPROTO_IP );
	}

	m_usetimeout = false;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
Csocket::~Csocket()
{

#ifdef _WIN32
	closesocket( current_sock );

	WSACleanup();
#else
    close( current_sock );
#endif
}

#ifndef SO_REUSEPORT
#define SO_REUSEPORT    0x0200          /* leave received OOB data in line */
#endif

//-----------------------------------------------------------------------------
// Purpose: binding port
//-----------------------------------------------------------------------------
int Csocket::BindPort( int port )
{
	int res;

	bool reuse = true;

	setsockopt(current_sock, SOL_SOCKET, SO_REUSEPORT, (char*)&reuse, sizeof(reuse));

    dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons ( port );
	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef _WIN32
	WSASetLastError(0);
#endif

	res = bind (current_sock, (sockaddr *)&dest_addr, sizeof(dest_addr));

	if(res)
	{
#ifdef _WIN32
		int iret = WSAGetLastError();

		if( iret == WSANOTINITIALISED )
		{
			LogPrintf( true, "Can't bind port: %d: WSANOTINITIALISED\n", port );
		}
		else if( iret == WSAENETDOWN )
		{
			LogPrintf( true, "Can't bind port: %d: WSAENETDOWN\n", port );
		}
		else if( iret == WSAEADDRINUSE )
		{
			LogPrintf( true, "Can't bind port: %d: WSAEADDRINUSE\n", port );
		}
		else if( iret == WSAEADDRNOTAVAIL )
		{
			LogPrintf( true, "Can't bind port: %d: WSAEADDRNOTAVAIL\n", port );
		}
		else if( iret == WSAEFAULT )
		{
			LogPrintf( true, "Can't bind port: %d: WSAEFAULT\n", port );
		}
		else if( iret == WSAEINPROGRESS )
		{
			LogPrintf( true, "Can't bind port: %d: WSAEINPROGRESS\n", port );
		}
		else if( iret == WSAEINVAL )
		{
			LogPrintf( true, "Can't bind port: %d: WSAEINVAL\n", port );
		}
		else if( iret == WSAENOBUFS )
		{
			LogPrintf( true, "Can't bind port: %d: WSAENOBUFS\n", port );
		}
		else if( iret == WSAENOTSOCK )
		{
			LogPrintf( true, "Can't bind port: %d: WSAENOTSOCK\n", port );
		}
		else
		{
			LogPrintf( true, "Can't bind port: %d: %d\n", port, iret );
		}
#endif
	}

	return res;
}

//-----------------------------------------------------------------------------
// Purpose: get ip by host
//-----------------------------------------------------------------------------
unsigned long Csocket::resolve(char *host)
{
    unsigned long ret;
    struct hostent * hp = gethostbyname(host);
    if (!hp) ret = inet_addr(host);
    if ((!hp)&&(ret == INADDR_NONE)) return 0;
    if (hp != NULL) memcpy((void*)&ret, hp->h_addr,hp->h_length);
    return ret;
}

//-----------------------------------------------------------------------------
// Purpose: set adr for socket
//-----------------------------------------------------------------------------
int Csocket::SetAdr( sockaddr_in  new_addr )
{
	dest_addr = new_addr;

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: set adr for socket
//-----------------------------------------------------------------------------
int Csocket::SetAdr( char *ip, int port )
{
	dest_addr.sin_family=AF_INET;
    dest_addr.sin_port=htons ( port );
	dest_addr.sin_addr.s_addr = resolve(ip);

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: set adr for socket
//-----------------------------------------------------------------------------
bool Csocket::Connect( )
{
	if( IfUseTimeOut() )
	{
		// Estabilish a TCP connection
		connect(current_sock, (sockaddr*)&dest_addr, sizeof(dest_addr));

		FD_ZERO( &fd_write );
		FD_SET( current_sock, &fd_write );

		FD_ZERO( &fd_error );
		FD_SET( current_sock, &fd_error );

		if (select(0, 0, &fd_write, &fd_error, &timeout) > 0)
		{
			if ( FD_ISSET(current_sock, &fd_write) )
			{
				return true;
			}
		}

		return false;
	}
	else
	{
		if( connect(current_sock, (sockaddr*)&dest_addr, sizeof(dest_addr)) == -1 )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Send Packet
//-----------------------------------------------------------------------------
bool Csocket::Send( unsigned char *buff, int sizebuff )
{
	int bufflen;

	if( !sizebuff ) //Not god idea, buffer  may be contents 0 bytes
	{
		bufflen = strlen( (char *)buff) ;
	}
	else
	{
		bufflen = sizebuff;
	}

	if( GetCurentProtocol( ) == eSocketProtocolTCP )
	{
		if (send(current_sock, (char *)buff, bufflen, 0) == SOCKET_ERROR)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
	else
	{
		if (sendto(current_sock, (char *)buff, bufflen, 0, (sockaddr*)&dest_addr, sizeof(dest_addr) ) == SOCKET_ERROR)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Recv Packet
//-----------------------------------------------------------------------------
int Csocket::Recv( unsigned char *recvbuff, int recvlen )
{
	if(!recvlen)
	{
		recvlen = MAXRECVSIZE;
	}

	int recvbytes;

	if( IfUseTimeOut() )
	{
		FD_ZERO( &fd_read );
		FD_SET( current_sock, &fd_read );

		if( GetCurentProtocol( ) == eSocketProtocolTCP )
		{
			if( select(current_sock +1, &fd_read, NULL, NULL, &timeout) < 1 )
			{
				return 0;
			}
			else
			{
				recvbytes = recv(current_sock, (char *)recvbuff, recvlen, 0);
			}
		}
		else
		{
#ifdef _WIN32
			int server_addr_size = sizeof(dest_addr);
#else
			unsigned int server_addr_size = sizeof(dest_addr);
#endif

			if( select(current_sock +1, &fd_read, NULL, NULL, &timeout) < 1 )
			{
				return 0;
			}
			else
			{
				recvbytes = recvfrom(current_sock, (char *)recvbuff, recvlen, 0, (sockaddr*) &dest_addr, &server_addr_size);
			}
		}
	}
	else
	{
		if( GetCurentProtocol( ) == eSocketProtocolTCP )
		{
			recvbytes = recv(current_sock, (char *)recvbuff, recvlen, 0);
		}
		else
		{


#ifdef _WIN32
			int server_addr_size = sizeof(dest_addr);
#else
			unsigned int server_addr_size = sizeof(dest_addr);
#endif


			recvbytes = recvfrom(current_sock, (char *)recvbuff, recvlen, 0, (sockaddr*) &dest_addr, &server_addr_size);
		}
	}

	return recvbytes;
}

//-----------------------------------------------------------------------------
// Purpose: TCP Timeout
//-----------------------------------------------------------------------------
int Csocket::SetTimeOut( int settimeout )
{
	timeout.tv_sec = 0;
	timeout.tv_usec = settimeout;
	u_long nNonBlocking = 1;

	m_usetimeout = true;

	// Set socket to non-blocking mode
#ifdef _WIN32
	ioctlsocket(current_sock, FIONBIO, &nNonBlocking);
#else
	ioctl(current_sock, FIONBIO, &nNonBlocking);
#endif

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: If use timeout
//-----------------------------------------------------------------------------
bool Csocket::IfUseTimeOut( )
{
	return m_usetimeout;
}

//-----------------------------------------------------------------------------
// Purpose: Accept TCP Connect
//-----------------------------------------------------------------------------
int Csocket::AcceptConnect( )
{
#ifdef _WIN32
			int server_addr_size = sizeof(dest_addr);
#else
			unsigned int server_addr_size = sizeof(dest_addr);
#endif


	current_sock = accept( current_sock, (sockaddr*) &dest_addr, &server_addr_size );

	return 1;
}


//-----------------------------------------------------------------------------
// Purpose: Listen TCP Server
//-----------------------------------------------------------------------------
bool Csocket::ListenServer( )
{
    if(listen(current_sock,10)!=0)
    {
        return false;
    }

	return true;
}

ESocketProtocol Csocket::GetCurentProtocol( )
{
	ESocketProtocol protocol;

	if( m_iftcp )
	{
		protocol = eSocketProtocolTCP;
	}
	else
	{
		protocol = eSocketProtocolUDP;
	}

	return protocol;
}
