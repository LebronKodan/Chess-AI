////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Դ�ļ���Search.cpp                                                                                     //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� IwfWcf                                                                                       //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��ΪCMoveGen�����࣬�̳и�������ݣ������̡����ӡ��ŷ��ȡ�                                          //
// 2. ���ս�������ݣ���ʼ��Ϊ������Ҫ����Ϣ��                                                            //
// 3. ����CHashTable�ִ࣬�кͳ����ŷ���                                                                  //
// 4. ����ð�ݷ����ŷ�����                                                                                //
// 5. ������������ MainSearch()                                                                           //
// 6.                                                                         //
// 7. �����������㷨 AlphaBetaSearch()                                                                    //
// 8.                                                                     //
// 9. ����1999��桶�й����徺������ʵ��ѭ�����                                                        //
//10. ѭ���ķ���ֵ���⡢��Hash���ͻ���⡣                            //
//11. ����Hash���ȡ����֧                                                                                //
//12. ����ʱ��Ŀ���                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include <time.h>
#include <string.h>
#include <assert.h>

#include "ucci.h"
#include "Search.h"

static const unsigned int nAdaptive_R = 3;
static const unsigned int nVerified_R = 3;
static const unsigned int BusyCounterMask = 4095;
static const unsigned int BitAttackPieces = 0xF87EF87E;    //11111000011111101111100001111110 ����������ʿʿ�����������ڳ���������������ʿʿ�����������ڳ���˧


CSearch::CSearch(void)
{	
	// ��ʼ������
	StepRecords[0] = 0;	
	
	bPruning = 1 ;							// ����ʹ��NullMove
	nSelectivity = 0;						// ѡ����ϵ����ͨ����0,1,2,3�ĸ�����
	Debug = 0;								// bDegug = 0, ���������Ҫ��������Ϣ
	SelectMask = 0;
	nStyle = 1;
	Ponder = 0;
	bStopThinking = 0;
	QhMode = 0;
	bBatch = 0;
	StyleShift = 5;
	bUseOpeningBook = 0;
	NaturalBouts = 120;
	nBanMoveNum = 0;

	// ��ʼ��Hash������21+1=22��Hash��64MB
	m_Hash.NewHashTable(22, 12);
	m_Hash.ClearHashTable();

	// ��ʼ����ʷ��
	UpdateHistoryRecord( 0 );
}

CSearch::~CSearch(void)
{
	m_Hash.DeleteHashTable();
}

// �ж��ƶ��Ƿ�Ϊ����
int CSearch::IsBanMove(CChessMove move)
{
	int m;
	for(m=0; m<nBanMoveNum; m++)
	{
		if( move == BanMoveList[m] )
			return true;
	}
	return 0;
}

//��ȵ�������
int CSearch::MainSearch(int nDepth, long nProperTimer, long nLimitTimer)
{
	// ��ʼ��һЩ���õı���
	int w, MoveStr, score=0;
	nPvLineNum = PvLine[0] = 0;
	
	// ��Щ�������ڲ������������ܵĸ��ֲ���
	nNullMoveNodes = nNullMoveCuts = 0;
	nHashMoves = nHashCuts = 0;	
	nAlphaNodes = nPvNodes = nBetaNodes = 0;
	nTreeNodes = nLeafNodes = nQuiescNodes = 0;
	m_Hash.nCollision = m_Hash.nHashAlpha = m_Hash.nHashExact = m_Hash.nHashBeta = 0;
	nTreeHashHit = nLeafHashHit = 0;
	nCheckEvasions = 0;
	nZugzwang = 0;	
	nCapCuts = nCapMoves = 0;
	for(w=0; w<MaxKiller; w++)
		nKillerCuts[w] = nKillerNodes[w] = 0;
	nCheckCounts = nNonCheckCounts = 0;
	// ��Щ�������ڲ������������ܵĸ��ֲ���


	//һ����������ʱ��
	StartTimer = clock();
	//nMinTimer = StartTimer + unsigned int(nProperTimer*0.618f);
	nMinTimer = StartTimer + CLOCKS_PER_SEC * 19.5;
	nMaxTimer = unsigned int(nProperTimer*1.618f);
	if(nMaxTimer > nLimitTimer)
		nMaxTimer = nLimitTimer;
	nMaxTimer += StartTimer;
	bStopThinking = 0;


	//���������ǰ����
	fen.BoardToFen(Board, Player, nNonCapNum, nCurrentStep, StepRecords);
	fprintf(OutFile, "info BoardToFen: %s\n", fen.FenStr);
	fflush(OutFile);


	//�����ڿ��ֿ��н�������
	//CChessMove BookMove;
	//if(bUseOpeningBook && m_Hash.ProbeOpeningBook(BookMove, Player))
	//{
	//	nPvLineNum = 1;
	//	PvLine[0] = BookMove;
	//	score = BookMove >> 16;
	//	MoveStr = Coord( BookMove );

	//	fprintf(OutFile, "info depth 0 score %d pv %.4s\n", score, &MoveStr);
	//	fflush(OutFile);
	//	fprintf(OutFile, "bestmove %.4s\n", &MoveStr);
	//	fflush(OutFile);

	//	return score;
	//}


	nStartStep = nCurrentStep;		// ��ʼ����ʱ�İ�غ�����

	score = SimpleSearch(nDepth, -WINSCORE, WINSCORE);
		

	//�����������
	// ���кϷ����ƶ������ bestmove %.4s �� ponder %.4s  �Լ���ϸ��������Ϣ��
	PopupInfo(MaxDepth, score, 1);
	if( nPvLineNum )
	{
		MoveStr = Coord(PvLine[0]);
		fprintf(OutFile, "bestmove %.4s", &MoveStr);
		if(Ponder && nPvLineNum>1)
		{
			MoveStr = Coord(PvLine[1]);
			fprintf(OutFile, " ponder %.4s", &MoveStr);
		}
		fprintf(OutFile, "\n");
		fflush(OutFile);
	}
	// ����ѭ���������ںϷ����ƶ�������score����ζ�Ž�����Ϸ��
	else
	{
		fprintf(OutFile, "depth %d score %d\n", MaxDepth, score);
		fflush(OutFile);
		fprintf(OutFile, "nobestmove\n");
		fflush(OutFile);
	}
	//fprintf(OutFile, "\n\n");
	//fflush(OutFile);


	//���Hash�����ʷ������
	StartTimer = clock() - StartTimer;
	m_Hash.ClearHashTable( 2 );
	SaveMoves("SearchInfo.txt");	
	UpdateHistoryRecord( 4 );
	nBanMoveNum = 0;

	return(score);
}

