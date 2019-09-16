// GoldSRCPacketDecoder.cpp:
//

#include "stdafx.h"
#include "tools.h"
#include "Csocket.h"
#include "DataParser.h"
#include "bzlib.h"

struct serverinfo_t
{
	int  serverport;
	char pServAddr[1024];

	byte protocol;
	char pHostName[1024];
	char pMapName[1024];
	char pGameDir[1024];
	char pGameName[1024];
	short appid;

	byte curpl;
	byte maxplayers;
	byte bots;
	byte type;
	byte os;
	byte password;

	//Mod info for old servers
	byte ismod;
	char pLinkMod[1024];
	char pDownloadLink[1024];
	long modversion;
	long modsize;
	byte typemod;
	byte isuseowndll;

	byte secure;

	char pVersion[1024];
	short gameport;
	long long steamid;
	short sourcetvport;
	char pSourceTVName[1024];
	char tags[1024];
	long long gameid;
};

//First 8 bytes - header - sequence data and etc

//cl: Header 01 00 00 80 - 00 00 00 00 Data 2C 7D 6E 02 1A 01-11 41
//sv: Header 01 00 00 C0 - 01 00 00 80 Data 5A 18 05 00 1B 00-10 41 5B 52 40 05 49 02 -- many data, all in bz2 stream compressed

//cl: Header 02 00 00 00 - 01 00 00 80 Data 59 19 01 03 19 01-11 43
//cl: Header 03 00 00 00 - 01 00 00 80 Data 58 19 01 02 18 01-11 42

//sv: Header 02 00 00 C0 - 03 00 00 80 Data 5A 18 05 03 18 00-10 42 25 81 40 06 12 48 

//cl: Header 04 00 00 00 - 01 00 00 80 Data 5F 19 01 05 1F 01-11 45
//cl: Header 05 00 00 00 - 02 00 00 00 Data 5E 19 01 04 1E 01-11 44 
//cl: Header 06 00 00 00 - 02 00 00 00 Data 5D 19 01 07 1D 01-11 47

int DecompressData( CDataParser *pCompressedData )
{
	char uncompressed[65536];
	unsigned int uncompressedSize = 65536;

	int ret = BZ2_bzBuffToBuffDecompress(uncompressed, &uncompressedSize, (char *)pCompressedData->GetCurrentData(), pCompressedData->GetCurrentSize(), 1, 0);

	if( ret != BZ_OK )
	{
		LogPrintf( false, "Error decompress: %d\n", ret );
	}


	filedata_t pTest;
	pTest.filebuf = (unsigned char *)uncompressed;
	pTest.filelen = uncompressedSize;

	FileReWrite( "Unzip.bin", pTest );

	return 1;
}

CDataParser *DecodeFunc(unsigned char *data, int size)
{
	CDataParser *pDataParser = new CDataParser(data, size);

	unsigned int sequence;
	unsigned int sequence_ack;

	unsigned int reliable_ack;
	unsigned int reliable_message;

	bool message_contains_fragments;

	// get sequence numbers
	sequence = pDataParser->GetLong();
	sequence_ack = pDataParser->GetLong();

	COM_UnMunge2(pDataParser->GetCurrentData(), pDataParser->GetCurrentSize(), sequence & 0xFF);

	//Later 
	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;
	message_contains_fragments = sequence & (1 << 30) ? true : false;

	return pDataParser;
}

void WriteDecodePack( unsigned char *pDecodeData, int decodedatasize, int packid )
{
	filedata_t pDecFileBuff;
	pDecFileBuff.filebuf = pDecodeData;
	pDecFileBuff.filelen = decodedatasize;

	char pDecodefileName[512];
	memset( pDecodefileName, 0, sizeof(pDecodefileName) );
	sprintf( pDecodefileName, "pack_%d.bin", packid );

	int ret = FileWrite( pDecodefileName, pDecFileBuff );

	if(ret)
	{
		LogPrintf(false, "File %s successfully decoded\n", pDecodefileName );
	}
	else
	{
		LogPrintf(false, "File %s can't be writed\n", pDecodefileName );
	}
}

