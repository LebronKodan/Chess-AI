////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Դ�ļ���MoveGen.cpp                                                                                    //
// *******************************************************************************************************//
// �й�����ͨ������----�������ģ�֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� �� �� ��                                                                                        //
// ��λ�� �й�ԭ���ܿ�ѧ�о�Ժ                                                                            //
// ���䣺 fan_de_jun@sina.com.cn                                                                          //
//  QQ �� 83021504                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��������CMoveGen, CSearch����̳�֮�����̡����ӡ�λ�С�λ�С��ŷ���������������б����塣           //
// 2. ͨ���ƶ�������                                                                                      //
// 3. �����ƶ�������                                                                                      //
// 4. �����ӱ��ƶ�������                                                                                  //
// 5. ɱ���ƶ��Ϸ��Լ���                                                                                  //
// 6. �������Checked(Player), Checking(1-Player)                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "MoveGen.h"
#include "PreMove.h"


// �����������������
int Board[256];								// �������飬��ʾ������ţ�0��15������; 16��31,����; 32��47, ���ӣ�
int Piece[48];								// �������飬��ʾ����λ�ã�0, ����������; 0x33��0xCC, ��Ӧ����λ�ã�	

// λ����λ������
unsigned int xBitBoard[16];					// 16��λ�У��������ڵĺ����ƶ���ǰ12λ��Ч
unsigned int yBitBoard[16];					// 16��λ�У��������ڵ������ƶ���ǰ13λ��Ч

// ���ں����������ƶ���16λ���̣�ֻ����ɱ���ƶ��Ϸ��Լ��顢�������ͽ����ӱ�   							          
unsigned short xBitRookMove[12][512];		//  12288 Bytes, ����λ������
unsigned short yBitRookMove[13][1024];		//  26624 Bytes  ����λ������
unsigned short xBitCannonMove[12][512];		//  12288 Bytes  �ڵ�λ������
unsigned short yBitCannonMove[13][1024];	//  26624 Bytes  �ڵ�λ������
								  // Total: //  77824 Bytes =  76K

unsigned short HistoryRecord[65535];		// ��ʷ�����������±�Ϊ: move = (nSrc<<8)|nDst;

const int xBitMask[256] = 
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   1,   2,   4,   8,  16,  32,  64, 128, 256,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

const int yBitMask[256] = 
{
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
	0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   0,   0,   0,   0,
	0,   0,   0,   4,   4,   4,   4,   4,   4,   4,   4,   4,   0,   0,   0,   0,
	0,   0,   0,   8,   8,   8,   8,   8,   8,   8,   8,   8,   0,   0,   0,   0,
	0,   0,   0,  16,  16,  16,  16,  16,  16,  16,  16,  16,   0,   0,   0,   0,
	0,   0,   0,  32,  32,  32,  32,  32,  32,  32,  32,  32,   0,   0,   0,   0,
	0,   0,   0,  64,  64,  64,  64,  64,  64,  64,  64,  64,   0,   0,   0,   0,
	0,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128,   0,   0,   0,   0,
	0,   0,   0, 256, 256, 256, 256, 256, 256, 256, 256, 256,   0,   0,   0,   0,
	0,   0,   0, 512, 512, 512, 512, 512, 512, 512, 512, 512,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// ����������ʿ��
static const int MvvValues[48] = 
{
      0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017,
  10000, 2000, 2000, 1096, 1096, 1088, 1088, 1040, 1040, 1041, 1041, 1017, 1018, 1020, 1018, 1017 
};

// ��������������������
// 1. nHorseLegTab[nDst-nSrc+256] != 0				// ˵�����ߡ��ա��ӣ���α�Ϸ��ƶ�
// 2. nLeg = nSrc + nHorseLegTab[nDst-nSrc+256]		// ���ȵĸ���
//    Board[nLeg] == 0								// ����λ��û�����ӣ�����Դ�nSrc�ƶ���nDst
const char nHorseLegTab[512] = {
                               0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0
};


// �ƶ������������ʺϽ������ƶ�������
// 0 --- �����ƶ�������
// 1 --- �������ң��ʺϽ�˧�ͱ�����ƶ�
// 2 --- ʿ�ܹ�����
// 3 --- ���ܹ�����
// 4 --- ���ܹ�����
const char nDirection[512] = {
                                   0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  0,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  3,  2,  1,  2,  3,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  1,  0,  1,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  3,  2,  1,  2,  3,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  4,  3,  0,  3,  4,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0,  0,  0,  0,  0,  0,  0
};


// �ŷ�Ԥ��������
	// �����ƶ� + ��ͨ�ƶ�
static unsigned char KingMoves[256][8];				//   2048 Bytes, �����ƶ�����
static unsigned char xRookMoves[12][512][12];		//  73728 Bytes, ���ĺ����ƶ�
static unsigned char yRookMoves[13][1024][12];		// 159744 Bytes, ���������ƶ�
static unsigned char xCannonMoves[12][512][12];		//  73728 Bytes, �ڵĺ����ƶ�
static unsigned char yCannonMoves[13][1024][12];	// 159744 Bytes, �ڵ������ƶ�
static unsigned char KnightMoves[256][12];			//   3072 Bytes, ����ƶ�����
static unsigned char BishopMoves[256][8];			//   2048 Bytes, ����ƶ�����
static unsigned char GuardMoves[256][8];			//   2048 Bytes, ʿ���ƶ�����
static unsigned char PawnMoves[2][256][4];		    //   2048 Bytes, �����ƶ����飺0-���䣬 1-���
										     // Total: 478208 Bytes = 467KB
	// �����ƶ�
static unsigned char xRookCapMoves[12][512][4];	//  24576 Bytes, ���ĺ����ƶ�
static unsigned char yRookCapMoves[13][1024][4];	//  53248 Bytes, ���������ƶ�
static unsigned char xCannonCapMoves[12][512][4];	//  24576 Bytes, �ڵĺ����ƶ�
static unsigned char yCannonCapMoves[13][1024][4];	//  53248 Bytes, �ڵ������ƶ�
										     // Total: 155648 Bytes = 152KB

CMoveGen::CMoveGen(void)
{
	// ������������������У�CPreMoveֻ�����һ��
	CPreMove PreMove;
	
	PreMove.InitKingMoves(KingMoves);
	PreMove.InitRookMoves(xRookMoves, yRookMoves, xRookCapMoves, yRookCapMoves);
	PreMove.InitCannonMoves(xCannonMoves, yCannonMoves, xCannonCapMoves, yCannonCapMoves);
	PreMove.InitKnightMoves(KnightMoves);
	PreMove.InitBishopMoves(BishopMoves);
	PreMove.InitGuardMoves(GuardMoves);
	PreMove.InitPawnMoves(PawnMoves);

	// ��ʼ�����ڳ��ں����������ƶ���16λ����
	PreMove.InitBitRookMove(xBitRookMove, yBitRookMove);
	PreMove.InitBitCannonMove(xBitCannonMove, yBitCannonMove);
}

CMoveGen::~CMoveGen(void)
{
}


// ������ʷ��¼
// nMode==0  �����ʷ��¼
// nMode!=0  ˥����ʷ��¼��HistoryRecord[m] >>= nMode;
void CMoveGen::UpdateHistoryRecord(unsigned int nMode)
{
	unsigned int m;
	unsigned int max_move = 0xCCCC;			// �ƶ������ֵ0xFFFF��0xCCCC������ƶ������õ�

	if( nMode )								// ˥����ʷ����
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] >>= nMode;
	}
	else									// ���㣬�����µľ���
	{
		for(m=0; m<max_move; m++)
			HistoryRecord[m] = 0;
	}
}