// ��һ����������
int CSearch::SimpleSearch(int depth, int alpha, int beta)
{	
	//nTreeNodes ++;	// ͳ����֦�ڵ�

	int score, nBestValue = -WINSCORE, w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // ��ȡ��ǰ�Ļغ���
	const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// ���߷�����(ȫ�ֱ���)�л�ȡ(�ҷ�)������־
	
	CChessMove ThisMove, BestMove = 0;									// ��ʼ��Ϊ���š����γ����գ����Է��ش˿��š�
	CChessMove HashMove = 0;												// HashMove

	int HashFlag = HashAlpha;												// Hash��־										// �������ڷ��صļ�С����ֵ
	nFirstLayerMoves = 0;

	for(w=1; w<=depth; w++)
	{	
		MaxDepth = w;
		UpdateHistoryRecord( 0 );

		PII temp = PrincipalVariation(MaxDepth, alpha, beta);
		if (temp.X == -WINSCORE && temp.Y == -WINSCORE) break;
		nBestValue = temp.X, BestMove = temp.Y;

		// ���нKֹ˼����ֹͣ����
		if(bStopThinking)
			break;

		//if (clock() > nMinTimer) break;

		// ��Ҏ������ȃȣ��������壬ֹͣ˼����
		if(nBestValue<-MATEVALUE || nBestValue>MATEVALUE)
			break;
	}

	m_Hash.RecordHash( BestMove, nBestValue, HashFlag, 0, ply, Player );
	return nBestValue;
}

PII CSearch::PrincipalVariation(int depth, int alpha, int beta)
{
	int score, nBestValue = -WINSCORE, w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // ��ȡ��ǰ�Ļغ���
	const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// ���߷�����(ȫ�ֱ���)�л�ȡ(�ҷ�)������־
	
	CChessMove ThisMove, BestMove = 0;									// ��ʼ��Ϊ���š����γ����գ����Է��ش˿��š�
	CChessMove HashMove = 0;												// HashMove

	int HashFlag = HashAlpha;												// Hash��־										// �������ڷ��صļ�С����ֵ
	nFirstLayerMoves = 0;

	if (!depth) return MP(m_Evalue.Evaluation(Player), BestMove);

	nTreeNodes ++;	// ͳ����֦�ڵ�

	//�������кϷ����ƶ�
	//1.��������  �����������ӱ��ŷ���
	//2.�ǽ������棺�����ŷ��ͷǳ����ŷ��������ŷ�������ʷ�÷֣�ȫ������ʷ��������
	CChessMove ChessMove[111], tmp[111];
	if( nChecked )
		way = CheckEvasionGen(Player, nChecked, ChessMove);					// �����ӱܽ������ŷ�
	else
	{
		way  = CapMoveGen(Player, ChessMove);								// �������еĳ����ƶ�
		//for(w=0; w<way; w++)
			//ChessMove[w] += HistoryRecord[ChessMove[w] & 0xFFFF] << 16;		// �����ŷ� + ��ʷ����
		way += MoveGenerator(Player, ChessMove+way);						// �������зǳ����ƶ�
	}

	for (int i = 0; i < way; i++) tmp[i] = ChessMove[i];
	if (MaxDepth > 1 && depth == MaxDepth) {
		for (int i = 0; i < way; i++) ChessMove[i] = tmp[Record[way - 1 - i].Y];
	} else {
		for (int i = 0; i < way; i++) Score[i].X = HistoryRecord[ChessMove[i] & 0xFFFF], Score[i].Y = i;
		sort(Score, Score + way);
		for (int i = 0; i < way; i++) ChessMove[i] = tmp[Score[way - 1 - i].Y];
	}

	if (depth == MaxDepth) 
		for (int i = 0; i < way; i++) Record[i].X = -WINSCORE, Record[i].Y = i;
	
	int nChecking;
	for(w=0; w<way; w++)
	{
		ThisMove = ChessMove[w] & 0xFFFF;

		// ����HashMove�ͽ�ֹ�ŷ�
		if( /*ThisMove==HashMove ||*/ IsBanMove(ThisMove) )
			continue;
		
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// ��Ӧ�ó��֣������д���ͨ��������⣬��ǰһ�����˵������߷�
		nCaptured = MovePiece( ThisMove );										// ע�⣺�ƶ���Player�Ѿ���ʾ�Է���������жϲ�Ҫ�������������ܱ�Ť�����������ط��ܷ��㣬�������ù���
		
		//if (Checked(1 - Player)) nBestValue = ply+1-WINSCORE;
		nBestValue = -PrincipalVariation(depth - 1, -beta, -alpha).X;
		if (clock() > nMinTimer) return(MP(-WINSCORE, -WINSCORE));
		if (depth == MaxDepth) Record[w].X = nBestValue;
		BestMove = ThisMove;
		UndoMove();					// �ָ��ƶ����ָ��ƶ������ָ�һ��
		nNonCapNum = nNonCaptured;	// �ָ�ԭ������ɱ���岽��Ŀ
		break;
	}

	for(++w; w<way; w++)
	{
		ThisMove = ChessMove[w] & 0xFFFF;

		// ����HashMove�ͽ�ֹ�ŷ�
		if( /*ThisMove==HashMove || */IsBanMove(ThisMove) )
			continue;
		if (nBestValue < beta) {
			if (nBestValue > alpha) alpha = nBestValue;
			nCaptured = MovePiece( ThisMove );
			if (Checked(1 - Player)) score = ply+1-WINSCORE;
			else {
				score = -PrincipalVariation(depth - 1, -alpha - 1, -alpha).X;
				if (clock() > nMinTimer) return(MP(-WINSCORE, -WINSCORE));
				if (score > alpha && score < beta) {
					nBestValue = -PrincipalVariation(depth - 1, -beta, -score).X;
					if (clock() > nMinTimer) return(MP(-WINSCORE, -WINSCORE));
					BestMove = ThisMove;
					if (depth == MaxDepth) Record[w].X = nBestValue;
				} else {
					if (score > nBestValue) nBestValue = score, BestMove = ThisMove;
					if (depth == MaxDepth) Record[w].X = score;
				}
			}
			UndoMove();					// �ָ��ƶ����ָ��ƶ������ָ�һ��
			nNonCapNum = nNonCaptured;	// �ָ�ԭ������ɱ���岽��Ŀ
		} else
			break;
	}

	HistoryRecord[BestMove] += 2 << depth;

	if (depth == MaxDepth) sort(Record, Record + way);

	return MP(nBestValue, BestMove);
}