//Clients commands
//	clc_bad = 0, //0 - only clc_bad byte - error opcode // immediately drop client when received
//	clc_nop, //1 - only clc_nop byte - nop opcode
//	clc_move, //2 // [[usercmd_t]
//	clc_stringcmd, //3 - send clc_stringcmd byte + string command + terminal 0
//	clc_delta, //4 // [byte] sequence number, requests delta compression of message
//	clc_resourcelist, //5 string filename + terminal 0 + byte type + short index + long download size + byte flag
//	clc_tmove, //6
//	clc_fileconsistency, //7
//	clc_voicedata, //8
//	clc_hltv, //9
//	clc_cvarvalue, //10
//	clc_cvarvalue2, //11
//	clc_endoflist = 255, //12

void SendDataPack( Csocket *pSocket, CDataParser *pEncodeQuery, int sequence, int sequence_ack)
{
	COM_Munge2(pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset(), sequence & 0xFF);

	//Data parsers
	CDataParser *pQueryPack = new CDataParser( 8192 );

	//Reset query data
	pQueryPack->ClearAllBuf();
	pQueryPack->SetOffset(0);

	//Add header and data to pack
	pQueryPack->SetLong( sequence );
	pQueryPack->SetLong( sequence_ack );
	pQueryPack->SetData( pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset() );

	pSocket->Send( pQueryPack->GetFullData(), pQueryPack->GetOffset() );

	delete pQueryPack;
}

int RecvDataPack( Csocket *pSocket, CDataParser *pRevData )
{
	unsigned int recv_sequence;
	unsigned int sequence_ack;

	//Reset recv buff data
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	//Recv and decode pack from server
	int recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

	if(recvbytes > 8 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Decode Data
		COM_UnMunge2(pRevData->GetCurrentData(), pRevData->GetCurrentSize(), recv_sequence & 0xFF);

		pRevData->SetOffset(0);
	}

	return recvbytes;
}

int SendEmptyNopAndGetData( Csocket *pSocket, unsigned int sequence, unsigned int sequence_ack, CDataParser *pRevData )
{
	CDataParser *pEncodeQuery = new CDataParser( 8192 );

	//Reset encode data
	pEncodeQuery->ClearAllBuf();
	pEncodeQuery->SetOffset(0);

	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );

	//Send
	SendDataPack( pSocket, pEncodeQuery, sequence, sequence_ack);

	int recvbytes = RecvDataPack( pSocket, pRevData );

	return recvbytes;
}