// ����Board[256], �������кϷ����ƶ��� Player==-1(�ڷ�), Player==+1(�췽)
// �ƶ�����Ӳ��� MVV/LVA(�����ƶ�) �� ��ʷ������ϣ��ȵ�������ʷ��������10%
int CMoveGen::MoveGenerator(const int Player, CChessMove* pGenMove)
{	
	const unsigned int  k = (1+Player) << 4;	    //k=16,����; k=32,���塣
	unsigned int  move, nSrc, nDst, x, y, nChess;
	CChessMove* ChessMove = pGenMove;		//�ƶ��ļ�����
	unsigned char *pMove;
							
	// ������˧���ƶ�********************************************************************************************
	nChess = k;
	nSrc = Piece[nChess];								// ��˧���ڣ�nSrc!=0
	{
		pMove = KingMoves[nSrc];
		while( *pMove )
		{
			nDst = *(pMove++);
			if( !Board[nDst] )
			{
				move = (nSrc<<8) | nDst;
				*(ChessMove++) = (HistoryRecord[move]<<16) | move;
			}
		}
	}
	nChess ++;


	// ���������ƶ�************************************************************************************************
	for( ; nChess<=k+2; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{			
			x = nSrc & 0xF;								// ��4λ��Ч
			y = nSrc >> 4;								// ǰ4λ��Ч

			//���ĺ����ƶ���
			pMove = xRookMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//���������ƶ�
			pMove = yRookMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;				// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// �����ڵ��ƶ�************************************************************************************************
	for( ; nChess<=k+4; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{			
			x = nSrc & 0xF;								// ��4λ��Ч
			y = nSrc >> 4;								// ǰ4λ��Ч

			//�ڵĺ����ƶ�
			pMove = xCannonMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}

			//�ڵ������ƶ�
			pMove = yCannonMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nDst = (*(pMove++)) | x;		// 0x y|x  ǰ4λ=y*16�� ��4λ=x	
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// ��������ƶ�******************************************************************************************
	for( ; nChess<=k+6; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = KnightMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
				{					
					if( !Board[nDst] )
					{
						move = (nSrc<<8) | nDst;
						*(ChessMove++) = (HistoryRecord[move]<<16) | move;
					}
				}
			}
		}
	}


	// ��������ƶ�******************************************************************************************
	for( ; nChess<=k+8; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[(nSrc+nDst)>>1] )				//��������
				{
					if( !Board[nDst] )
					{
						move = (nSrc<<8) | nDst;
						*(ChessMove++) = (HistoryRecord[move]<<16) | move;
					}
				}
			}
		}
	}


	// ����ʿ���ƶ�******************************************************************************************
	for( ; nChess<=k+10; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = GuardMoves[nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}


	// ����������ƶ�******************************************************************************************
	for( ; nChess<=k+15; nChess++)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = PawnMoves[Player][nSrc];
			while( *pMove )
			{
				nDst = *(pMove++);
				if( !Board[nDst] )
				{
					move = (nSrc<<8) | nDst;
					*(ChessMove++) = (HistoryRecord[move]<<16) | move;
				}
			}
		}
	}

	return int(ChessMove-pGenMove);
}