PII CSearch::NegaScout(int depth, int alpha, int beta)
{
	int score, nBestValue = -WINSCORE, w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // ��ȡ��ǰ�Ļغ���
	const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// ���߷�����(ȫ�ֱ���)�л�ȡ(�ҷ�)������־
	
	CChessMove ThisMove, BestMove = 0;									// ��ʼ��Ϊ���š����γ����գ����Է��ش˿��š�
	CChessMove HashMove = 0;												// HashMove

	int HashFlag = HashAlpha;												// Hash��־										// �������ڷ��صļ�С����ֵ
	nFirstLayerMoves = 0;

	if (!depth) return MP(m_Evalue.Evaluation(Player), BestMove);

	nTreeNodes ++;	// ͳ����֦�ڵ�

	//�������кϷ����ƶ�
	//1.��������  �����������ӱ��ŷ���
	//2.�ǽ������棺�����ŷ��ͷǳ����ŷ��������ŷ�������ʷ�÷֣�ȫ������ʷ��������
	CChessMove ChessMove[111], tmp[111];
	if( nChecked )
		way = CheckEvasionGen(Player, nChecked, ChessMove);					// �����ӱܽ������ŷ�
	else
	{
		way  = CapMoveGen(Player, ChessMove);								// �������еĳ����ƶ�
		//for(w=0; w<way; w++)
			//ChessMove[w] += HistoryRecord[ChessMove[w] & 0xFFFF] << 16;		// �����ŷ� + ��ʷ����
		way += MoveGenerator(Player, ChessMove+way);						// �������зǳ����ƶ�
	}

	//if (MaxDepth > 1) {
		for (int i = 0; i < way; i++) tmp[i] = ChessMove[i];
		if (depth == MaxDepth) {
			for (int i = 0; i < way; i++) ChessMove[i] = tmp[Record[way - 1 - i].Y];
		} else {
			for (int i = 0; i < way; i++) Score[i].X = HistoryRecord[ChessMove[i] & 0xFFFF], Score[i].Y = i;
			sort(Score, Score + way);
			for (int i = 0; i < way; i++) ChessMove[i] = tmp[Score[way - 1 - i].Y];
		}
	//}

	if (depth == MaxDepth) 
		for (int i = 0; i < way; i++) Record[i].X = -WINSCORE, Record[i].Y = i;
	
	int nChecking, a = alpha, b = beta;
	bool first = true;
	for(w=0; w<way; w++)
	{
		ThisMove = ChessMove[w] & 0xFFFF;

		// ����HashMove�ͽ�ֹ�ŷ�
		if( /*ThisMove==HashMove ||*/ IsBanMove(ThisMove) )
			continue;
		
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// ��Ӧ�ó��֣������д���ͨ��������⣬��ǰһ�����˵������߷�
		nCaptured = MovePiece( ThisMove );										// ע�⣺�ƶ���Player�Ѿ���ʾ�Է���������жϲ�Ҫ�������������ܱ�Ť�����������ط��ܷ��㣬�������ù���
		
		if (Checked(1 - Player)) score = ply+1-WINSCORE;
		else {
			score = -NegaScout(depth - 1, -b, -a).X;
			if (clock() > nMinTimer) return(MP(-WINSCORE, -WINSCORE));
			if (score > alpha && score < beta && !first) {
				a = -NegaScout(depth - 1, -beta, -score).X;
				if (clock() > nMinTimer) return(MP(-WINSCORE, -WINSCORE));
				BestMove = ThisMove;
				if (depth == MaxDepth) Record[w].X = a;
			}
			UndoMove();					// �ָ��ƶ����ָ��ƶ������ָ�һ��
			nNonCapNum = nNonCaptured;	// �ָ�ԭ������ɱ���岽��Ŀ
			first = false;
			if (depth == MaxDepth) Record[w].X = score;
			if (score > a) a = score, BestMove = ThisMove;
			if (a >= beta) break;
			b = a + 1;
		}
	}

	HistoryRecord[BestMove] += 2 << depth;

	if (depth == MaxDepth) sort(Record, Record + way);

	return MP(a, BestMove);
}