int StartCommunicationWithServer( char *pIP, int port, Csocket *pSocket, char *pChallenge)
{
	unsigned int send_sequence = 0;
	unsigned int recv_sequence = 0;
	unsigned int sequence_ack = 0;

	int recvbytes = 0;
	unsigned char *pDecodeData = NULL;
	int decodedatasize = 0;
	int packid = 0;

	//First pack
	send_sequence = 0x80000001;
	sequence_ack = 0; //later get from server

	CDataParser *pEncodeQuery = new CDataParser( 8192 );
	CDataParser *pRevData = new CDataParser( 8192 );

	//Reset encode data
	pEncodeQuery->ClearAllBuf();
	pEncodeQuery->SetOffset(0);

	//First send new - for new players
	pEncodeQuery->SetByte( clc_stringcmd );
	pEncodeQuery->SetString( "new" );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );

	//Send
	SendDataPack( pSocket, pEncodeQuery, send_sequence, sequence_ack);

	//Recv and decode pack from server
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	recvbytes = RecvDataPack( pSocket, pRevData );

	if(recvbytes > 8 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Decode Data
		pDecodeData = pRevData->GetCurrentData();
		decodedatasize = recvbytes - pRevData->GetOffset();

		//Write decode data to file, for manual anais
		packid++;
		WriteDecodePack( pRevData->GetFullData(), pRevData->GetFullSize(), packid );
	}

	//reset
	send_sequence = 1;

	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);
	send_sequence++;

	recvbytes = SendEmptyNopAndGetData( pSocket, send_sequence, sequence_ack, pRevData );

	if(recvbytes > 8 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Decode Data
		pDecodeData = pRevData->GetCurrentData();
		decodedatasize = recvbytes - pRevData->GetOffset();

		//Write decode data to file, for manual anais
		packid++;
		WriteDecodePack( pRevData->GetFullData(), pRevData->GetFullSize(), packid );
	}

	//PART2 - Spawn
	Sleep(100);

	//ENCODE AND SEND DATA
	send_sequence++;

	unsigned int w1, w2;
	w1 = send_sequence | (0<<30) | (0<<31);
	w2 = 0 | (0<<31);

	//Reset encode data
	pEncodeQuery->ClearAllBuf();
	pEncodeQuery->SetOffset(0);


	//First send new - for new players
	pEncodeQuery->SetByte( clc_resourcelist );
	pEncodeQuery->SetByte( 0 );
	pEncodeQuery->SetByte( 0 );
	pEncodeQuery->SetByte( clc_stringcmd );

	char pCmd[512];
	memset( pCmd, 0, sizeof(pCmd) );
	sprintf( pCmd, "spawn 1 %s", pChallenge);
	pEncodeQuery->SetString( pCmd );

	//Send
	SendDataPack( pSocket, pEncodeQuery, send_sequence, sequence_ack); //I don't know why it doesn't work otherwise

	//Reset recv buff data
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	//Recv and decode pack from server
	recvbytes = RecvDataPack( pSocket, pRevData );

	if(recvbytes > 8 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Write decode data to file, for manual anais
		packid++;
		WriteDecodePack( pRevData->GetFullData(), pRevData->GetFullSize(), packid );
	}

	int messageid = 0;

	//PART3 - Chat message
	while(true)
	{
		Sleep(1000);
		//ENCODE AND SEND DATA
		send_sequence++;
		messageid++;

		w1 = send_sequence | (0<<30) | (0<<31);
		w2 = 0 | (0<<31);

		//Reset encode data
		pEncodeQuery->ClearAllBuf();
		pEncodeQuery->SetOffset(0);

		char pMessage[1024];
		memset( pMessage, 0, sizeof(pMessage) );
		sprintf( pMessage, "say Hey. I am a bot. Hello to you from Fire64. This is message number %d", messageid );

		//Send chat message
		pEncodeQuery->SetByte( clc_nop );
		pEncodeQuery->SetByte( clc_stringcmd );
		pEncodeQuery->SetString( pMessage );
		pEncodeQuery->SetByte( clc_nop );

		SendDataPack( pSocket, pEncodeQuery, w1, w2);

		//Reset recv buff data
		pRevData->ClearAllBuf();
		pRevData->SetOffset(0);

		//Recv and decode pack from server
		recvbytes = RecvDataPack( pSocket, pRevData );

		if(recvbytes > 8 )
		{
			// get sequence numbers
			recv_sequence = pRevData->GetLong();
			sequence_ack = pRevData->GetLong();

			packid++;
			WriteDecodePack( pRevData->GetFullData(), pRevData->GetFullSize(), packid );
		}
	}

	return 1;
}

