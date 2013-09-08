////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ���Search.h                                                                                       //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� IwfWcf                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��ΪCMoveGen�����࣬�̳и�������ݣ������̡����ӡ��ŷ��ȡ�                                          //
// 2. ���ս�������ݣ���ʼ��Ϊ������Ҫ����Ϣ��                                                            //
// 3. ����CHashTable�ִ࣬�кͳ����ŷ���                                                                  //
// 4. ����ð�ݷ����ŷ�����                                                                                //
// 5. ������������ MainSearch()                                                                           //
// 6. ���ڵ��������� RootSarch()                                                                          //
// 7. �����������㷨 AlphaBetaSearch()                                                                    //
// 8. �ž������㷨 QuiescenceSearch()                                                                     //
// 9. ����1999��桶�й����徺������ʵ��ѭ�����                                                        //
//10. ѭ���ķ���ֵ���⡢��Hash���ͻ���⡣                            //
//11. ����Hash���ȡ����֧                                                                                //
//12. ����ʱ��Ŀ���                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <utility>
#include <algorithm>
#include "MoveGen.h"
#include "HashTable.h"
#include "Evaluation.h"

using namespace std;

#define X first
#define Y second
#define MP make_pair

#pragma once

typedef pair<int, int> PII;

// 
const int MaxKiller = 4;
struct CKillerMove
{
	int MoveNum;
	CChessMove MoveList[MaxKiller];
};


// CSearch��
class CSearch : public CMoveGen
{
public:
	CSearch(void);
	~CSearch(void);

// ����
	FILE *OutFile;
	CFenBoard fen;

	int Player;								// ��ǰ�ƶ���	
	int MaxDepth;							// ����������
	unsigned long BitPieces;				// 32�����ӵ�λͼ��0-15Ϊ���壬16-31Ϊ����  0 11 11 11 11 11 11111 0 11 11 11 11 11 11111
	
	CHashTable m_Hash;			// ��ϣ��
	CEvaluation m_Evalue;

	// ��ucciЭ��ͨѶ�ı���
	int bPruning;							// �Ƿ�����ʹ��NullMove
	int bKnowledge;							// ��ֵ������ʹ��
	int nSelectivity;						// ѡ����ϵ����ͨ����0,1,2,3�ĸ�����ȱʡΪ0
	int Debug;								// bDegug = ~0, �����ϸ��������Ϣ
	int SelectMask;							// ѡ����ϵ��
	int nStyle;								// ����(0)����ͨ(1)��ð��(2)
	int Ponder;								// ��̨˼��
	int bStopThinking;						// ֹͣ˼��
	int QhMode;								// �����Ƿ�ʹ��ǳ��Э�飽��
	int bBatch;								// �ͺ�̨˼����ʱ������й�
	int StyleShift;
	long nMinTimer, nMaxTimer;				// ����˼��ʱ��
	int bUseOpeningBook;					// �Ƿ�������ʹ�ÿ��ֿ�
	int nMultiPv;							// ��Ҫ��������Ŀ
	int nBanMoveNum;						// ������Ŀ
	CChessMove BanMoveList[111];			// ���ֶ���
	int NaturalBouts;						// ��Ȼ����
	
	
	unsigned int StartTimer, FinishTimer;	// ����ʱ��
	unsigned int nNonCapNum;				// �߷�����δ���ӵ���Ŀ��>=120(60�غ�)Ϊ���壬>=5���ܳ���ѭ��
	unsigned int nStartStep;
	unsigned int nCurrentStep;				// ��ǰ�ƶ��ļ�¼���
	
	unsigned int nPvLineNum;
	CChessMove PvLine[64];					// ����֧·��
	CChessMove StepRecords[256];			// �߷���¼
	unsigned int nZobristBoard[256];
	PII Record[111];
	PII Score[111];
	

	int nFirstLayerMoves;
	CChessMove FirstLayerMoves[111];		// fen C8/3P1P3/3kP1N2/5P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// ��������    way = 111;
											// fen C8/3P1P3/4k1N2/3P1P3/4N1P2/7R1/1R7/4B3B/3KA4/2C6 r - - 0 1	// �ǽ�������  way = 110;


// ����
public:
	unsigned int nTreeNodes;	
	unsigned int nLeafNodes;	
	unsigned int nQuiescNodes;

	unsigned int nTreeHashHit;
	unsigned int nLeafHashHit;

	unsigned int nNullMoveNodes;
	unsigned int nNullMoveCuts;

	unsigned int nHashMoves;
	unsigned int nHashCuts;	

	unsigned int nKillerNodes[MaxKiller];
	unsigned int nKillerCuts[MaxKiller];

	unsigned int nCapCuts;
	unsigned int nCapMoves;

	unsigned int nBetaNodes;
	unsigned int nPvNodes;
	unsigned int nAlphaNodes;

	
	unsigned int nZugzwang;

	//char FenBoard[2048];	// ������ֵ��ַ������ַ��������㹻�������򵱻غ���̫��ʱ��moves�������������໥����ʱ�ᷢ����ѭ����


// ����
public:
	void InitBitBoard(const int Player, const int nCurrentStep);		// ��ʼ��������Ҫ���������ݣ������ҷ��������ı�־��

	int MovePiece(const CChessMove move);	 //&---���Դ������������ٶȣ��븴��Դ�����ٶ���ȣ�inline�������á�
	void UndoMove(void);

	//void BubbleSortMax(CChessMove *ChessMove, int w, int way);	//ð������ֻ�����һ�Σ�Ѱ�����ֵ���������һ�ν�����¼��λ��

	int MainSearch(int nDepth, long nProperTimer=0, long nLimitTimer=0);
	//int RootSearch(int depth, int alpha, int beta);
	int SimpleSearch(int depth, int alpha, int beta);
	PII PrincipalVariation(int depth, int alpha, int beta);
	PII NegaScout(int depth, int alpha, int beta);
	PII FAlphaBeta(int depth, int alpha, int beta);
	//int AlphaBetaSearch(int depth, int bDoCut, CKillerMove &KillerTab, int alpha, int beta);
	//int QuiescenceSearch(int depth, int alpha, int beta);	
	
	int RepetitionDetect(void);								// ѭ�����
	int LoopValue(int Player, int ply, int nLoopStyle);		// ѭ����ֵ

	//int Evaluation(int player);

	int IsBanMove(CChessMove move);
	void GetPvLine(void);
	void PopupInfo(int depth, int score, int Debug=0);
	int Interrupt(void);


	char *GetStepName(const CChessMove ChessMove, int *Board) const;
	void SaveMoves(char *FileName);
};