PII CSearch::FAlphaBeta(int depth, int alpha, int beta)
{
	nTreeNodes ++;	// ͳ����֦�ڵ�

	int score, current = -WINSCORE, w, way, nCaptured;	
		
	const int  ply = nCurrentStep - nStartStep;			                    // ��ȡ��ǰ�Ļغ���
	const unsigned int nNonCaptured = nNonCapNum;
	const int nChecked = (StepRecords[nCurrentStep-1] & 0xFF000000) >> 24;	// ���߷�����(ȫ�ֱ���)�л�ȡ(�ҷ�)������־
	
	CChessMove ThisMove, BestMove = 0;									// ��ʼ��Ϊ���š����γ����գ����Է��ش˿��š�
	CChessMove HashMove = 0;												// HashMove

	int HashFlag = HashAlpha;												// Hash��־										// �������ڷ��صļ�С����ֵ
	nFirstLayerMoves = 0;

	if (!depth) return MP(m_Evalue.Evaluation(Player), BestMove);

	//int nHashValue = m_Hash.ProbeHash(HashMove, alpha, beta, 127, ply, Player);
	
	//�������кϷ����ƶ�
	//1.��������  �����������ӱ��ŷ���
	//2.�ǽ������棺�����ŷ��ͷǳ����ŷ��������ŷ�������ʷ�÷֣�ȫ������ʷ��������
	CChessMove ChessMove[111];
	if( nChecked )
		way = CheckEvasionGen(Player, nChecked, ChessMove);					// �����ӱܽ������ŷ�
	else
	{
		way  = CapMoveGen(Player, ChessMove);								// �������еĳ����ƶ�
		for(w=0; w<way; w++)
			ChessMove[w] += HistoryRecord[ChessMove[w] & 0xFFFF] << 16;		// �����ŷ� + ��ʷ����
		way += MoveGenerator(Player, ChessMove+way);						// �������зǳ����ƶ�
	}
	
	int nChecking;
	for(w=0; w<way; w++)
	{
		ThisMove = ChessMove[w] & 0xFFFF;

		// ����HashMove�ͽ�ֹ�ŷ�
		if( ThisMove==HashMove || IsBanMove(ThisMove) )
			continue;
		
		assert(Board[ThisMove & 0xFF]!=16 && Board[ThisMove & 0xFF]!=32);		// ��Ӧ�ó��֣������д���ͨ��������⣬��ǰһ�����˵������߷�
		nCaptured = MovePiece( ThisMove );										// ע�⣺�ƶ���Player�Ѿ���ʾ�Է���������жϲ�Ҫ�������������ܱ�Ť�����������ط��ܷ��㣬�������ù���
	
		score = -FAlphaBeta(depth - 1, -beta, -alpha).X;
		UndoMove();					// �ָ��ƶ����ָ��ƶ������ָ�һ��
		nNonCapNum = nNonCaptured;	// �ָ�ԭ������ɱ���岽��Ŀ
		if(score > current) {
			current = score, BestMove = ThisMove;
			if (score > alpha) alpha = score;
			if (score >= beta) break;
		}
	}
	
	//m_Hash.RecordHash(BestMove, current, HashFlag, depth, ply, Player);
	return MP(current, BestMove);
}

int CSearch::MovePiece(const CChessMove move)
{	
	int nSrc = (move & 0xFF00) >> 8;
	int nDst = move & 0xFF;
	int nMovedChs = Board[nSrc];
	int nCaptured = Board[nDst];
	int nMovedPiece = nPieceType[nMovedChs];

	assert( nMovedChs>=16 && nMovedChs<48 );
	assert( nCaptured>=0 && nCaptured<48 );
	

	//��������
	Board[nSrc] = 0;
	Board[nDst] = nMovedChs;


	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	
	//������������
	Piece[nMovedChs] = nDst;
	Evalue[Player] += PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// ���¹�ֵ
	

	if( nCaptured )
	{
		nNonCapNum = 0;
		Piece[nCaptured] = 0;
		BitPieces ^= 1<<(nCaptured-16);

		int nKilledPiece = nPieceType[nCaptured];
		
		m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nKilledPiece][nDst];
		m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nKilledPiece][nDst];

		Evalue[1-Player] -= PositionValue[nKilledPiece][nDst] + BasicValues[nKilledPiece];
	}
	else
		nNonCapNum ++;				// ˫����ɱ�ӵİ�غ���


	// �������nSrc��λ����λ�У��á�0��
	xBitBoard[ nSrc >> 4 ]  ^= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] ^= yBitMask[nSrc];	

	// �����յ�nDst��λ����λ�У��á�1��
	xBitBoard[ nDst >> 4 ]  |= xBitMask[nDst];
	yBitBoard[ nDst & 0xF ] |= yBitMask[nDst];

	
	//��¼��ǰ�����ZobristKey������ѭ��̽�⡢�������
	StepRecords[nCurrentStep] = move  | (nCaptured<<16);
	nZobristBoard[nCurrentStep] = m_Hash.ZobristKey;		// ��ǰ���������
	nCurrentStep++;


	Player = 1 - Player;		// �ı��ƶ���
	//m_Hash.ZobristKey ^= m_Hash.ZobristKeyPlayer;
    //m_Hash.ZobristLock ^= m_Hash.ZobristLockPlayer;
	
	return(nCaptured);
}

