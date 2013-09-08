////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ���HashTable.h                                                                                    //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� wying                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. Ϊ�������Hash��1��1024MB                                                                         //
// 2. Hash̽�� �� Hash�洢                                                                                //
// 3. ��ʼ�����ֿ�                                                                                        //
// 4. ���ҿ��ֿ��е��ŷ�                                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "FenBoard.h"

#pragma once


const int WINSCORE = 10000;				// ʤ���ķ���WINSCORE��ʧ��Ϊ-WINSCORE
const int MATEVALUE = 9000;				// score<-MATEVALUE || score>+MATEVALUE, ˵���Ǹ������ľ���


// ����
const int BookUnique = 1;
const int BookMulti = 2;
const int BookExist = BookUnique | BookMulti;
const int HashAlpha = 4;
const int HashBeta = 8;
const int HashPv = 16;
const int HashExist = HashAlpha | HashBeta | HashPv;
const int MaxBookMove = 15;

#define INT_MIN  (-2147483648)  //const int INT_MIN = -2147483648;//-2147483647-1;   //wyingdebug
#define INT_MAX  (2147483647)  //const int INT_MAX = 2147483647;
#define SHRT_MAX  (32767) //const int SHRT_MAX = 32767;

// ���ֿ��ŷ��ṹ
struct CBookRecord
{
	int MoveNum;
	CChessMove MoveList[MaxBookMove];
};

// �������¼�ṹ
struct CHashRecord					// 14Byte --> 16Byte
{
	unsigned __int64   zobristlock;	// 8 byte  64λ��ʶ
	unsigned char      flag;        // 1 byte  flag==0, Hashֵ�����
	char			   depth;		// 1 byte  �������
	short			   value;       // 2 byte  ��ֵ
	unsigned short     move;		// 2 byte
};


// Hash����
class CHashTable
{
public:
	CHashTable(void);
	virtual ~CHashTable(void);

// 
public:
	//unsigned long    ZobristKeyPlayer;
	//unsigned __int64 ZobristLockPlayer;

	unsigned long    ZobristKey;
	unsigned __int64 ZobristLock;

	unsigned long    ZobristKeyTable[14][256];
	unsigned __int64 ZobristLockTable[14][256];

	unsigned long  nHashSize;			// Hash���ʵ�ʴ�С
	unsigned long  nHashMask;			// nHashMask = nHashSize-1;
	CHashRecord    *pHashList[2];		// Hash���ں�˫������һ�������ⷢ����ͻ������������
										// �ο�����������PC��Ϸ���----�˻�����.pdf��

	unsigned int   nMaxBookPos;
	unsigned int   nBookPosNum;
	CBookRecord    *pBookList;


// ������Ϣ
public:
	unsigned long nCollision;	//Hash��ͻ������
	unsigned long nHashAlpha, nHashExact, nHashBeta;
	unsigned long nHashCovers;


public:
	// ΪHash�����Ϳ��ֿ�����ڴ�
	unsigned long NewHashTable(unsigned long nHashPower, unsigned long nBookPower);

	// ���Hash��Ϳ��ֿ�
	void DeleteHashTable();

	// Hash�����㣬���ҷ���Hash��ĸ�����
	float ClearHashTable(int style=0);

	// ��ʼ����Hash����ص���������
	void InitZobristPiecesOnBoard(int *Piece);

	// Hash̽��
	int ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player);

	// Hash�洢
	void RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player);
	
	// ��ʼ�����ֿ�
	int LoadBook(const char *BookFile);

	// �ڿ��ֿ���Ѱ���ŷ�
	int ProbeOpeningBook(CChessMove &BookMove, int Player);

private:
	// 32λ�����������
	unsigned long Rand32U();

	// ��ʼ��ZobristKeyTable[14][256]��ZobristLockTable[14][256]������32λ��64λ�漴��
	// ֻ��һ�Σ���CHashTable()���캯�����Զ����У������������
	// ���������������򣬷����漴��������ı�
	void ZobristGen(void);
};
