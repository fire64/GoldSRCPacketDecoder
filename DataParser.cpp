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
	buffsize = bufflen;
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

//Set data
void CDataParser::SetByte( byte val )
{
	pBuffLink[offset] = val;
	offset++;
}

void CDataParser::SetShort( short val )
{
	(short)((short*)(pBuffLink + offset))[0] = val;
	offset += sizeof(short);
}

void CDataParser::SetInt( int val )
{
	(int)((int*)(pBuffLink + offset))[0] = val;
	offset += sizeof(int);
}

void CDataParser::SetLong( long val )
{
	(long)((long*)(pBuffLink + offset))[0] = val;
	offset += sizeof(long);
}

void CDataParser::SetLongLong( long long val )
{
	(long long)((long long*)(pBuffLink + offset))[0] = val;
	offset += sizeof(long long);
}

void CDataParser::SetString( char *pVal )
{
	strcpy( (char *)pBuffLink + offset, pVal );
	offset += ( strlen(pVal) + 1 );
}

void CDataParser::SetData( unsigned char *pVal, int valsize )
{
	memcpy( pBuffLink + offset, pVal, valsize );
	offset += valsize;
}