int ConnectToServer( char *pIP, int port = 27015)
{
	Csocket *pSocket = new Csocket(eSocketProtocolUDP);
	pSocket->SetAdr( pIP, port );
	pSocket->SetTimeOut( 500000 );

	char pQueryPack[8192];
	memset(pQueryPack, 0, sizeof(pQueryPack) );

	unsigned char pRecvBuff[8192];
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
	sprintf( pQueryPack, "\xFF\xFF\xFF\xFF\connect 48 %s \"\\prot\\2\\unique\\-1\\raw\\861078331b85a424935805ca54f82891\" \"\\name\\Chat bot\\cl_lw\\1\\cl_lc\\1\\*hltv\\1\\rate\\10000\\cl_updaterate\\20\\hspecs\\0\\hslots\\0\\hdelay\\30\"\n", pChallenge );
	pSocket->Send( (unsigned char *)pQueryPack, strlen(pQueryPack) + 1 );

	memset(pRecvBuff, 0, sizeof(pRecvBuff));
	recvbytes = pSocket->Recv(pRecvBuff, sizeof(pRecvBuff) );

	if( pRecvBuff[0] == 0xFF && pRecvBuff[1] == 0xFF && pRecvBuff[2] == 0xFF && pRecvBuff[3] == 0xFF && pRecvBuff[4] == 0x42 )
	{
		LogPrintf( false, "Good connect to server: %s:%d\n", pIP, port);
		StartCommunicationWithServer( pIP, port, pSocket, pChallenge);
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

int DecodePack( char *pEncodeFileName, char *pDecodefileName )
{
	filedata_t pFileEnc = FileRead(pEncodeFileName);

	if(!pFileEnc.filelen)
	{
		LogPrintf( false, "Can't open file: %s\n", pEncodeFileName );
		return 0;
	}

	CDataParser *pDecBuff = DecodeFunc(pFileEnc.filebuf, pFileEnc.filelen);

	filedata_t pDecFileBuff;
	pDecFileBuff.filebuf = pDecBuff->GetCurrentData();
	pDecFileBuff.filelen = pDecBuff->GetCurrentSize();

	int ret = FileWrite( pDecodefileName, pDecFileBuff );

	if(ret)
	{
		LogPrintf(false, "File %s successfully decoded\n", pDecodefileName );
	}
	else
	{
		LogPrintf(false, "File %s can't be writed\n", pDecodefileName );
	}

	delete pFileEnc.filebuf;

	return 1;
}

void TestDecodePackets()
{
	//Test decode packets
	DecodePack( "pack/encode/1cl.bin", "pack/decode/1cl-dec.bin" ); //Client send to server: clc_stringcmd byte + string "new" with terminal 0 + clc_nop + clc_nop + clc_nop
	DecodePack( "pack/encode/2sv.bin", "pack/decode/2sv-dec.bin" ); //server send unk code + size data in bytes + SVC_STUFFTEXT byte + string cl_forwardspeed 320 cl_backspeed 320 cl_sidespeed 320
	DecodePack( "pack/encode/3cl.bin", "pack/decode/3cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/4sv.bin", "pack/decode/4sv-dec.bin" ); //Server send BZ2 compressed data
	DecodePack( "pack/encode/5cl.bin", "pack/decode/5cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/6cl.bin", "pack/decode/6cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/7sv.bin", "pack/decode/7sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/8cl.bin", "pack/decode/8cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/9cl.bin", "pack/decode/9cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/10cl.bin", "pack/decode/10cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/11sv.bin", "pack/decode/11sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/12cl.bin", "pack/decode/12cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/13cl.bin", "pack/decode/13cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/14sv.bin", "pack/decode/14sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/15cl.bin", "pack/decode/15cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/16cl.bin", "pack/decode/16cl-dec.bin" ); //client send clc_stringcmd + string "sendres" with terminal 0 
	DecodePack( "pack/encode/17cl.bin", "pack/decode/17cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/18sv.bin", "pack/decode/18sv-dec.bin" ); //Server send BZ2 compressed data
	DecodePack( "pack/encode/19cl.bin", "pack/decode/19cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/20cl.bin", "pack/decode/20cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/21sv.bin", "pack/decode/21sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/22cl.bin", "pack/decode/22cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/23cl.bin", "pack/decode/23cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/24cl.bin", "pack/decode/24cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/25sv.bin", "pack/decode/25sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/26cl.bin", "pack/decode/26cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/27cl.bin", "pack/decode/27cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/28sv.bin", "pack/decode/28sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/29cl.bin", "pack/decode/29cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/30cl.bin", "pack/decode/30cl-dec.bin" ); //client send clc_resourcelist byte + 0 + 0 + clc_stringcmd + "spawn 1 1518259236" with terminal 0 (may be 1518259236 its challenge??? )
	DecodePack( "pack/encode/31sv.bin", "pack/decode/31sv-dec.bin" ); //Server send BZ2 compressed data
	DecodePack( "pack/encode/32cl.bin", "pack/decode/32cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/33cl.bin", "pack/decode/33cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/34sv.bin", "pack/decode/34sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/35cl.bin", "pack/decode/35cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/36cl.bin", "pack/decode/36cl-dec.bin" ); //Error less dats
	DecodePack( "pack/encode/37cl.bin", "pack/decode/37cl-dec.bin" ); //client send clc_stringcmd + string "sendents " with terminal 0 + clc_stringcmd + string "spectate" with terminal 0 + clc_stringcmd + string "VModEnable 1" with terminal 0 + clc_stringcmd + string "vban 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0" with terminal 0
	DecodePack( "pack/encode/38cl.bin", "pack/decode/38cl-dec.bin" ); //client send many count clc_nop
	DecodePack( "pack/encode/39sv.bin", "pack/decode/39sv-dec.bin" ); //Unk command with string, not compressed: #Game_connected HLTV Proxy \cl_lw\1\cl_lc\1\*hltv\1\rate\10000\cl_updaterate\20\hspecs\0\hslots\0\hdelay\30\name\HLTV Proxy\*sid\90071996842377216\model\male\topcolor\0\bottomcolor\-10 #Spec_Mode
	DecodePack( "pack/encode/40sv.bin", "pack/decode/40sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
	DecodePack( "pack/encode/41sv.bin", "pack/decode/41sv-dec.bin" ); //server send unk data, may be it is part BZ2 stream
}

int SendInfoAboutServer( Csocket *pSocket, serverinfo_t *pServerInfo )
{
	CDataParser *pQuery = new CDataParser( 8192 );

	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0x49 );

	pQuery->SetByte( pServerInfo->protocol );
	pQuery->SetString( pServerInfo->pHostName );
	pQuery->SetString( pServerInfo->pMapName );
	pQuery->SetString( pServerInfo->pGameDir );
	pQuery->SetString( pServerInfo->pGameName );

	pQuery->SetShort( pServerInfo->appid );
	pQuery->SetByte( pServerInfo->curpl );
	pQuery->SetByte( pServerInfo->maxplayers );
	pQuery->SetByte( pServerInfo->bots );

	pQuery->SetByte( pServerInfo->type );
	pQuery->SetByte( pServerInfo->os );
	pQuery->SetByte( pServerInfo->password );
	pQuery->SetByte( pServerInfo->secure );

	pQuery->SetString( pServerInfo->pVersion );

	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	delete pQuery;

	return 1;
}

int SendChallengeForSteam( Csocket *pSocket )
{
	CDataParser *pQuery = new CDataParser( 8192 );

	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0x41 );
	pQuery->SetString( "00000000 466639471 3 90127494217252875m 1" );
	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	delete pQuery;

	return 1;
}

int SendChallengeForValve( Csocket *pSocket )
{
	return 1;
}

int SendConnectionApproval( Csocket *pSocket )
{
	CDataParser *pQuery = new CDataParser( 8192 );

	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0x42 );
	pQuery->SetString( " 984 \"128.72.231.11:27005\" 1 7882" );

	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	delete pQuery;

	return 1;
}