// �������кϷ��ĳ����ƶ�
// �������������ƶ�ռ����ʱ���23%��
// ���������ģ���Ϊ�����˷���������Σ���Ҫ�Ǳ�����Ŀ��ࡣ
int CMoveGen::CapMoveGen(const int Player, CChessMove* pGenMove)
{	
	const unsigned int k = (1+Player) << 4;				// k=16,����; k=32,���塣
	unsigned int  nSrc, nDst, x, y, nChess, nCaptured;	
	CChessMove  *ChessMove = pGenMove;					// ����������ƶ�ָ��
	unsigned char *pMove;

	nChess = k+15;

	// ����������ƶ�******************************************************************************************
	for( ; nChess>=k+11; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = PawnMoves[Player][nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 20)>>16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ����ʿ���ƶ�******************************************************************************************
	for( ; nChess>=k+9; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = GuardMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 41)>>16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ��������ƶ�******************************************************************************************
	for( ; nChess>=k+7; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = BishopMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
				{
					if( !Board[(nSrc+nDst)>>1] )					//��������
						*(ChessMove++) = ((MvvValues[nCaptured] - 40)>>16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// ��������ƶ�******************************************************************************************
	for( ; nChess>=k+5; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{
			pMove = KnightMoves[nSrc];
			while( *pMove )
			{
				nCaptured = Board[ nDst = *(pMove++) ];
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
				{
					if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
						*(ChessMove++) = ((MvvValues[nCaptured] - 88)>>16) | (nSrc<<8) | nDst;
				}
			}
		}
	}


	// �����ڵ��ƶ�************************************************************************************************
	for( ; nChess>=k+3; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{			
			x = nSrc & 0xF;								// ��4λ��Ч
			y = nSrc >> 4;								// ǰ4λ��Ч

			//�ڵĺ����ƶ�
			pMove = xCannonCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)>>16) | (nSrc<<8) | nDst;
			}

			//�ڵ������ƶ�
			pMove = yCannonCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{		
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 96)>>16) | (nSrc<<8) | nDst;
			}
		}
	}


	// ���������ƶ�************************************************************************************************
	for( ; nChess>=k+1; nChess--)
	{
		if( nSrc = Piece[nChess] )						// ���Ӵ��ڣ�nSrc!=0
		{			
			x = nSrc & 0xF;								// ��4λ��Ч
			y = nSrc >> 4;								// ǰ4λ��Ч

			//���ĺ����ƶ�
			pMove = xRookCapMoves[x][xBitBoard[y]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (nSrc & 0xF0) | (*(pMove++)) ];	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)>>16) | (nSrc<<8) | nDst;
			}

			//���������ƶ�
			pMove = yRookCapMoves[y][yBitBoard[x]];
			while( *pMove )
			{
				nCaptured = Board[ nDst = (*(pMove++)) | x ];		// 0x y|x  ǰ4λ=y*16�� ��4λ=x
				if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
					*(ChessMove++) = ((MvvValues[nCaptured] - 200)>>16) | (nSrc<<8) | nDst;
			}
		}
	}
                                                                                                                                                                                                                                                                                                                                                     

	// ������˧���ƶ�********************************************************************************************
	nSrc = Piece[nChess];								// ���Ӵ��ڣ�nSrc!=0
	{
		pMove = KingMoves[nSrc];
		while( *pMove )
		{
			nCaptured = Board[ nDst = *(pMove++) ];
			if( (nChess ^ nCaptured) >= 48 )		// ��ɫ����
				*(ChessMove++) = ((MvvValues[nCaptured] - 1000)>>16) | (nSrc<<8) | nDst;
		}
	}	

	return int(ChessMove-pGenMove);
}


// �ж�ɱ�������ŷ��ĺϷ���
int CMoveGen::IsLegalKillerMove(int Player, const CChessMove KillerMove)
{	
	int nSrc = (KillerMove & 0xFF00) >> 8;
	int nMovedChs = Board[nSrc];
	if( (nMovedChs >> 4) != Player+1 )			// ��ɱ�ֲ��Ǳ��������ӣ���Ϊ�Ƿ��ƶ�
		return 0;

	int nDst = KillerMove & 0xFF;
	if( (Board[nDst] >> 4) == Player+1 )		// ��ɱ��Ϊ�����ƶ�����ͬɫ������Ϊ�Ƿ�
		return 0;

	int x, y;
	switch( nPieceID[nMovedChs] )
	{
		case 1:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitRookMove[y][yBitBoard[x]] & yBitMask[nDst];		// x��ȵ������ƶ�
			else if( y == (nDst >> 4) )
				return xBitRookMove[x][xBitBoard[y]] & xBitMask[nDst];		// y��ȵ������ƶ�
			break;

		case 2:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( x == (nDst & 0xF) )
				return yBitCannonMove[y][yBitBoard[x]] & yBitMask[nDst];	// x��ȵ������ƶ�
			else if( y == (nDst >> 4) )
				return xBitCannonMove[x][xBitBoard[y]] & xBitMask[nDst];	// y��ȵ������ƶ�
			break;

		case 3:		// ��
			if( !Board[ nSrc + nHorseLegTab[ nDst-nSrc+256 ] ] )		// ��������
				return 3;
			break;

		case 4:		// ��
			if( !Board[(nSrc+nDst)>>1] )									// ��������
				return 4;
			break;

		default:
			return 1;														// ɱ������, ��ʿ������Ȼ�Ϸ�
			break;
	}

	return 0;
}

// ʹ��λ����λ�м���ʵ�ֵĽ������
// ����һ�������������������طǡ�0������ֵ
// �˺������ڵ�ǰ�ƶ����Ƿ񱻽���
// ע�⣺���ڵ�λ����λ�в���nDst->nSrc��ɱ���ŷ������Լ���nSrc->nDst�����෴
int CMoveGen::Checked(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// �ҷ���˧��λ��
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;				// �Է���˧�����

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];
	
	// ˫�����棺�Խ�������ʹ�ó���λ������
	int nSrc = Piece[king];
	if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
		return nSrc;

	// ������
	nSrc = Piece[king+1];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		return nSrc;

	nSrc = Piece[king+2];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		return nSrc;	


	xBitMove = xBitCannonMove[x][xBitBoard[y]];
	yBitMove = yBitCannonMove[y][yBitBoard[x]];

	// �ڽ���
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		return nSrc;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		return nSrc;


	// ���Է�������
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �����������������ӣ�����
		return nSrc;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �����������������ӣ�����
		return nSrc;


	// ���Է����ӵı��佫��
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// ע�⣺��˧������ʱ�������м����ı������
		return Player ? nKingSq-16 : nKingSq+16;

	if( nPieceID[ Board[nKingSq-1] ]==6 )
		return nKingSq-1;
		
	if( nPieceID[ Board[nKingSq+1] ]==6 )
		return nKingSq+1;

	nNonCheckCounts ++;
	return 0;
}


