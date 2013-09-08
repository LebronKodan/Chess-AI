////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ͷ�ļ���HashTable.cpp                                                                                  //
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

#include <stdlib.h>			// �����������rand()������ͷ�ļ�
#include <stdio.h>			// �ļ�����
#include <time.h>			// ��ʼ�������
#include "HashTable.h"
#include "FenBoard.h"



CHashTable::CHashTable(void)
{
	//srand( (unsigned)time( 0 ) );		// ��ʹ��ʱ��ÿ�ζ�������ͬ�ľ���ֵ���Է������
}

CHashTable::~CHashTable(void)
{
}

void CHashTable::ZobristGen(void)
{
	int i, j;

	//ZobristKeyPlayer = Rand32U();

	//((unsigned long *) &ZobristLockPlayer)[0] = Rand32U();
	//((unsigned long *) &ZobristLockPlayer)[1] = Rand32U();

	for (i = 0; i < 14; i ++) 
	{
		for (j = 0; j < 256; j ++) 
		{
			ZobristKeyTable[i][j] = Rand32U();

			((unsigned long *) &ZobristLockTable[i][j])[0] = Rand32U();
			((unsigned long *) &ZobristLockTable[i][j])[1] = Rand32U();
		}
	}
}

//ΪHash������ڴ棺�Ժ�����ڴ��⣬�Ż���С�����ɹ��򷵻�0
unsigned long CHashTable::NewHashTable(unsigned long nHashPower, unsigned long nBookPower)
{
	ZobristGen();

	nHashSize = 1<<(nHashPower-1); 
	nHashMask = nHashSize-1;						// �� & ���� % ����

	pHashList[0]  = new CHashRecord[nHashSize];		// �ڷ���Hash��
	pHashList[1]  = new CHashRecord[nHashSize];		// �췽��Hash��
	
	nMaxBookPos = 1<<nBookPower;
	pBookList = new CBookRecord[nMaxBookPos];

	return nHashSize;
}

// �ͷ��ڴ�
void CHashTable::DeleteHashTable()
{
	// �ͷ�Hash����ڴ�
	delete [] pHashList[0];
	delete [] pHashList[1];

	// �ͷſ��ֿ���ڴ�
	delete [] pBookList;
}

// ���Hash�����ҷ���Hash��ĸ�����
// style==0����ȫ���Hash��¼���������������½����ʱ��ʹ�����������
// style!=0������ȫ���������ʹ��(Hash.depth - 2)�Ĳ��ԣ����������ԭ��
// 1.���غ��ƣ�2ply����غϺ��ֵ��������塣��ǰ������MaxDepth-2��������һ�����������ǽ�Ҫ������ȫ��������֦��
// �������֦�������õġ�������µ��������ԣ��ǿ����������������㡣�ѵ�ǰ��Hash�ڵ����ֵ��2�����ö�Ӧ��һ�غ�
// MaxDepth-2�����������ֵ��2�Ժ���Hash��֮ǰdepth=1��depth-2�ĵĽڵ㽫�������У�����Ȼ�ܹ�Ϊ�µ��������ṩ
// HashMove��Ҳ����ʡһ����ʱ�䡣����������ȸ��ǲ��ԣ�������һ�غ��µ����������������ֵdepth<MaxDepth-2��ǰ��
// ������������֧��ֱ�����У�Ҳ���Ḳ���κ�Hash�ڵ㣻֮���������ֵ�����ӣ������𽥸�����ͬ�����Hash�ڵ㡣
// 2.���������򿪺�̨˼�����ܣ��µ���������ʹ��̨˼��û�����У�Hash���л��۵ķḻ��Ϣͬ���ܹ����á��ڹ涨��
// ʱ���ڣ���������ڴ˻������������
// 3.����ɱ�壬Hash̽��ʱ����ȡ������Ȳ��ԣ���Hash��ʱ����ȡ��ȫ����ķ��������ⷵ��ֵ��׼ȷ�����⡣��ʹɱ��
// �ľ���Ҳ�ܹ����ã�value<-MATEVALUEʱ��value+=2��value>MATEVALUEʱ��value-=2����������Ժ�ڶ�ս����Ӧ�á�1��
// Ϊ����Ӧ��ͬ��������ʽ�����Ӱ�ȫ�ԣ����ñ��յĴ�ʩ---��ȫ�����
// 4.���Hash���Ǹ�����ķ�ʱ��Ĺ���������ѡ���������BestMove����У�������̨���񡱡�
// 5.���Ա�����������������ܹ���ʡ10��15%������ʱ�䡣
float CHashTable::ClearHashTable(int style)
{
	CHashRecord *pHash0 = pHashList[0];
	CHashRecord *pHash1 = pHashList[1];

	CHashRecord *pHashLimit = pHashList[0] + nHashSize;

	nHashCovers = 0;
	if( style )
	{
		while( pHash0 < pHashLimit )
		{
			// ����Hash��ĸ�����
			//if(pHash0->flag)
			//	nHashCovers++;
			//if(pHash1->flag)
			//	nHashCovers++;

			// ����ɱ�壬HashFlag��0����ʾ��ȫ���
			if( (pHash0->value) < -MATEVALUE || (pHash0->value) > MATEVALUE )
				pHash0->flag = 0;
			if( (pHash1->value) < -MATEVALUE || (pHash1->value) > MATEVALUE )
				pHash1->flag = 0;

			// ���ֵ�� 2����һ�غϻ���������
			(pHash0 ++)->depth -= 2;
			(pHash1 ++)->depth -= 2;
		}
	}
	else
	{
		while( pHash0 < pHashLimit )
		{
			(pHash0 ++)->flag = 0;
			(pHash1 ++)->flag = 0;
		}
	}

	return nHashCovers/(nHashSize*2.0f);
}

