#pragma once

class CDataParser
{
public:
	CDataParser( unsigned char *pBuff, int bufflen );
	CDataParser( int bufflen );
	~CDataParser(void);

	//Get Data
	byte GetByte();
	short GetShort();
	int GetInt();
	long GetLong();
	float GetFloat( );
	long long GetLongLong();
	char *GetString();

	//Set Data
	void SetByte( byte val );
	void SetShort( short val );
	void SetInt( int val );
	void SetLong( long val );
	void SetFloat( float val );
	void SetLongLong( long long val );
	void SetString( char *pVal );
	void SetData( unsigned char *pVal, int valsize );

	int GetOffset();
	void SetOffset(int val);
	int MoveOffset(int changeoffs);
	void ClearAllBuf( );

	//Full data
	unsigned char *CDataParser::GetFullData();
	int GetFullSize();

	//Current data
	unsigned char *GetCurrentData();
	int GetCurrentSize();

private:
	int offset;
	int buffsize;
	unsigned char *pBuffLink;
	bool isAllocated;
};