// ʹ��λ����λ�м���ʵ�ֵĽ������
// ���������ܹ�������˧������checkers�������ӱܺ����ܹ�ֱ��ʹ��checkers�����������ظ�����
// ����ֵ��checkers!=0����ʾ������checkers==0����ʾû�н���
// checkers�ĺ��λ��ʾ���������ͣ��ֱ��ʾ����0x80 ��0x40 ��0x20 ��0x10 ��0x08 ��0x04 ��0x02 ��0x01
// ʹ�ô˺���ǰ��������Checked()�ж��ҷ��Ƿ񱻽�����Ȼ�����ô˺�������Է��Ƿ񱻽���
// ע�⣺��Ϊǰ���Ѿ�ʹ����Checked()�����Դ˺�������Ҫ��˫�������ٴν��м��
// ע�⣺���ڵ�λ����λ�в���nDst->nSrc��ɱ���ŷ������Լ���nSrc->nDst�����෴��������ƿ��Լ��ټ���
int CMoveGen::Checking(int Player)
{
	nCheckCounts ++;
	
	int nKingSq = Piece[(1+Player)<<4];		// ���㽫˧��λ��
	int x = nKingSq & 0xF;
	int y = nKingSq >> 4;
	int king = (2-Player) << 4 ;			// ��˧
	int checkers = 0;
	int nSrc;

	int xBitMove = xBitRookMove[x][xBitBoard[y]];
	int yBitMove = yBitRookMove[y][yBitBoard[x]];

	// ˫�����棺�����⣬���ᷢ��
	//nSrc = Piece[king];
	//if( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] )
	//	checkers |= 0xFF;

	// ��������0x01��ʾ��king+1, 0x02��ʾ��king+2
	nSrc = Piece[king+1];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		checkers |= 0x01;

	nSrc = Piece[king+2];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc] ) )
		checkers |= 0x02;


	xBitMove = xBitCannonMove[x][xBitBoard[y]];
	yBitMove = yBitCannonMove[y][yBitBoard[x]];
	
	// �ڽ�����0x04��ʾ��king+3, 0x08��ʾ��king+4
	nSrc = Piece[king+3];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )
		checkers |= 0x04;

	nSrc = Piece[king+4];
	if( ( x==(nSrc & 0xF) && yBitMove & yBitMask[nSrc] ) ||
		( y==(nSrc >>  4) && xBitMove & xBitMask[nSrc]) )		
		checkers |= 0x08;


	// ������0x10��ʾ��king+5��0x20��ʾ��king+6
	nSrc = Piece[king+5];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �����������������ӣ�����
		checkers |= 0x10;

	nSrc = Piece[king+6];
	x = nHorseLegTab[nKingSq - nSrc + 256];
	if( x && !Board[nSrc + x] )							// �����������������ӣ�����
		checkers |= 0x20;


	// ������䣺0x40��ʾ����ı�/�佫��
	if( nPieceType[ Board[Player ? nKingSq-16 : nKingSq+16] ] == 13-7*Player )		// ��˧������ʱ�������м������б�����
		checkers |= 0x40;

	// ������䣺0x80��ʾ����ı�/�佫��
	if( nPieceID[ Board[nKingSq-1] ]==6 || nPieceID[ Board[nKingSq+1] ]==6 )		// ��ѯ��˧�����Ƿ��б�/�����
		checkers |= 0x80;


	if(!checkers)
		nNonCheckCounts ++;
	return checkers;
}

// �����жϺ���
// ���Ӵ�from->nDst, Playerһ���Ƿ��γɱ���
// ��������������Գ����߷������𵽵����ú�С��
int CMoveGen::Protected(int Player, int from, int nDst)
{
	const int king = (2-Player) << 4 ;			// ��˧
	
	//****************************************************************************************************
	// ��
	int nSrc = Piece[king];
	if( nDirection[nDst-nSrc+256]==1 && nCityIndex[nDst] )
		return 1000;

	// ��
	nSrc = Piece[king+7];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	nSrc = Piece[king+8];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
		return 40;

	// ʿ
	nSrc = Piece[king+9];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	nSrc = Piece[king+10];
	if( nSrc && nSrc!=nDst && nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
		return 41;

	// ����
	nSrc = Player ? nDst-16 : nDst+16;
	if( nPieceType[ Board[nSrc] ] == 13-7*Player )		// ע�⣺��˧������ʱ�������м����ı������
		return 20;

	if( (Player && nDst<128) || (!Player && nDst>=128) )
	{
		if( nPieceID[ Board[nDst-1] ]==6 )
			return 17;
			
		if( nPieceID[ Board[nDst+1] ]==6 )
			return 17;
	}

	//*****************************************************************************************************
	// �����ڡ�����ʱ���б�Ҫ������λ��from�������ӣ����ܹ�����׼ȷ
	int x = nDst & 0xF;
	int y = nDst >> 4;
	int xBitIndex = xBitBoard[y] ^ xBitMask[from];		// ���from֮λ��
	int yBitIndex = yBitBoard[x] ^ yBitMask[from];		// ���from֮λ��

	const int m_Piece = Board[from];					// ����from֮����
	Piece[m_Piece] = 0;									// ��ʱ������Ժ�ָ�

	// ��
	nSrc = Piece[king+1];
	if( nSrc && nSrc!=nDst )
	{
		if(	( x==(nSrc & 0xF) && yBitRookMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitRookMove[x][xBitIndex] & xBitMask[nSrc] ) )
		{
			Piece[m_Piece] = from;
			return 200;
		}
	}

	nSrc = Piece[king+2];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitRookMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >> 4) && xBitRookMove[x][xBitIndex] & xBitMask[nSrc] ) )
		{
			Piece[m_Piece] = from;
			return 200;
		}
	}

	// ��
	nSrc = Piece[king+3];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitCannonMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitCannonMove[x][xBitIndex] & xBitMask[nSrc]) )
		{
			Piece[m_Piece] = from;
			return 96;
		}
	}

	nSrc = Piece[king+4];
	if( nSrc && nSrc!=nDst )
	{
		if( ( x==(nSrc & 0xF) && yBitCannonMove[y][yBitIndex] & yBitMask[nSrc] ) ||
			( y==(nSrc >>  4) && xBitCannonMove[x][xBitIndex] & xBitMask[nSrc]) )		
		{
			Piece[m_Piece] = from;
			return 96;
		}
	}


	// ��
	nSrc = Piece[king+5];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// �����������������ӣ�����
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	nSrc = Piece[king+6];
	if( nSrc!=nDst )
	{
		x = nHorseLegTab[nDst - nSrc + 256];
		if( x && (!Board[nSrc + x] || x==from) )			// �����������������ӣ�����
		{
			Piece[m_Piece] = from;
			return 88;
		}
	}

	// �ָ�from��������
	Piece[m_Piece] = from;

	return 0;
}

