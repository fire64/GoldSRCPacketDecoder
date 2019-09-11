#pragma once

class CDataParser
{
public:
	CDataParser(void);
	CDataParser( unsigned char *pBuff, int bufflen );
	~CDataParser(void);

	//Get Data
	byte GetByte();
	short GetShort();
	int GetInt();
	long GetLong();
	long long GetLongLong();
	char *GetString();

	//SetData
	void SetByte( byte val );
	void SetShort( short val );
	void SetInt( int val );
	void SetLong( long val );
	void SetLongLong( long long val );
	void SetString( char *pVal );
	void SetData( unsigned char *pVal, int valsize );

	int GetOffset();
	int MoveOffset(int changeoffs);

	//Full data
	unsigned char *CDataParser::GetFullData();
	int GetFullSize();

	//Current data
	unsigned char *GetCurrentData();
	int GetCurrentSize();

	int offset;
	int buffsize;
	unsigned char *pBuffLink;
};