unsigned long CHashTable::Rand32U()
{
	//return rand() ^ ((long)rand() << 15) ^ ((long)rand() << 30);
	//�������ʹ��������������0<=rand()<=32767, ֻ�ܲ���0��max(unsigned long)/2֮������������Ȼ�ܲ����ȣ�������һ���ĳ�ͻ����

	return ((unsigned long)(rand()+rand())<<16) ^ (unsigned long)(rand()+rand());	//�Ľ���Ӧ���Ǿ��ȷֲ���
}


//�������ӵ�λ����Ϣ����ʼ��ZobristKey��ZobristLock
//����һ���µ���ֻ����������ʱ��Ӧ�����ô˺���
void CHashTable::InitZobristPiecesOnBoard(int *Piece)
{
	int m, n, chess;
	
	ZobristKey  = 0;
	ZobristLock = 0;

	for(n=16; n<48; n++)
	{
		m = Piece[n];
		if( m )
		{
			chess = nPieceType[n];

			ZobristKey  ^= ZobristKeyTable[chess][m];
			ZobristLock ^= ZobristLockTable[chess][m];
		}
	}
}

// ̽��Hash�������ɹ�����"INT_MIN"����������ͻ���򷵻�HashMove����־Ϊ"INT_MAX"
// ɢ�з���
// 1. ���ɢ�з�����h(x) = ZobristKey % MaxHashZize =ZobristKey & (MaxHashSize-1)  M = MaxHashSize = 1<<k = 1024*1204, k=20 
//    ��2���ݱ�ʾ�����԰ѳ������λ(��)���㣬�ٶȺܿ졣����������ɢ�б�Ĵ�ɣ�2^k-1=11111111111111111111(B),�����õ���ZobristKey��20λ��Ϣ����12λ��Ϣ�˷�
//    ���ڲ�����ZobristKey�Ѿ�ʱ���ȷֲ�����������������ɢ�з��Ṥ���úܺá�Ψһ��ȱ���Ǵ�����ż���󣬼�h(x) �� ZobristKey����ż��ͬ��
// 2. ƽ��ȡ��ɢ�з�����h(x)=[M/W((x*x)%W)] = (x*x)>>(w-k) = (x*x)>>(32-20) = (x*x)>>12  ǰ����x<sqrt(W/M)��β����x=n*sqrt(W)�Ĺؼ��ֻ��ͻ
//    ��x*x���ƶ�12λ��ʣ������20λ���ܹ�����0��M-1֮�������
// 3. Fibonacci(쳲���������)���ɢ�з�, h(x) = (x*2654435769)>>(32-k)  2654435769�Ǹ���������2654435769/2^32 = 0.618 �ƽ�ָ�㣬��ʹ�����ļ�ֵ���ܾ��ȷֲ�
//    2^16  40503
//    2^32  2654435769					������ 340573321   a%W
//    2^64  11400714819323198485
// CHashRecord *pHashIndex = pHashList[player] + ((ZobristKey * 2654435769)>>(32 - nHashPower));		// Fibonacci(쳲���������)���ɢ�з�
// �����飬���ɢ�з�����ʱ����졣Fibonacci��Ҫ�漰�����ӵĳ˷���λ�����㣬�ʶ��ٶȷ�������򵥵ķ�����
int CHashTable::ProbeHash(CChessMove &hashmove, int alpha, int beta, int nDepth, int ply, int player)
{
	CHashRecord HashIndex = pHashList[player][ZobristKey & nHashMask];								//�ҵ���ǰ����Zobrist��Ӧ��Hash��ĵ�ַ
	
	if( HashIndex.zobristlock == ZobristLock )				//�Ѿ�����Hashֵ, ��ͬһ���
	{	
		if( HashIndex.flag & HashExist )
		{
			// �������������Hashֵ����ʹ������������Ȼ�����ҵ����·�ߣ�Hash�����к�Ĳ���ʧȥ׼ȷ�ԡ�
			// ���Ǿ��뽫���ķ�������ȷ�ģ���Ϊ�ɹ۵��ǣ��о�ʱ�����ٶȿ�30��40%.
			// �������ŷ�������޹أ��������Ĳ㽫����ֻҪ������ͬ������Ե���Hash���е�ֵ��
			bool bMated = false;
			if( HashIndex.value > MATEVALUE )				// ��ʤ����
			{
				bMated = true;
				HashIndex.value -= ply;						// ��ȥ��ǰ�Ļغ�������score = WINSCORE-Hash(ply)-ply, ��Ҫ����Ĳ������ܻ�ʤ������ܹ��õ���̵Ľ���·��
			}
			else if( HashIndex.value < -MATEVALUE )			// ʧ�ܾ���
			{
				bMated = true;
				HashIndex.value += ply;						// ���ϵ�ǰ�Ļغ�������score = Hash(ply)+ply-WINSCORE, ��˵��Ի���ȡ������Ļغϣ���ǿ�ֿ����ȴ����������߳�¶�Ŷ��ӱܽ���
			}

			if( HashIndex.depth >= nDepth || bMated )		// �������ڵ��ֵ����ȷ��������ȣ��������棬������ȣ�ֻҪ����ȷ���ؾ���ʤ�ܵĻغ�������
			{
				//�����ȵ���ʱ��������⣬����ÿ�ζ����Hash��
				//��Ϊ����������ȸ��ǣ��ѿ������ڵ��ֵ�����ȷ�ġ�
				//������ǰ��ǳ�������Ĺ�ֵ�ǲ���ȷ�ģ��������ʱӦ��������Щ���ݡ�
				if(HashIndex.flag & HashBeta)			
				{
					if (HashIndex.value >= beta)
					{
						nHashBeta++;					// 95-98%
						return HashIndex.value;
					}
				}
				else if(HashIndex.flag & HashAlpha)		// Ҳ�ܼ���һ�����������Ӷ���ʡʱ��
				{
					if (HashIndex.value <= alpha)	
					{
						nHashAlpha++;					// 2-5%
						return HashIndex.value;
					}
				}
				else if(HashIndex.flag & HashPv)		// 
				{
					nHashExact++;						// 0.1-1.0%
					return HashIndex.value;
				}

				// ʣ�º������ķ�֧����һЩ�ո�չ���ķ�֧(���ٷ��֣�����Ŀ��HashAlpha���)��beta=+32767, ����ĵ�̽���ʧ��
			}
		}

		// ����NullMove�ƶ���Ҷ�ڵ㣬û��HashMove��
		if( !HashIndex.move )
			return(INT_MIN);

		// Hash��δ���У���������ͬ������HashMove��
		// Ҳ��������һ�غ������������ƶ������Hash��ʱ��ֻ�ǰ�flag=0��HashMove��Ȼ�����ڼ�¼�С�
		// ���鷢�֣��Ժ�������ͬһ���棬ʱ���Խ��Խ�٣���������ƽ�⡣
		if( HashIndex.depth > 0 )
		{
			nCollision ++;
			hashmove = HashIndex.move;
			return INT_MAX;		// ��ʾ��ͬ����ĳ¾�Hashֵ
		}
	}

	return(INT_MIN);		// ������ͬ�����
}