void CSearch::UndoMove(void)
{
	CChessMove move = StepRecords[nCurrentStep-1];
	int nSrc = (move & 0xFF00) >> 8;;
	int nDst = move & 0xFF;
	int nMovedChs = Board[nDst];
	int nMovedPiece = nPieceType[nMovedChs];
	int nCaptured = (move & 0xFF0000) >> 16;


	// ���Ȼָ��ƶ���
	Player = 1 - Player;		
	//m_Hash.ZobristKey ^= m_Hash.ZobristKeyPlayer;
    //m_Hash.ZobristLock ^= m_Hash.ZobristLockPlayer;

	m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nMovedPiece][nSrc] ^ m_Hash.ZobristKeyTable[nMovedPiece][nDst];
	m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nMovedPiece][nSrc] ^ m_Hash.ZobristLockTable[nMovedPiece][nDst];

	//��������������
	Board[nSrc] = nMovedChs;
	Board[nDst] = nCaptured;
	Piece[nMovedChs] = nSrc;
	Evalue[Player] -= PositionValue[nMovedPiece][nDst] - PositionValue[nMovedPiece][nSrc];	// ���¹�ֵ


	// �ָ�λ����λ�е���ʼλ��nSrc��ʹ�á�|���������á�1��
	xBitBoard[ nSrc >> 4 ]  |= xBitMask[nSrc];
	yBitBoard[ nSrc & 0xF ] |= yBitMask[nSrc];
	
	if( nCaptured )							//�����ƶ�
	{
		int nKilledPiece = nPieceType[nCaptured];

		Piece[nCaptured] = nDst;			//�ָ���ɱ���ӵ�λ�á�
		BitPieces ^= 1<<(nCaptured-16);

		m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[nKilledPiece][nDst];
		m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[nKilledPiece][nDst];

		Evalue[1-Player] += PositionValue[nKilledPiece][nDst] + BasicValues[nKilledPiece];
	}
	else									//���Ƿǳ����ƶ�������ѷ���λ����"0"
	{
		// ���λ����λ�е���ʼλ��nDst��ʹ�á�^���������á�0��
		// ��֮�����ǳ����ƶ����յ�λ�ñ�������"1"�����Բ��ûָ���
		xBitBoard[ nDst >> 4 ]  ^= xBitMask[nDst];
		yBitBoard[ nDst & 0xF ] ^= yBitMask[nDst];
	}

	//����߷�����
	nCurrentStep --;
	nNonCapNum --;
}

// ��������λ����Ϣ����ʼ���������������ӵ�����
// Ҳ��ʹ��������Ϣ����ѭ��256�Σ��ٶ�������
void CSearch::InitBitBoard(const int Player, const int nCurrentStep)
{
	int m,n,x,y;	
	int chess;
	
	// ��ʼ��������
	BitPieces = 0;
	Evalue[0] = Evalue[1] = 0;
	m_Hash.ZobristKey  = 0;
	m_Hash.ZobristLock = 0;
	for(x=0; x<16; x++)
		xBitBoard[x] = 0;
	for(y=0; y<16; y++)
		yBitBoard[y] = 0;
	
	// ����32������λ�ø���
	for(n=16; n<48; n++)
	{
		m = Piece[n];													// ����λ��
		if( m )															// ��������
		{
			chess               = nPieceType[n];						// �������ͣ�0��14
			BitPieces          |= 1<<(n-16);							// 32������λͼ
			xBitBoard[m >> 4 ] |= xBitMask[m];							// λ��
			yBitBoard[m & 0xF] |= yBitMask[m];							// λ��
			m_Hash.ZobristKey  ^= m_Hash.ZobristKeyTable[chess][m];		// Hash��
			m_Hash.ZobristLock ^= m_Hash.ZobristLockTable[chess][m];	// Hash��
			
			if(n!=16 && n!=32)			
				Evalue[ (n-16)>>4 ] += PositionValue[chess][m] + BasicValues[chess];
		}
	}

	//m_Hash.InitZobristPiecesOnBoard( Piece );

	// ����ѭ�����
	nZobristBoard[nCurrentStep-1] =  m_Hash.ZobristKey;

	// ��ǰ�ƶ����Ƿ񱻽�����д���߷�����
	// StepRecords[nCurrentStep-1] |= Checking(Player) << 24;
}

//��Ӧ�������������ϸ���жϣ�Ϊ��ͨ�ã���ò�ʹ��CString��
char *CSearch::GetStepName(CChessMove ChessMove, int *Board) const
{
	//���ӱ��
	static char StepName[12];	// �����þ�̬�����������ܷ���

	static const char ChessName[14][4] = {"��","܇","��","��","��","ʿ","��", "��","܇","��","�R","��","��","��"};

	static const char PostionName[2][16][4] = { {"", "", "", "��","��","��","��","��","��","��","��","��", "", "", "", ""}, 
	                                            {"", "", "", "��","��","��","��","��","��","��","��","һ", "", "", "", ""} };

	const int nSrc = (ChessMove & 0xFF00) >> 8;
	const int nDst = ChessMove & 0xFF;

	if( !ChessMove )
		return("HashMove");

	const int nMovedChs = Board[nSrc];
	const int x0 = nSrc & 0xF;
	const int y0 = nSrc >> 4;
	const int x1 = nDst & 0xF;
	const int y1 = nDst >> 4;

	const int Player = (nMovedChs-16) >> 4;	

	strcpy( StepName, ChessName[nPieceType[nMovedChs]] );
	strcat( StepName, PostionName[Player][x0] );
	
	//������x0�Ƿ������һ�ųɶԵ�����.
	int y,chess;
	if( nPieceID[nMovedChs]!=0 && nPieceID[nMovedChs]!=4 && nPieceID[nMovedChs]!=5 )	// ����ʿ����������
	{
		for(y=3; y<13; y++)
		{
			chess = Board[ (y<<4) | x0 ];

			if( !chess || y==y0)														// ���ӻ���ͬһ�����ӣ���������
				continue;

			if( nPieceType[nMovedChs] == nPieceType[chess] )							// ������һ����ͬ������
			{
				if( !Player )			// ����
				{
					if(y > y0)
						strcpy( StepName, "ǰ" );
					else
						strcpy( StepName, "��" );
				}
				else					// ����
				{
					if(y < y0)
						strcpy( StepName, "ǰ" );
					else
						strcpy( StepName, "��" );
				}

				strcat( StepName, ChessName[nPieceType[nMovedChs]] );
				break;
			}
		}
	}

	int piece = nPieceID[nMovedChs];

	//��, ��, ƽ
	if(y0==y1)
	{
		strcat( StepName, "ƽ" );
		strcat( StepName, PostionName[Player][x1]);					// ƽ���κ����Ӷ��Ծ���λ�ñ�ʾ
	}
	else if((!Player && y1>y0) || (Player && y1<y0))
	{
		strcat( StepName, "��" );

		if(piece==3 || piece==4 || piece==5)						// ����ʿ�þ���λ�ñ�ʾ
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// ���������ڡ��������λ�ñ�ʾ
			strcat( StepName, PostionName[1][y1-y0+12] );			// �췽
		else
			strcat( StepName, PostionName[0][y1-y0+2] );			// �ڷ�
	}
	else
	{
		strcat( StepName, "��" );

		if(piece==3 || piece==4 || piece==5)						// ����ʿ�þ���λ�ñ�ʾ
			strcat( StepName, PostionName[Player][x1] );			
		else if(Player)												// ���������ڡ��������λ�ñ�ʾ
			strcat( StepName, PostionName[1][y0-y1+12] );			// �췽
		else
			strcat( StepName, PostionName[0][y0-y1+2] );			// �ڷ�		
	}

	return(StepName);
}