int CMoveGen::AddLegalMove(const int nChess, const int nSrc, const int nDst, CChessMove *ChessMove)
{
	int x, y, nCaptured;
	
	switch( nPieceID[nChess] )
	{
		case 0:		// �����������ҡ��ھŹ���
			if( nDirection[nDst-nSrc+256]==1 && nCityIndex[nDst] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 1:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( (x==(nDst & 0xF) && yBitRookMove[y][yBitBoard[x]] & yBitMask[nDst]) ||
				(y==(nDst >>  4) && xBitRookMove[x][xBitBoard[y]] & xBitMask[nDst]) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+1)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 2:		// ��
			x = nSrc & 0xF;
			y = nSrc >> 4;
			if( (x==(nDst & 0xF) && yBitCannonMove[y][yBitBoard[x]] & yBitMask[nDst]) ||
				(y==(nDst >>  4) && xBitCannonMove[x][xBitBoard[y]] & xBitMask[nDst]) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+3)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 3:		// �����ա���������
			x = nHorseLegTab[nDst - nSrc + 256];
			if( x && !Board[nSrc + x] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+3)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 4:		// ������������ӡ���δ����
			if( nDirection[nDst-nSrc+256]==4 && !Board[(nSrc+nDst)>>1] && (nSrc^nDst)<128 )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+5)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 5:		// ʿ��б�ߡ��ھŹ���
			if( nDirection[nDst-nSrc+256]==2 && nCityIndex[nDst] )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+7)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		case 6:		// �������������ҡ��߷�����
			x = nDst-nSrc;						// ��ǰ�ƶ�����Player
			if(  nDirection[x+256]==1 && (
				(nChess<32  && (x==16 || (nSrc>=128 && (x==1 || x==-1)))) ||
				(nChess>=32 && (x==-16 || (nSrc<128 && (x==1 || x==-1)))) ) )
			{
				nCaptured = Board[nDst];
				if( !nCaptured )
					*(ChessMove++) = (nSrc<<8) | nDst;
				else if( (nChess^nCaptured) >= 48 )
					*(ChessMove++) = ((MvvValues[nCaptured]+9)<<16) | (nSrc<<8) | nDst;
				return 1;
			}
			break;

		default:
			break;
	}

	return 0;
}


