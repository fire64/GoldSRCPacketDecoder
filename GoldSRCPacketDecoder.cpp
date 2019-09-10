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

//First 8 bytes - header - sequence data and etc

//cl: Header 01 00 00 80 00 00 00 00 Data 2C 7D 6E 02 1A 01-11 41
//sv: Header 01 00 00 C0 01 00 00 80 Data 5A 18 05 00 1B 00-10 41 5B 52 40 05 49 02 -- many data, all in bz2 stream compressed

//cl: Header 02 00 00 00 01 00 00 80 Data 59 19 01 03 19 01-11 43
//cl: Header 03 00 00 00 01 00 00 80 Data 58 19 01 02 18 01-11 42

//sv: Header 02 00 00 C0 03 00 00 80 Data 5A 18 05 03 18 00-10 42 25 81 40 06 12 48 

//cl: Header 04 00 00 00 01 00 00 80 Data 5F 19 01 05 1F 01-11 45
//cl: Header 05 00 00 00 02 00 00 00 Data 5E 19 01 04 1E 01-11 44 
//cl: Header 06 00 00 00 02 00 00 00 Data 5D 19 01 07 1D 01-11 47

int main(int argc, char* argv[])
{
	ConnectToServer( "127.0.0.1", 27015 );

	return 1;
}