// �������֧
// ����ʹ����Hash����ʱ����֧�Ǵ���ģ��д�������������
void CSearch::GetPvLine(void)
{
	CHashRecord *pHashIndex = m_Hash.pHashList[Player] + (m_Hash.ZobristKey & m_Hash.nHashMask);		//�ҵ���ǰ����Zobrist��Ӧ��Hash��ĵ�ַ

	if((pHashIndex->flag & HashExist) && pHashIndex->zobristlock==m_Hash.ZobristLock)
	{
		if( pHashIndex->move )
		{
			PvLine[nPvLineNum] = pHashIndex->move;
			
			MovePiece( PvLine[nPvLineNum] );

			nPvLineNum++;

			if( nNonCapNum<4 || !RepetitionDetect() )
				GetPvLine();

			UndoMove();
		}
	}
}

void CSearch::PopupInfo(int depth, int score, int Debug)
{
	unsigned int n;
	int MoveStr;
	if(depth)
	{
		fprintf(OutFile, "info depth %d score %d pv", depth, score);
		
		n = nNonCapNum;
		nPvLineNum = 0;
		GetPvLine();
		nNonCapNum = n;

		for(n=0; n<nPvLineNum; n++) 
		{
			MoveStr = Coord(PvLine[n]);
			fprintf(OutFile, " %.4s", &MoveStr);
		}

		fprintf(OutFile, "\n");
		fflush(OutFile);
	}

	if(Debug)
	{
		n = nTreeNodes + nLeafNodes + nQuiescNodes;
		fprintf(OutFile, "info Nodes %d = %d(T) + %d(L) + %d(Q)\n", n, nTreeNodes, nLeafNodes, nQuiescNodes);
		fflush(OutFile);

		float SearchTime = (clock() - StartTimer)/(float)CLOCKS_PER_SEC;
		fprintf(OutFile, "info TimeSpan = %.3f s\n", SearchTime);
		fflush(OutFile);

		fprintf(OutFile, "info NPS = %d\n", int(n/SearchTime));
		fflush(OutFile);
	}	
}