int SendPlayersListChallenge( Csocket *pSocket )
{
	CDataParser *pQuery = new CDataParser( 8192 );

	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0x41 );
	pQuery->SetLong( 123456 );

	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	delete pQuery;

	return 1;
}

int SendPlayersList( Csocket *pSocket, serverinfo_t *pServerInfo )
{
	CDataParser *pQuery = new CDataParser( 8192 );

	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0xFF );
	pQuery->SetByte( 0x44 );

	pQuery->SetByte( pServerInfo->curpl );

	for( int i = 0; i < pServerInfo->curpl; i++ )
	{
		char pPlayerName[256];
		memset( pPlayerName, 0, sizeof(pPlayerName) );
		sprintf( pPlayerName, "player %d", i );

		pQuery->SetByte( i ); //player id
		pQuery->SetString( pPlayerName ); //player name
		pQuery->SetLong( i * 2 ); //score
		pQuery->SetFloat( 60.0f * i ); //score
	}

	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	delete pQuery;

	return 1;
}

int AddServerToMasterServer( Csocket *pSocket, serverinfo_t *pServerInfo )
{
	pSocket->SetAdr( "hl2master.steampowered.com", 27011 );

	CDataParser *pQuery = new CDataParser( 8192 );
	CDataParser *pRevData = new CDataParser( 8192 );

	//Reset send data
	pQuery->ClearAllBuf();
	pQuery->SetOffset(0);

	pQuery->SetByte( 0x71 );
	pSocket->Send( pQuery->GetFullData(), pQuery->GetOffset() );

	//Reset recv data
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	int recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

	if( recvbytes > 6 && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0x73 && pRevData->GetByte() == 0x0A )
	{
		long chalenge = pRevData->GetLong();

		char pMasterQuery[1024];
		memset(pMasterQuery, 0, sizeof(pMasterQuery) );
		sprintf( pMasterQuery, "0\n\\protocol\\%d\\challenge\\%ld\\players\\%d\\max\\%d\\bots\\%d\\gamedir\\%s\\map\\%s\\password\\%d\\os\\%c\\lan\\0\\region\\255\\gameport\\%d\\specport\\0\\dedicated\\1\\appid\\%d\\type\\%c\\secure\\%d\\version\\%s\\product\\%s\n", pServerInfo->protocol, chalenge, pServerInfo->curpl, pServerInfo->maxplayers, pServerInfo->bots, pServerInfo->pGameDir, pServerInfo->pMapName, pServerInfo->password, pServerInfo->os, pServerInfo->gameport, pServerInfo->appid, pServerInfo->type, pServerInfo->secure, pServerInfo->pVersion, pServerInfo->pGameDir );

		pSocket->Send( (unsigned char *)pMasterQuery, strlen(pMasterQuery) );

		//Reset recv data
		pRevData->ClearAllBuf();
		pRevData->SetOffset(0);

		int recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

		delete pQuery;
		delete pRevData;
		return 1;
	}
	else
	{
		delete pQuery;
		delete pRevData;
		return 0;
	}

	return 1;
}