void CHashTable::RecordHash(const CChessMove hashmove, int score, int nFlag, int nDepth, int ply, int player)
{
	//�ҵ���ǰ����Zobrist��Ӧ��Hash��ĵ�ַ
	CHashRecord *pHashIndex = pHashList[player] + (ZobristKey & nHashMask);

	// ���ˣ��������ԭ��
	if((pHashIndex->flag & HashExist) && pHashIndex->depth > nDepth)
		return;

	// ����ѭ�����档ѭ�����治��д��Hash��
	//     ����ĳ��·�ߴ���ѭ������ô���������ص�ѭ���ĳ�ʼ����ʱ��·�ߵ�ÿ�����涼��������Hash������Ӯ��������塣
	// �ٴ�����ʱ������������·���ϵ���֣���û����������ѭ��·�ߣ������ϾͲ�����ѭ������������Hash���з��֣�������
	// ·�߻�Ӯ��������塣��������壬������ᱻ��ǰ�ľ����ŵ���������̤������·��ʵ���ϣ��������ǿ��Եģ������������Σ�
	// ��Ϊ���ξ͹�����ѭ����
	//    �о�ʱ����������ż����ʡ���Щ���棬����������·�߻�Ӯ��ʤ��������ʱ������ǿ������
	//    �����漰��ѭ���ķ���ֵ���⡣ʵ���ϣ����ڲ��Ǻ���ľ��棬ѭ�����ŷ�������������ǿ��Ӧ�Է�ʽ����ʹ���������
	// ��ʹ��ѭ��Ӧ�Բſ���ά�ֵø��á��������ԭ��ѭ��������Է���С��-WINSCORE���ߴ���+WINSCORE��ֵ�����ص�������
	// ����һ�㣬RecordHash()�����Ϳ����������ֵ���жϳ�����ѭ���ľ��档���ǲ���¼��Hash���У�ֻ�Ǽ򵥵ķ��ء��Ժ�����
	// �����������Ӱ�졣
	if(score<-WINSCORE || score>+WINSCORE)
		return;

	// �������������Hashֵ�����Ի����̵Ľ���·�ߡ�
	if( score > MATEVALUE )
		score += ply;
	else if( score < -MATEVALUE )
		score -= ply;

	// ��¼Hashֵ
	pHashIndex->zobristlock = ZobristLock;
	pHashIndex->flag    	= (unsigned char)nFlag;
	pHashIndex->depth   	= (char)nDepth;
	pHashIndex->value       = (short)score;
	pHashIndex->move        = (unsigned short)hashmove;		//�������16λ����Ϣ
}