void CSearch::SaveMoves(char *szFileName)
{
	unsigned int m, n;
	int k, nSrc, nDst, nCaptured;
	
	// �����ļ�����ɾ��ԭ�������ݡ��������ָ�ʽ������ȽϷ��㡣
	FILE *out = fopen(szFileName, "w+");

	fprintf(out, "***************************������Ϣ***************************\n\n");

	fprintf(out, "������ȣ�%d\n", MaxDepth);
	n = nTreeNodes + nLeafNodes + nQuiescNodes;
	fprintf(out, "TreeNodes : %u\n", n);
	fprintf(out, "TreeStruct: BranchNodes = %10u\n", nTreeNodes);
	fprintf(out, "            LeafNodes   = %10u\n", nLeafNodes);
	fprintf(out, "            QuiescNodes = %10u\n\n", nQuiescNodes);

	float TimeSpan = StartTimer/1000.0f;
	fprintf(out, "����ʱ��    :   %8.3f ��\n", TimeSpan);
	fprintf(out, "֦Ҷ�����ٶ�:   %8.0f NPS\n", (nTreeNodes+nLeafNodes)/TimeSpan);
	fprintf(out, "���������ٶ�:   %8.0f NPS\n\n", n/TimeSpan);

	fprintf(out, "Hash���С: %d Bytes  =  %d M\n", m_Hash.nHashSize*2*sizeof(CHashRecord), m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
	fprintf(out, "Hash������: %d / %d = %.2f%%\n\n", m_Hash.nHashCovers, m_Hash.nHashSize*2, m_Hash.nHashCovers/float(m_Hash.nHashSize*2.0f)*100.0f);

	unsigned int nHashHits = m_Hash.nHashAlpha+m_Hash.nHashExact+m_Hash.nHashBeta;
	fprintf(out, "Hash����: %d = %d(alpha:%.2f%%) + %d(exact:%.2f%%) +%d(beta:%.2f%%)\n", nHashHits, m_Hash.nHashAlpha, m_Hash.nHashAlpha/(float)nHashHits*100.0f, m_Hash.nHashExact, m_Hash.nHashExact/(float)nHashHits*100.0f, m_Hash.nHashBeta, m_Hash.nHashBeta/(float)nHashHits*100.0f);
	fprintf(out, "���и���: %.2f%%\n", nHashHits/float(nTreeNodes+nLeafNodes)*100.0f);
	fprintf(out, "��֦����: %d / %d = %.2f%%\n", nTreeHashHit, nTreeNodes, nTreeHashHit/(float)nTreeNodes*100.0f);
	fprintf(out, "Ҷ������: %d / %d = %.2f%%\n\n", nLeafHashHit, nLeafNodes, nLeafHashHit/(float)nLeafNodes*100.0f);

	fprintf(out, "NullMoveCuts   = %u\n", nNullMoveCuts);
	fprintf(out, "NullMoveNodes  = %u\n", nNullMoveNodes);
	fprintf(out, "NullMove��֦�� = %.2f%%\n\n", nNullMoveCuts/(float)nNullMoveNodes*100.0f);

	fprintf(out, "Hash��ͻ   : %d\n", m_Hash.nCollision);
	fprintf(out, "Null&Kill  : %d\n", m_Hash.nCollision-nHashMoves);
	fprintf(out, "HashMoves  : %d\n", nHashMoves);
	fprintf(out, "HashCuts   : %d\n", nHashCuts);
	fprintf(out, "Hash��֦�� : %.2f%%\n\n", nHashCuts/(float)nHashMoves*100.0f);

	fprintf(out, "ɱ���ƶ� : \n");
	k = n = 0;
	for(m=0; m<MaxKiller; m++)
	{
		fprintf(out, "    Killer   %d : %8d /%8d = %.2f%%\n", m+1, nKillerCuts[m], nKillerNodes[m], nKillerCuts[m]/float(nKillerNodes[m]+0.001f)*100.0f);
		n += nKillerCuts[m];
		k += nKillerNodes[m];
	}
	fprintf(out, "    ɱ�ּ�֦�� : %8d /%8d = %.2f%%\n\n", n, k, n/float(k+0.001f)*100.0f);


	fprintf(out, "�����ƶ���֦�� = %d / %d = %.2f%%\n\n", nCapCuts, nCapMoves, nCapCuts/(float)nCapMoves*100.0f);


	m = nBetaNodes + nPvNodes + nAlphaNodes;
	fprintf(out, "�ǳ����ƶ�: %d\n", m);	
	fprintf(out, "    BetaNodes: %10d  %4.2f%%\n", nBetaNodes, nBetaNodes/float(m)*100.0f);
	fprintf(out, "    PvNodes  : %10d  %4.2f%%\n", nPvNodes, nPvNodes/float(m)*100.0f);
	fprintf(out, "    AlphaNode: %10d  %4.2f%%\n\n", nAlphaNodes, nAlphaNodes/float(m)*100.0f);

	m += nNullMoveCuts + nHashMoves + nKillerNodes[0] + nKillerNodes[1] + nCapMoves;
	fprintf(out, "TotalTreeNodes: %d\n\n\n", m);

	n = nCheckCounts-nNonCheckCounts;
	fprintf(out, "��������: %d\n", n);
	fprintf(out, "̽�����: %d\n", nCheckCounts);
	fprintf(out, "�ɹ�����: %.2f%%\n\n", n/(float)nCheckCounts*100.0f);

	fprintf(out, "CheckEvasions = %d\n", nCheckEvasions);
	fprintf(out, "�⽫ / ����   = %d / %d = %.2f%%\n\n", nCheckEvasions, n, nCheckEvasions/float(n)*100.0f);


	// ��ʾ����֧
	int BoardStep[256];
	for(n=0; n<256; n++)
		BoardStep[n] = Board[n];

	static const char ChessName[14][4] = {"��","܇","��","��","��","ʿ","��", "��","܇","��","�R","��","��","��"};

	fprintf(out, "\n����֧��PVLine***HashDepth**************************************\n");
	for(m=0; m<nPvLineNum; m++)
	{
		nSrc = (PvLine[m] & 0xFF00) >> 8;
		nDst = PvLine[m] & 0xFF;
		nCaptured = BoardStep[nDst];

		// �غ������岽����
		fprintf(out, "    %2d. %s", m+1, GetStepName( PvLine[m], BoardStep ));

		// �����ŷ�
		if( nCaptured )
			fprintf(out, " k-%s", ChessName[nPieceType[nCaptured]]);
		else
			fprintf(out, "     ");

		// �������
		fprintf(out, "  depth = %2d", PvLine[m]>>16);

		// ������־
		nCaptured = (PvLine[m] & 0xFF0000) >> 16;
		if(nCaptured)
			fprintf(out, "   Check Extended 1 ply ");
		fprintf(out, "\n");

		BoardStep[nDst] = BoardStep[nSrc];
		BoardStep[nSrc] = 0;
	}

	fprintf(out, "\n\n***********************��%2d �غ�********************************\n\n", (nCurrentStep+1)/2);
	fprintf(out, "***********************�������ɣ�%d ������ŷ�**********************\n\n", nFirstLayerMoves);
	for(m=0; m<(unsigned int)nFirstLayerMoves; m++)
	{
		nSrc = (FirstLayerMoves[m] & 0xFF00) >> 8;
		nDst = FirstLayerMoves[m] & 0xFF;

		// Ѱ������֧
		if(PvLine[0] == FirstLayerMoves[m])
		{
			fprintf(out, "*PVLINE=%d***********Nodes******History**************************\n", m+1);
			fprintf(out, "*%2d.  ", m+1);
		}
		else
			fprintf(out, "%3d.  ", m+1);

		//n = m==0 ? FirstLayerMoves[m].key : FirstLayerMoves[m].key-FirstLayerMoves[m-1].key;	// ͳ�Ʒ�֧��Ŀ
		n = FirstLayerMoves[m] >> 16;																// ͳ�ƹ�ֵ
		fprintf(out, "%s = %6d    hs = %6d\n", GetStepName(FirstLayerMoves[m], Board), n, HistoryRecord[FirstLayerMoves[m]&0xFFFF]);
	}
	
	fprintf(out, "\n\n********************������ˣ�%d����ֹ�ŷ�********************************\n\n", nBanMoveNum);
	for(m=0; m<(unsigned int)nBanMoveNum; m++)
	{
		fprintf(out, "%3d. %s\n", m+1, GetStepName( BanMoveList[m], Board ));
	}

	fprintf(out, "\n\n***********************��ʷ��¼********************************\n\n", (nCurrentStep+1)/2);
	
	int MoveStr; 
	for(m=0; m<=(int)nCurrentStep; m++)
	{
		MoveStr = Coord(StepRecords[m]);
		fprintf(out, "%3d. %s  %2d  %2d  %12u\n", m, &MoveStr, (StepRecords[m] & 0xFF0000)>>16, (StepRecords[m] & 0xFF000000)>>24, nZobristBoard[m]);
	}


	// �ر��ļ�
	fclose(out);
}



// ѭ����⣺ͨ���Ƚ���ʷ��¼�е�zobrist��ֵ���жϡ�
// �Ľ�������ʹ��΢��Hash��LoopHash[zobrist & LoopMask] = zobrist  LoopMask=1023=0B1111111111  ����ʡȥ�������е�ѭ���жϡ�
// ����Ϊ˫���������������ӣ������������������ϵ�λ��ѭ�������˶���
// �����й��������棬���ֽ�������ѭ�������������������оٲ��Գ���ʱ���ֵĽ���ѭ�����ͣ�
	// 5.         10101  01010  00100
	// 6.        001010
	// 7.       1001001
	// 8.      00001010
	// 9.     100000001  101000001  010001000 000001000
	//12. 1000000000001  0000001000000
// ��˿��������ָ�����ѭ�������ܳ��֡�ѭ���ĵ�һ�������ǳ����ƶ����Ժ���Ƿǳ����ƶ���
// ����5������Թ���ѭ�����ǳ����ƶ���������Ŀ��4��5ѭ������������ͣ�6��120��ѭ�����ͣ�������沢û�ж��塣
// ѭ����⣬ʵ�����𵽼�֦�����ã����Լ�С�������ķ�֧����������д���������ĳ�����ʱ���޷��˳���
int CSearch::RepetitionDetect(void)
{
	// 100��(50�غ�)��ɱ�ӣ���Ϊ�ﵽ��Ȼ���ţ��ж�Ϊ����
	//if(nNonCapNum >= 120)
	//	return(-120);
	
	unsigned int m, n;
	unsigned int *pBoard = &nZobristBoard[nCurrentStep-1];
	
	for(m=4; m<=nNonCapNum; m++)	
	{
		if( *pBoard == *(pBoard-m) )		// ����ѭ��
		{
			// ͳ��ѭ���г��ֵĽ���������
			CChessMove *pMove = &StepRecords[nCurrentStep-1];
			int nOwnChecks = 0;
			int nOppChecks = 0;
			for(n=0; n<=m; n++)
			{
				if((*(pMove-n)) & 0xFF000000)
				{
					if( 1 & n )
						nOppChecks ++;
					else
						nOwnChecks ++;
				}				
			}

			// �鿴ѭ��������
			// �ҳ�������, ��һ�����ƶ������ӽ���, �ƶ������������ӽ���Ҳ���ڴ��ࡣ
			if( nOwnChecks>=2 && !nOppChecks )			// �� 10101
				return 1;

			// ���ֳ������ҷ�������Ȩ���ҷ�����Ϊ���һ���ҷ��ƶ���
			// ����ƶ���λ�ã���ʱ�Ǳ��ȵģ���ʱ�����Լ���������ѭ������ɶ��ַ��档
			else if( nOppChecks>=2 && !nOwnChecks )		// �� 01010
				return 2;
			
			// ����������糤׽�ȣ����γɽ�������Ϊ���塣
			// �й����������൱���ӣ������赲�Է�ĳ�����ӵ���һ����������Ȼѭ�����ڲ����ɽ������������赲��������ɡ�
			// ������棬�������Ϊ�Է�����Ϊ����ʵ�ִ��㷨�൱���ӡ�
			else
				return -int(m);
		}
	}

	return(0);
}


// ѭ���ķ���ֵ
// ���壺 - WINSCORE * 2	����¼��Hash��
// Ӯ�壺 + WINSCORE * 2	����¼��Hash��
// ���壺��ֵ * ��������    
int CSearch::LoopValue(int Player, int ply, int nLoopStyle)
{
	// �ҷ�ѭ���������ڳ����Է����ʶ��Է�ʤ��������¼��Hash��
	if( nLoopStyle == 2 )
		return (ply-1)-WINSCORE*2;

	// �Է��������ҷ�ʤ��������ʤ���ķ���������¼��Hash��
	else if( nLoopStyle == 1 )
		return WINSCORE*2-(ply-1);

	// ���壺���ع�ֵ���������ӣ����ټ���������
	else // nLoopStyle < 0 
		return int(m_Evalue.Evaluation(Player)*0.9f);

	// �������ӣ�0.9f ����ʱð����׷��Ӯ�壻����ʱ���أ�������ͣ��ƾ�����ʱ�����⣬���ⷸ��
	// �����������ʹ��һ����������delta������Ϊ��������Ϊ����
	// ���ݹ�������ľ��飬����Ϊ0.50�������о�Ϊ0.25�������оֽӽ���0��
}


// �ж�����˼��
int CSearch::Interrupt(void)
{
	if(!Ponder && clock() > nMaxTimer)
		bStopThinking = 1;
	else if(!bBatch) 
	{
		switch(BusyLine(Debug)) 
		{
			case e_CommIsReady:
				fprintf(OutFile, "readyok\n");
				fflush(OutFile);
				break;

			case e_CommPonderHit:
				if(Ponder != 2) 
					Ponder = 0;
				break;

			case e_CommStop:
				bStopThinking = 1;
				break;
		}
	}

	return bStopThinking;
}