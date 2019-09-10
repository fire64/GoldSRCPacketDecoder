//========= Copyright © 2010, fire64 LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================
#include "StdAfx.h"

#define MAXRECVSIZE 100000

#ifndef SOCKET
typedef unsigned int SOCKET;
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif


typedef enum ESocketProtocol
{
	eSocketProtocolUnknown = 0,
	eSocketProtocolTCP = 1,
	eSocketProtocolUDP = 2,
	eSocketProtocolIP = 3,

} ESocketProtocol;

class Csocket
{
public:
	Csocket( ESocketProtocol protocol );
	~Csocket();

	//Socket Function
	int BindPort( int port );
	unsigned long resolve(char *host);
	int SetAdr( sockaddr_in  new_addr );
	int SetAdr( char *ip, int port );
	int SetTimeOut( int settimeout );
	bool Connect( );
	bool IfUseTimeOut();
	bool Send( unsigned char *buff, int sizebuff = 0 );
	int Recv( unsigned char *recvbuffint, int recvlen = 0 );
	int AcceptConnect();
	bool ListenServer();
	ESocketProtocol GetCurentProtocol( );

//private:
	bool m_iftcp;
	bool m_usetimeout;
	timeval timeout;

	fd_set fd_read;
	fd_set fd_write;
	fd_set fd_error;

	sockaddr_in  dest_addr;
	SOCKET  current_sock;
};