// ���ֿ��ʽ��
// �ƶ� Ȩ�� ���棬��b2e2 5895 rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR w - - 0 1
int CHashTable::LoadBook(const char *BookFile) 
{
	FILE *FilePtr;
	char *LinePtr;
	char LineStr[256];
	CChessMove  BookMove, temp;
	CHashRecord TempHash;
	CFenBoard fen;

	int Board[256], Piece[48];
	int Player;
	unsigned int nNonCapNum;
	unsigned int nCurrentStep;

	// �����������ʱ������������ɵ���֣���������ʱ�����ָ���
	// ��Ϊ�����ֿ�ʱ����ı�Hash���е�ǰ�ľ��棬�������治�ָܻ�ԭ���ľ��档
	unsigned long    Old_ZobristKey = ZobristKey;
	unsigned __int64 Old_ZobristLock = ZobristLock;

	if(!BookFile)
		return 0;

	// ��ֻ���ı���ʽ�򿪿��ֿ�
	if((FilePtr = fopen(BookFile, "rt"))==0)
		return 0;										// ���ɹ�������0

	nBookPosNum = 0;
	LineStr[254] = LineStr[255] = '\0';
	
	// �ӿ��ֿ��ж�ȡ254���ַ�
	while(fgets(LineStr, 254, FilePtr) != 0)
	{
		// �ƶ�
		LinePtr = LineStr;
		BookMove = Move(*(long *) LinePtr);				// ���ƶ����ַ���ת��Ϊ�����ͣ�BookMove��16λ��Ч

		if( BookMove )
		{
			// Ȩ��
			LinePtr += 5;								// �����ŷ���4���ַ���1���ո�
			temp = 0;
			while(*LinePtr >= '0' && *LinePtr <= '9')
			{
				temp *= 10;
				temp += *LinePtr - '0';
				LinePtr ++;
			}
			
			BookMove |= temp<<16;						// �����ƶ���Ȩֵ(�÷�)������Book�ĸ�16λ����16λ���ƶ����ŷ�

			// ����
			LinePtr ++;														// �����ո�
			fen.FenToBoard(Board, Piece, Player, nNonCapNum, nCurrentStep, LinePtr);	// ��LinePtr�ַ���ת��Ϊ��������
			InitZobristPiecesOnBoard( Piece );								// ���������ϵ����Ӽ���ZobristKey��ZobristLock

			if( Board[(BookMove & 0xFF00)>>8] )								// ��λ�������Ӵ���
			{
				TempHash = pHashList[Player][ZobristKey & nHashMask];
				if(TempHash.flag)											// Hash����������
				{
					if(TempHash.zobristlock == ZobristLock)					// ��������ͬ�ľ���
					{
						if(TempHash.flag & BookUnique)						// ���ֿ�����Ψһ�ŷ�
						{
							if(nBookPosNum < nMaxBookPos)					// û�г������ֿ�ķ�Χ
							{
								TempHash.zobristlock = ZobristLock;
								TempHash.flag = BookMulti;
								TempHash.value = (short)nBookPosNum;

								pBookList[nBookPosNum].MoveNum = 2;
								pBookList[nBookPosNum].MoveList[0] = (TempHash.value<<16) | TempHash.move;
								pBookList[nBookPosNum].MoveList[1] = BookMove;
								
								nBookPosNum ++;
								pHashList[Player][ZobristKey & nHashMask] = TempHash;
							}
						} 
						else															// ���ֿ������������ϵı��� 
						{
							if(pBookList[TempHash.value].MoveNum < MaxBookMove)
							{
								pBookList[TempHash.value].MoveList[pBookList[TempHash.value].MoveNum] = BookMove;
								pBookList[TempHash.value].MoveNum ++;
							}
						}
					}
				} 
				else					// Hash����û�е�ǰ�ľ��棬д��BestMove
				{
					TempHash.zobristlock = ZobristLock;
					TempHash.flag = BookUnique;
					TempHash.move = unsigned short(BookMove & 0xFFFF);
					TempHash.value = unsigned short(BookMove >> 16);
					pHashList[Player][ZobristKey & nHashMask] = TempHash;
				}
			}
		}
	}
	fclose(FilePtr);

	// �ָ���֣�������Խ���ԭ������ּ������ġ�
	ZobristKey = Old_ZobristKey;
	ZobristLock = Old_ZobristLock;

	return 1;
}



int CHashTable::ProbeOpeningBook(CChessMove &BookMove, int Player)
{
	CHashRecord TempHash = pHashList[Player][ZobristKey & nHashMask];
	
	if((TempHash.flag & BookExist) && TempHash.zobristlock == ZobristLock)
	{
		if(TempHash.flag & BookUnique)			// ���ֿ��д���Ψһ���ŷ������С�
		{
			BookMove = (TempHash.value<<16) | TempHash.move;
			return INT_MAX;
		} 
		else
		{			
			CBookRecord *pBookIndex = pBookList + TempHash.value;

			int m, ThisValue = 0;
			for(m=0; m<pBookIndex->MoveNum; m++)
				ThisValue += (pBookIndex->MoveList[m] & 0xFFFF0000) >> 16;

			if(ThisValue) 
			{
				ThisValue = Rand32U() % ThisValue;
				for(m=0; m<pBookIndex->MoveNum; m++)
				{
					ThisValue -= (pBookIndex->MoveList[m] & 0xFFFF0000) >> 16;

					if( ThisValue < 0 ) 
						break;
				}

				BookMove = pBookIndex->MoveList[m];
				return INT_MAX;
			}
		}
	}

	return 0;
}