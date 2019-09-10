// GoldSRCPacketDecoder.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "tools.h"
#include "Csocket.h"
#include "DataParser.h"

int ConnectToServer( char *pIP, int port = 27015)
{
	Csocket *pSocket = new Csocket(eSocketProtocolUDP);
	pSocket->SetAdr( pIP, port );

	char pQueryPack[4096];
	memset(pQueryPack, 0, sizeof(pQueryPack) );

	unsigned char pRecvBuff[4096];
	memset(pRecvBuff, 0, sizeof(pRecvBuff));

	sprintf( pQueryPack, "\xFF\xFF\xFF\xFFgetchallenge\n" );
	pSocket->Send( (unsigned char *)pQueryPack, strlen(pQueryPack) + 1 );

	int recvbytes = pSocket->Recv(pRecvBuff, sizeof(pRecvBuff) );

	char pChallenge[512];
	memset(pChallenge, 0, sizeof(pChallenge));
	strcpy( pChallenge, (char *)pRecvBuff + 14 );

	for(size_t i = 0; i < strlen(pChallenge); i++ )
	{
		if(pChallenge[i] == ' ')
		{
			pChallenge[i] = NULL;
			break;
		}
	}

	memset(pQueryPack, 0, sizeof(pQueryPack) );
	sprintf( pQueryPack, "\xFF\xFF\xFF\xFF\connect 48 %s \"\\prot\\2\\unique\\-1\\raw\\861078331b85a424935805ca54f82891\" \"\\name\\HLTV Proxy\\cl_lw\\1\\cl_lc\\1\\*hltv\\1\\rate\\10000\\cl_updaterate\\20\\hspecs\\0\\hslots\\0\\hdelay\\30\"\n", pChallenge );
	pSocket->Send( (unsigned char *)pQueryPack, strlen(pQueryPack) + 1 );

	memset(pRecvBuff, 0, sizeof(pRecvBuff));
	recvbytes = pSocket->Recv(pRecvBuff, sizeof(pRecvBuff) );

	if( pRecvBuff[0] == 0xFF && pRecvBuff[1] == 0xFF && pRecvBuff[2] == 0xFF && pRecvBuff[3] == 0xFF && pRecvBuff[4] == 0x42 )
	{
		LogPrintf( false, "Good connect to server: %s:%d\n", pIP, port);
	}
	else if( pRecvBuff[0] == 0xFF && pRecvBuff[1] == 0xFF && pRecvBuff[2] == 0xFF && pRecvBuff[3] == 0xFF && pRecvBuff[4] == 0x39 )
	{
		LogPrintf( false, "Server %s:%d return error: %s\n", pIP, port, (char *)pRecvBuff + 5 );
	}
	else
	{
		LogPrintf( false, "Error connect to server: %s:%d\n", pIP, port );
	}

	return 1;
}

int main(int argc, char* argv[])
{
	ConnectToServer( "127.0.0.1", 27015 );

	return 1;
}

