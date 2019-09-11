// GoldSRCPacketDecoder.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include "tools.h"
#include "Csocket.h"
#include "DataParser.h"

//First 8 bytes - header - sequence data and etc

//cl: Header 01 00 00 80 - 00 00 00 00 Data 2C 7D 6E 02 1A 01-11 41
//sv: Header 01 00 00 C0 - 01 00 00 80 Data 5A 18 05 00 1B 00-10 41 5B 52 40 05 49 02 -- many data, all in bz2 stream compressed

//cl: Header 02 00 00 00 - 01 00 00 80 Data 59 19 01 03 19 01-11 43
//cl: Header 03 00 00 00 - 01 00 00 80 Data 58 19 01 02 18 01-11 42

//sv: Header 02 00 00 C0 - 03 00 00 80 Data 5A 18 05 03 18 00-10 42 25 81 40 06 12 48 

//cl: Header 04 00 00 00 - 01 00 00 80 Data 5F 19 01 05 1F 01-11 45
//cl: Header 05 00 00 00 - 02 00 00 00 Data 5E 19 01 04 1E 01-11 44 
//cl: Header 06 00 00 00 - 02 00 00 00 Data 5D 19 01 07 1D 01-11 47

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
	sprintf( pDecodefileName, "srvpack_%d.bin", packid );

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

int StartCommunicationWithServer( char *pIP, int port, Csocket *pSocket, char *pChallenge)
{
	unsigned int send_sequence;
	unsigned int recv_sequence;
	unsigned int sequence_ack;

	int recvbytes = 0;
	unsigned char *pDecodeData = NULL;
	int decodedatasize = 0;
	int packid = 0;

	//First pack
	send_sequence = 0x80000001;
	sequence_ack = 0; //later get from server


	//Data parsers
	CDataParser *pQueryPack = new CDataParser( 8192 );
	CDataParser *pEncodeQuery = new CDataParser( 8192 );
	CDataParser *pRevData = new CDataParser( 8192 );

	//ENCODE AND SAND DATA

	//Reset encode data
	pEncodeQuery->ClearAllBuf();
	pEncodeQuery->SetOffset(0);

	//First send new - for new players
	pEncodeQuery->SetByte( clc_stringcmd );
	pEncodeQuery->SetString( "new" );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	pEncodeQuery->SetByte( clc_nop );
	COM_Munge2(pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset(), send_sequence & 0xFF);

	//Reset query data
	pQueryPack->ClearAllBuf();
	pQueryPack->SetOffset(0);

	//Add header and data to pack
	pQueryPack->SetLong( send_sequence );
	pQueryPack->SetLong( sequence_ack );
	pQueryPack->SetData( pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset() );

	pSocket->Send( pQueryPack->GetFullData(), pQueryPack->GetOffset() );

	//GET DATA AND DECODE

	//Reset recv buff data
	pRevData->ClearAllBuf();
	pRevData->SetOffset(0);

	//Recv and decode pack from server
	recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

	// get sequence numbers
	recv_sequence = pRevData->GetLong();
	sequence_ack = pRevData->GetLong();

	//Decode Data
	COM_UnMunge2(pRevData->GetCurrentData(), pRevData->GetCurrentSize(), recv_sequence & 0xFF);

	pDecodeData = pRevData->GetCurrentData();
	decodedatasize = recvbytes - pRevData->GetOffset();

	//Write decode data to file, for manual anais
	packid++;
	WriteDecodePack( pDecodeData, decodedatasize, packid );

	//reset
	send_sequence = 1;

	//PART2

	while(true)
	{
		Sleep(1000);

		//ENCODE AND SAND DATA
		send_sequence++;

		//Reset encode data
		pEncodeQuery->ClearAllBuf();
		pEncodeQuery->SetOffset(0);

		//First send new - for new players
		pEncodeQuery->SetByte( clc_stringcmd );
		pEncodeQuery->SetString( "say I am bad bot!" );
		COM_Munge2(pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset(), send_sequence & 0xFF);

		//Reset query data
		pQueryPack->ClearAllBuf();
		pQueryPack->SetOffset(0);

		//Add header and data to pack
		pQueryPack->SetLong( send_sequence );
		pQueryPack->SetLong( 0 );
		pQueryPack->SetData( pEncodeQuery->GetFullData(), pEncodeQuery->GetOffset() );

		pSocket->Send( pQueryPack->GetFullData(), pQueryPack->GetOffset() );

		//GET DATA AND DECODE

		//Reset recv buff data
		pRevData->ClearAllBuf();
		pRevData->SetOffset(0);

		//Recv and decode pack from server
		recvbytes = pSocket->Recv(pRevData->GetFullData(), pRevData->GetFullSize() );

		// get sequence numbers
		recv_sequence = pRevData->GetLong();
		sequence_ack = pRevData->GetLong();

		//Decode Data
		COM_UnMunge2(pRevData->GetCurrentData(), pRevData->GetCurrentSize(), recv_sequence & 0xFF);

		pDecodeData = pRevData->GetCurrentData();
		decodedatasize = recvbytes - pRevData->GetOffset();

		//Write decode data to file, for manual anais
		packid++;
		WriteDecodePack( pDecodeData, decodedatasize, packid );
	}

	return 1;
}

int ConnectToServer( char *pIP, int port = 27015)
{
	Csocket *pSocket = new Csocket(eSocketProtocolUDP);
	pSocket->SetAdr( pIP, port );

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
	sprintf( pQueryPack, "\xFF\xFF\xFF\xFF\connect 48 %s \"\\prot\\2\\unique\\-1\\raw\\861078331b85a424935805ca54f82891\" \"\\name\\HLTV Proxy\\cl_lw\\1\\cl_lc\\1\\*hltv\\1\\rate\\10000\\cl_updaterate\\20\\hspecs\\0\\hslots\\0\\hdelay\\30\"\n", pChallenge );
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

int main(int argc, char* argv[])
{
	ConnectToServer( "127.0.0.1", 27015 );

	return 1;
}