//********************�����ӱܲ�������CheckEvasionGen()***********************************************
// �˺�����������ȫ�⽫��Ŀ�ģ����ܹ��ѽ�������жϵĴ��������١�
// ����ȫ�⽫������ǳ����ӣ�ÿ���ƶ�һ�����ӣ���Ҫ�����Ƿ��öԷ��Ľ���������Ȼ�����ɶ������µĽ�����
// ��Ȼ�ǲ���ȫ�⽫������һ��Ҫ�ӽ�����⡣���������ȫ�⽫����������һЩ���п�����ǰ�õ���֦��
// Player   ��ʾ��������һ��������ǰ�ƶ���
// checkers ������⺯��Checking(Player)�ķ���ֵ����ʾ���������ͣ���������֪�����ļ������ӽ���
//          ��8λ��Ч���ֱ��ʾ��������ݱ�����2����1����2����1����2����1
//          ֮���Բ�������˧�Ľ�����Ϣ������ΪChecked(Player)�����Ѿ����˴���
int CMoveGen::CheckEvasionGen(const int Player, int checkers, CChessMove* pGenMove)
{
	nCheckEvasions ++;							// ͳ�ƽ⽫���������д���

	const int MyKing   = (1+Player) << 4;		// MyKing=16,����; MyKing=32,���塣
	const int OpKing = (2-Player) << 4;			// �Է������������
	const int nKingSq = Piece[MyKing];			// ���㽫˧��λ��

	int nDir0=0, nDir1=0;						// ��������1������16������
	int nCheckSq0, nCheckSq1;					// �������ӵ�λ��
	int nMin0, nMax0, nMin1, nMax1;				// ���ڳ����뽫˧֮��ķ�Χ
	int nPaojiazi0, nPaojiazi1;					// �ڼ��ӵ�λ��	
	int nPaojiaziID0, nPaojiaziID1;				// �ڼ��ӵ���������

	int nChess, nCaptured, nSrc, nDst, x, y;
	unsigned char *pMove;
	CChessMove *ChessMove = pGenMove;			// ��ʼ���ƶ���ָ��

	// �⺬�ڽ����ķ�����ӣ���Ժ��ڽ������������⴦���Լ����ظ�����
	if( checkers & 0x0C )
	{
		// ��һ���ڽ�����λ��
		nCheckSq0 = Piece[ OpKing + (checkers&0x04 ? 3:4) ];

		// ��������1������16������
		nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;

		// �ڽ�֮��ķ�Χ[nMin0, nMax0)����������˧��λ��
		nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
		nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;		

		// Ѱ���ڼ��ӵ�λ�ã�nPaojiazi0
		// �����ڼ��ӵ����ͣ�nPaojiaziID0
		for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
		{
			if( Board[nDst] && nDst!=nCheckSq0 )
			{
				nPaojiazi0 = nDst;
				nPaojiaziID0 = Board[nDst];
				break;
			}
		}
		
		// ���ڼ����ǶԷ����ڣ���������˫��֮����ƶ�
		if( nPaojiaziID0==OpKing+3 || nPaojiaziID0==OpKing+4 )
		{
			nMin0 = nPaojiazi0>nCheckSq0 ? nCheckSq0 : nPaojiazi0+nDir0;
			nMax0 = nPaojiazi0<nCheckSq0 ? nCheckSq0 : nPaojiazi0-nDir0;
		}
	}

	// ���ݡ��������͡����н⽫
	// ������ٷ����������ÿ���ܹ��⽫������
	switch( checkers )
	{
		// ����������ɱ����
		case 0x01:
		case 0x02:
			nCheckSq0 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];			
			nDir0 = (nKingSq&0xF)==(nCheckSq0&0xF) ? 16:1;
			nMin0 = nKingSq>nCheckSq0 ? nCheckSq0 : nKingSq+nDir0;
			nMax0 = nKingSq<nCheckSq0 ? nCheckSq0 : nKingSq-nDir0;

			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				// ��������ʿ����ɱ�����ߵ���
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			break;


		// ���ڽ�����ɱ����
		case 0x04:  // ��1
		case 0x08:  // ��2
			// ɱ�ڡ��赲�ڽ������ݲ������ڼ��ӵ��ƶ�
			// �ڼ��ӱ���Ҳ����ɱ�ڣ������ܲ�����������ķǳ����ƶ�
			x = nDir0==1 ? MyKing+10 : MyKing+15;
			for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
			{	
				if( nDst==nPaojiazi0 )							// ɱ�ڼ������ã�
					continue;

				// ��������ʿ����ɱ�ڻ��ߵ���
				for(nChess=MyKing+1; nChess<=x; nChess++)
				{
					nSrc = Piece[nChess];
					if(nSrc && nSrc!=nPaojiazi0)				// �ڼ������ҷ����ӣ��ݲ����������ƶ�
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}

			// �������ڼ��ӵ��ƶ����γɿ�ͷ�ڣ�ʹ��ɥʧ����������
			// ���ڼ����ǳ���������ɱ�������ڣ��ڼ������ڣ�����Խ���Է����ڻ��߼����Ľ�ɱ�����롣
			// ���ڳ��ڱ������������ӣ�Ӧ�ý�ֹ���������ϵķǳ����ƶ����ƶ������ڼ��ӣ�
			// �����Է��ڵı��󣬴�����һ���ڣ�Ψ�����ڴ�ԣ����Խ⽫���������ӳ������γ�˫��֮��
			if( (nPaojiaziID0-16)>>4==Player )					// �ڼ����Ǽ���������
			{
				nSrc = nPaojiazi0;
				nChess = Board[nSrc];
				x = nSrc & 0xF;								// ��4λ��Ч
				y = nSrc >> 4;								// ǰ4λ��Ч

				switch( nPieceID[Board[nSrc]] )
				{
					case 1:			
						// ���ĺ����ƶ������򽫾�ʱ�������Ժ�����
						//               ���򽫾�ʱ��������ɱ��
						pMove = xRookMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}
						// ���������ƶ������򽫾�ʱ��������������
						pMove = yRookMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;				// 0x y|x  ǰ4λ=y*16�� ��4λ=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}
						break;

					case 2:
						// �ڵĺ����ƶ������򽫾�ʱ���ڿ��Ժ����룻�����ݺὫ�����ڶ����Գ�������
						pMove = xCannonMoves[x][xBitBoard[y]];
						while( *pMove )
						{
							nDst = (nSrc & 0xF0) | (*(pMove++));	// 0x y|x  ǰ4λ=y*16�� ��4λ=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==16 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}

						// �ڵ������ƶ������򽫾�ʱ���ڿ��������룻�����ݺὫ�����ڶ����Գ�������
						pMove = yCannonMoves[y][yBitBoard[x]];
						while( *pMove )
						{
							nDst = (*(pMove++)) | x;		// 0x y|x  ǰ4λ=y*16�� ��4λ=x
							nCaptured = Board[nDst];

							if( !nCaptured && nDir0==1 )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}
						break;

					case 3:		
						// �����뽫������
						pMove = KnightMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);			
							nCaptured = Board[nDst];

							if( !Board[nSrc+nHorseLegTab[nDst-nSrc+256]] )
							{
								if( !nCaptured )
									*(ChessMove++) = (nSrc<<8) | nDst;
								else if( (nChess^nCaptured) >= 48 )
									*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
							}
						}
						break;

					case 4:
						// �����뽫������
						pMove = BishopMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);				
							nCaptured = Board[nDst];

							if( !Board[(nSrc+nDst)>>1] )					//��������
							{
								if( !nCaptured )
									*(ChessMove++) = (nSrc<<8) | nDst;
								else if( (nChess^nCaptured) >= 48 )
									*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
							}
						}
						break;

					case 5:
						// ʿ�����뽫������
						pMove = GuardMoves[nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);
							nCaptured = Board[nDst];

							if( !nCaptured )
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured) >= 48 )
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}
						break;

					case 6:
						// ���䣺���򽫾����������뽫�����򣻺��򽫾������䲻�ܵ���⽫λ��
						pMove = PawnMoves[Player][nSrc];
						while( *pMove )
						{
							nDst = *(pMove++);
							nCaptured = Board[nDst];
							
							if( !nCaptured && nDir0==16 && x != (nDst&0xF) )			// �������룬��ֹ�����ƶ�
								*(ChessMove++) = (nSrc<<8) | nDst;
							else if( (nChess^nCaptured)>=48 )							// ����ɱ��������
								*(ChessMove++) = (MvvValues[nCaptured]>>16) | (nSrc<<8) | nDst;
						}
						break;

					default:
						break;
				}
			}
			// �ڼ����ǶԷ��������γ������֮�ƣ��������ƽ����⽫������Ϊֹ�����򷵻�
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			break;


		// �ڳ�������
		case 0x05:  // ��1��1
		case 0x06:  // ��1��2
		case 0x09:  // ��2��1
		case 0x0A:  // ��2��2
			nCheckSq1 = Piece[ OpKing + (checkers&0x01 ? 1:2) ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// ���ڷֱ���������򽫾��������ڼ����ǶԷ����ӣ��޽�
			if( nDir0!=nDir1 && (nPaojiaziID0-16)>>4==1-Player )
				return 0;
			// �ڼ����ǶԷ��������γ������֮�ƣ��޽�
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			// ���ڼ����ǶԷ��ĳ����������ڳ���֮��ķǳ����ƶ��ƶ������������뽫
			else if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;

				x = nDir0==1 ? MyKing+10 : MyKing+15;
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{	
					// ��������ʿ����ɱ�����ߵ���
					for(nChess=MyKing+1; nChess<=x; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
					}
				}
			}
			// �ڼ����Ǽ���������(������ʿ)�������޷��⽫
			else if( nPaojiaziID0>=MyKing+3 && nPaojiaziID0<=MyKing+10 )
			{
				// �����ڼ���ɱ�������赲�����ƶ�
				nChess = Board[nPaojiazi0];
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
					ChessMove += AddLegalMove(nChess, nPaojiazi0, nDst, ChessMove);
			}

			break;


		// ���ڽ��������ڳ�����
		// "4ka2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		// "4kR2C/9/4b4/9/9/4C4/9/9/9/4K4 b - - 0 1"
		case 0x0C:	// ��1��2		
		case 0x0D:  // ��1��2��1
		case 0x0E:  // ��1��2��2
			// ���ڼ����ǶԷ��ĳ����������ڳ���֮����ƶ�
			if( nPaojiaziID0==OpKing+1 || nPaojiaziID0==OpKing+2 )
			{
				nMin0 = nKingSq>nPaojiazi0 ? nPaojiazi0+nDir0 : nKingSq+nDir0;
				nMax0 = nKingSq<nPaojiazi0 ? nPaojiazi0-nDir0 : nKingSq-nDir0;
			}
			

			nCheckSq1 = Piece[ OpKing + 4 ];
			nDir1 = (nKingSq&0xF)==(nCheckSq1&0xF) ? 16:1;
			nMin1 = nKingSq>nCheckSq1 ? nCheckSq1 : nKingSq+nDir1;
			nMax1 = nKingSq<nCheckSq1 ? nCheckSq1 : nKingSq-nDir1;

			// Ѱ���ڼ��ӵ�λ��
			for(nDst=nMin1+nDir1; nDst<nMax1; nDst+=nDir1)
			{
				if( Board[nDst] )
				{
					nPaojiazi1 = nDst;
					nPaojiaziID1 = Board[nDst];
					break;
				}
			}

			// ���ڼ����ǶԷ��ĳ����������ڳ���֮����ƶ�
			if( nPaojiaziID1==OpKing+1 || nPaojiaziID1==OpKing+2 )
			{
				nMin1 = nKingSq>nPaojiazi1 ? nPaojiazi1+nDir0 : nKingSq+nDir0;
				nMax1 = nKingSq<nPaojiazi1 ? nPaojiazi1-nDir0 : nKingSq-nDir0;
			}
			
			// �ڼ����Ǽ��������ӣ�����ʿ
			if( nPaojiaziID0>=MyKing+5 && nPaojiaziID0<=MyKing+10 )
			{
				for(nDst=nMin1; nDst<=nMax1; nDst+=nDir1)
				{
					if( nDst==nPaojiazi1 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
				}
			}
			// �ڼ����Ǽ��������ӣ�����ʿ
			if( nPaojiaziID1>=MyKing+5 && nPaojiaziID1<=MyKing+10 )	
			{
				for(nDst=nMin0; nDst<=nMax0; nDst+=nDir0)
				{
					if( nDst==nPaojiazi0 )
						continue;
					ChessMove += AddLegalMove(nPaojiaziID1, nPaojiazi1, nDst, ChessMove);
				}
			}

			// �ڼ����ǶԷ��������γ������֮�ƣ��ƽ������ܽ⽫�����˷���
			if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return int(ChessMove-pGenMove);
			}

			// �ڼ����ǶԷ��������γ������֮�ƣ��ƽ������ܽ⽫�����˷���
			if( nPaojiaziID1==OpKing+5 || nPaojiaziID1==OpKing+6 )
			{
				if( nKingSq-nPaojiazi1==2  || nKingSq-nPaojiazi1==-2 || 
					nKingSq-nPaojiazi1==32 || nKingSq-nPaojiazi1==-32 )
				return int(ChessMove-pGenMove);
			}

			break;

		// ��������
		case 0x14:	// ��1��1
		case 0x18:  // ��2��1
		case 0x24:	// ��1��2
		case 0x28:  // ��2��2
			// �ڼ����Ǽ��������ӣ������ڡ�����ʿ�����������ͱ�
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq1 = Piece[ OpKing + (checkers&0x10 ? 5:6) ];
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
				ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256], ChessMove);
			}
			break;


		// �������������������λ����ͬʱ�������ڼ���(������ʿ)�����ȵ��ƶ�
		// "3k1a2C/1r3N3/4N4/5n3/9/9/9/9/9/3K5 b - - 0 1"
		case 0x34:  // ��1��1��2
		case 0x38:  // ��2��1��2
			// �ڼ����Ǽ��������ӣ������ڡ�����ʿ�����������ͱ�
			if( nPaojiaziID0>=MyKing+1 && nPaojiaziID0<=MyKing+10 )
			{
				nCheckSq0 = Piece[ OpKing + 5 ];
				nCheckSq1 = Piece[ OpKing + 6 ];
				nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
				if( nDst==nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nDst, ChessMove);
			}
			break;


		// �ڱ��������ڼ��ӳԱ������ڼ����Ǳ���ֻ���ƽ��⽫
		case 0x44:  // ��1�ݱ�
		case 0x48:  // ��2�ݱ�
		case 0x84:  // ��1���
		case 0x88:  // ��2���
			// �ڼ����Ǽ���������
			if( (nPaojiaziID0-16)>>4==Player )					
			{
				// �ڼ���ɱ����֮��
				if( checkers<=0x48 )
					ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nKingSq+(Player?-16:16), ChessMove);
				// �ڼ���ɱ����֮��
				else
				{
					nCheckSq0 = nKingSq-1;
					nCheckSq1 = nKingSq+1;
					// ���Ҷ��Ǳ������������ַ����⽫
					if( Board[nCheckSq0]!=Board[nCheckSq1] )
					{
						// �ڼ��ӳ����
						if( nPieceID[Board[nCheckSq0]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq0, ChessMove);
						// �ڼ��ӳ��ұ�
						if( nPieceID[Board[nCheckSq1]]==6 )
							ChessMove += AddLegalMove(nPaojiaziID0, nPaojiazi0, nCheckSq1, ChessMove);
					}
				}
			}
			// �ڼ����ǶԷ��������γ������֮�ƣ����ܽ⽫
			else if( nPaojiaziID0==OpKing+5 || nPaojiaziID0==OpKing+6 )
			{
				if( nKingSq-nPaojiazi0==2  || nKingSq-nPaojiazi0==-2 || 
					nKingSq-nPaojiazi0==32 || nKingSq-nPaojiazi0==-32 )
				return 0;
			}
			break;


		// ������������������
		case 0x10:
		case 0x20:
			nCheckSq0 = Piece[ OpKing + (checkers&0x10 ? 5:6) ];
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nCheckSq0, ChessMove);
			}

			nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
			}

			break;

		// ˫���������������λ����ͬʱ�����������ȵ��ƶ�
		case 0x30:
			nCheckSq0 = Piece[ OpKing + 5 ];
			nCheckSq1 = Piece[ OpKing + 6 ];
			nDst = nCheckSq0+nHorseLegTab[nKingSq-nCheckSq0+256];
			if( nDst == nCheckSq1+nHorseLegTab[nKingSq-nCheckSq1+256] )
			{
				// ��������ʿ
				for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
				{
					if( (nSrc=Piece[nChess]) != 0 )
						ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
				}
			}
			break;

		// ���򵥱����Ա�
		case 0x40:
			nDst = nKingSq+(Player?-16:16);
			// ��������ʿ
			for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
			{
				if( (nSrc=Piece[nChess]) != 0 )
					ChessMove += AddLegalMove(nChess, nSrc, nDst, ChessMove);
			}
			break;

		// ������䣺�Ա�
		case 0x80:
			nCheckSq0 = nKingSq-1;
			nCheckSq1 = nKingSq+1;

			// ���Ҷ��Ǳ������������ַ����⽫
			if( Board[nCheckSq0]!=Board[nCheckSq1] )
			{
				// �����
				if( nPieceID[Board[nCheckSq0]]==6 )
				{
					// ��������ʿ
					for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nCheckSq0, ChessMove);
					}
				}

				// ���ұ�
				if( nPieceID[Board[nCheckSq1]]==6 )
				{
					// ��������ʿ
					for(nChess=MyKing+1; nChess<=MyKing+10; nChess++)
					{
						if( (nSrc=Piece[nChess]) != 0 )
							ChessMove += AddLegalMove(nChess, nSrc, nCheckSq1, ChessMove);
					}
				}
			}
			break;

		default:
			break;
	}

	// �ƽ������н⽫
	pMove = KingMoves[nKingSq];
	while( *pMove )
	{
		nDst = *(pMove++);
		nCaptured = Board[nDst];			
		
		if( !nCaptured && (													// �����ǳ����ƶ�
			(!nDir0 && !nDir1) ||											// û�г��ڽ���
			(nDir0!=1 && nDir1!=1 && (nKingSq&0xF0)==(nDst&0xF0)) ||		// �������ƶ������ڲ��ɺ��򽫾�
			(nDir0!=16 && nDir1!=16 && (nKingSq&0xF)==(nDst&0xF)) ) )		// �������ƶ������ڲ������򽫾�
			*(ChessMove++) = (nKingSq<<8) | nDst;
		else if( (MyKing^nCaptured) >= 48 )					// ���������ƶ�
			*(ChessMove++) = (MvvValues[nCaptured]<<16) | (nKingSq<<8) | nDst;
	}
	
	return int(ChessMove-pGenMove);
}
