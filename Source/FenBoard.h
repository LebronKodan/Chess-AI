////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ���FenBoard.h                                                                                     //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� wying                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��fen��ת��Ϊ������Ϣ                                                                               //
// 2. ��������Ϣת��Ϊfen��                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#pragma once

#define  CChessMove  unsigned int


const int nPieceType[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// ����
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// ���ӣ�˧��������ʿ��
					       7,  8,  8,  9,  9, 10, 10, 11, 11, 12, 12, 13, 13, 13, 13, 13  };	// ���ӣ������������˱�

const int nPieceID[] = {  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,		// ����
	                       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6,		// ���ӣ�˧��������ʿ��
					       0,  1,  1,  2,  2,  3,  3,  4,  4,  5,  5,  6,  6,  6,  6,  6  };	// ���ӣ������������˱�

class CFenBoard
{
public:
	CFenBoard(void);
	~CFenBoard(void);

public:	
	// ������ֵ��ַ������ַ��������㹻�������򵱻غ���̫��ʱ��moves�������������໥����ʱ�ᷢ����ѭ����
	char FenStr[2048];

public:
	// ��fen��ת��Ϊ�����Ϣ�����سɹ���ʧ�ܵı�־
	int FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr);

	// ����ǰ���ת��Ϊfen�������ش���ָ��
	char *BoardToFen(const int *Board, int Player, const unsigned int nNonCapNum=0, const unsigned int nCurrentStep=1, CChessMove *StepRecords=0);

private:
	// ��fen�ַ�ת��Ϊ�������
	int FenToPiece(char fen);	
};


inline unsigned int Coord(const CChessMove move)
{
    unsigned char RetVal[4];
	unsigned int  src = (move & 0xFF00) >> 8;
	unsigned int  dst = move & 0xFF;

	RetVal[0] = unsigned char(src & 0xF) -  3 + 'a';
	RetVal[1] = 12 - unsigned char(src >> 4 ) + '0';
	RetVal[2] = unsigned char(dst & 0xF) -  3 + 'a';
	RetVal[3] = 12 - unsigned char(dst >> 4 ) + '0';

	return *(unsigned int *) RetVal;
}

inline CChessMove Move(const unsigned int MoveStr) 
{
	unsigned char *ArgPtr = (unsigned char *) &MoveStr;
	unsigned int src = ((12-ArgPtr[1]+'0')<<4) + ArgPtr[0]-'a'+3;	// y0x0
	unsigned int dst = ((12-ArgPtr[3]+'0')<<4) + ArgPtr[2]-'a'+3;	// y1x1
	return ( src << 8 ) | dst;										// y0x0y1x1
}