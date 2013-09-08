////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ���MoveGen.h                                                                                      //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� wying                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��������CMoveGen, CSearch����̳�֮�����̡����ӡ�λ�С�λ�С��ŷ���������������б����塣           //
// 2. ͨ���ƶ�������                                                                                      //
// 3. �����ƶ�������                                                                                      //
// 4. �����ӱ��ƶ�������                                                                                  //
// 5. ɱ���ƶ��Ϸ��Լ���                                                                                  //
// 6. �������Checked(Player), Checking(1-Player)                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "FenBoard.h"

#pragma once


// �����������������
extern int Board[256];								// �������飬��ʾ������ţ�0��15������; 16��31,����; 32��47, ���ӣ�
extern int Piece[48];								// �������飬��ʾ����λ�ã�0, ����������; 0x33��0xCC, ��Ӧ����λ�ã�

// λ����λ����������
extern unsigned int xBitBoard[16];					// 16��λ�У��������ڵĺ����ƶ���ǰ12λ��Ч
extern unsigned int yBitBoard[16];					// 16��λ�У��������ڵ������ƶ���ǰ13λ��Ч

// λ����λ�����̵�ģ
extern const int xBitMask[256];
extern const int yBitMask[256];

// ���ں����������ƶ���16λ���̣�ֻ����ɱ���ƶ��Ϸ��Լ��顢�������ͽ����ӱ�   							          
extern unsigned short xBitRookMove[12][512];		//  12288 Bytes, ����λ������
extern unsigned short yBitRookMove[13][1024];		//  26624 Bytes  ����λ������
extern unsigned short xBitCannonMove[12][512];		//  12288 Bytes  �ڵ�λ������
extern unsigned short yBitCannonMove[13][1024];	    //  26624 Bytes  �ڵ�λ������
									      // Total: //  77824 Bytes =  76K
extern unsigned short HistoryRecord[65535];		// ��ʷ�����������±�Ϊ: move = (nSrc<<8)|nDst;

extern const char nHorseLegTab[512];
extern const char nDirection[512];

class CMoveGen
{
public:
	CMoveGen(void);
	~CMoveGen(void);

	//unsigned short HistoryRecord[65535];		// ��ʷ�����������±�Ϊ: move = (nSrc<<8)|nDst;

// ������Ϣ
public:
	unsigned int nCheckCounts;
	unsigned int nNonCheckCounts;
	unsigned int nCheckEvasions;

// ����
public:	
	// ������ʷ��¼���������˥��
	void UpdateHistoryRecord(unsigned int nMode=0);

	// ͨ���ƶ�������
	int MoveGenerator(const int player, CChessMove* pGenMove);

	// �����ƶ�������
	int CapMoveGen(const int player, CChessMove* pGenMove);

	// �����ӱ��ƶ�������
	int CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove);
	
	// ɱ���ƶ��Ϸ��Լ���
	int IsLegalKillerMove(int Player, const CChessMove KillerMove);

	// ������⣬���������Ƿ������ҷ��Ƿ񱻽���
	int Checked(int player);

	// ������⣬���ؽ������ͣ����ڶԷ��Ƿ񱻽���
	int Checking(int Player);

	// �����ж�
	int Protected(int Player, int from, int nDst);

private:
	// ��������piece�Ƿ��ܹ���nSrc�ƶ���nDst�����ɹ����뵽�߷�����ChessMove��
	int AddLegalMove(const int piece, const int nSrc, const int nDst, CChessMove *ChessMove);
};