int StartCommunicationWithClient( Csocket *pSocket, CDataParser *pRevData, serverinfo_t *pServerInfo, int recvbytes )
{
	unsigned int send_sequence = 0;
	unsigned int recv_sequence = 0;
	unsigned int sequence_ack = 0;
	
	unsigned int w1, w2;

	CDataParser *pEncodeQuery = new CDataParser( 8192 );

	int packid = 0;

	if(recvbytes > 7 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Decode Data
		COM_UnMunge2(pRevData->GetCurrentData(), pRevData->GetCurrentSize(), recv_sequence & 0xFF);

		pRevData->SetOffset(0);
	}
	else
	{
		return 0;
	}

	//ENCODE AND SEND DATA
	send_sequence++;
	w1 = send_sequence | (0<<30) | (0<<31);
	w2 = 0 | (0<<31);

	//Reset encode data
	pEncodeQuery->ClearAllBuf();
	pEncodeQuery->SetOffset(0);

	//Send protocol version
	pEncodeQuery->SetByte( svc_version );
	pEncodeQuery->SetLong( pServerInfo->protocol );

	pEncodeQuery->SetByte( svc_time );
	pEncodeQuery->SetFloat( 1 );

	//Send message to console
	pEncodeQuery->SetByte( svc_print );
	pEncodeQuery->SetString( "I'm not server :)\n" );

	pEncodeQuery->SetByte( svc_signonnum );
	pEncodeQuery->SetByte( 1 );

	//Send command to client
	pEncodeQuery->SetByte( svc_stufftext );
	pEncodeQuery->SetString( "echo test\n" );

/*
	//Message error
	pEncodeQuery->SetByte( svc_bad );
*/

	//Send disconnect command
	pEncodeQuery->SetByte( svc_disconnect );
	pEncodeQuery->SetString( "I'm not server :)\n" );

	//Send
	SendDataPack( pSocket, pEncodeQuery, send_sequence, sequence_ack); //I don't know why it doesn't work otherwise

	//Reset recv buff data
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	//Recv and decode pack from server
	recvbytes = RecvDataPack( pSocket, pRevData );

	if(recvbytes > 8 )
	{
		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();
	}

	return 1;
}

