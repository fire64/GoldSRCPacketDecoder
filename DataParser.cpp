#include "StdAfx.h"
#include "DataParser.h"

CDataParser::CDataParser(void)
{

}

CDataParser::~CDataParser(void)
{

}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CDataParser::CDataParser( unsigned char *pBuff, int bufflen )
{
	pBuffLink = pBuff;
	offset = 0;
}

byte CDataParser::GetByte( )
{
	byte val =  pBuffLink[offset];
	offset++;
	return val;
}

short CDataParser::GetShort( )
{
	short val = (short)((short*)(pBuffLink + offset))[0];
	offset += sizeof(short);

	return val;
}

int CDataParser::GetInt( )
{
	int val = (int)((int*)(pBuffLink + offset))[0];
	offset += sizeof(int);

	return val;
}

long CDataParser::GetLong( )
{
	long val = (long)((long*)(pBuffLink + offset))[0];
	offset += sizeof(long);

	return val;
}

long long CDataParser::GetLongLong( )
{
	long long val = (long long)((long long*)(pBuffLink + offset))[0];
	offset += sizeof(long long);

	return val;
}

char *CDataParser::GetString( )
{
	int oldoffset = offset;

	char *pVal = (char *)pBuffLink + offset;

	offset += ( strlen(pVal) + 1 );
	return pVal;
}

int CDataParser::GetOffset()
{
	return offset;
}

int CDataParser::MoveOffset(int changeoffs)
{
	offset += changeoffs;

	return 1;
}