int EmulationServer( int port )
{
	Csocket *pSocket = new Csocket(eSocketProtocolUDP);
	pSocket->BindPort( port );
	pSocket->SetTimeOut( 500000 );

	serverinfo_t pServerInfo;
	memset( &pServerInfo, 0, sizeof(pServerInfo) );

	pServerInfo.serverport = port;
	pServerInfo.protocol = 48;

	strcpy( pServerInfo.pHostName, "Ricochet CTF Server" );
	strcpy( pServerInfo.pMapName, "rc_ctfdm_m2" );
	strcpy( pServerInfo.pGameDir, "ricochet" );
	strcpy( pServerInfo.pGameName, "Ricochet CTF" );
	pServerInfo.appid = 60;

	pServerInfo.curpl = 3;
	pServerInfo.maxplayers = 12;
	pServerInfo.bots = 0;
	pServerInfo.type = 'd';
	pServerInfo.os = 'l';
	pServerInfo.password = 0;
	pServerInfo.ismod = 0;
	pServerInfo.secure = 0;

	strcpy( pServerInfo.pVersion, "1.1.2.1" );

	pServerInfo.gameport = port;

	AddServerToMasterServer(pSocket, &pServerInfo); 

	CDataParser *pRevData = new CDataParser( 8192 );

	while(true)
	{
		//Reset recv data
		pRevData->ClearAllBuf();
		pRevData->SetOffset(0);

		int recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

		if(recvbytes > 4 )
		{
			if( pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF && pRevData->GetByte() == 0xFF )
			{
				byte opcode = pRevData->GetByte();

				if( opcode == 0x54 && strstr( pRevData->GetString(), "Source Engine Query" ) ) //'T' - Source Engine Query
				{
					SendInfoAboutServer( pSocket, &pServerInfo );
				}
				else if( opcode == 0x55) //'U' - Get users list
				{
					long challenge = pRevData->GetLong();

					if( challenge == 0xFFFFFFFF || challenge == 0 ) //if get challenge for get players list
					{
						SendPlayersListChallenge( pSocket);
					}
					else
					{
						SendPlayersList( pSocket, &pServerInfo );
					}
				}
				else if( opcode == 0x67) //'g' - Get challenge ???
				{
					pRevData->SetOffset(4); //return to start data

					char *pCommand = pRevData->GetString();

					if( strcmp( "getchallenge steam\n", pCommand ) == 0 )
					{
						SendChallengeForSteam(pSocket);
					}
					else if( strcmp( "getchallenge valve\n", pCommand ) == 0 )
					{
						SendChallengeForValve(pSocket);
					}
				}
				else if( opcode == 0x63) //'c' - Connect ???
				{
					pRevData->SetOffset(4); //return to start data

					char *pCommand = pRevData->GetString();

					if( strstr( pCommand, "connect" ) )
					{
						SendConnectionApproval(pSocket);
					}
				}
			}
			else
			{
				pRevData->SetOffset(0);
				StartCommunicationWithClient( pSocket, pRevData, &pServerInfo, recvbytes );
			}
		}
	}

	return 1;
}

int main(int argc, char* argv[])
{
//	ConnectToServer( "127.0.0.1", 27015 );

	EmulationServer( 27015 );

	return 1;
}

