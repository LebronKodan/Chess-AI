////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Դ�ļ���PreMove.cpp                                                                                    //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� wying                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��ʼ�����ӣ�����������ʿ�������ƶ�������α�Ϸ����ŷ���                                              //
// 2. ��ʼ������λ����λ������                                                                            //
//                                                                                                        //
// ע��CPreMove�಻����ʵ�ʵ��ŷ����飬��ʼ����ɺ󣬼����ͷŴ��࣬����������ֻ�����һ��                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreMove.h"


CPreMove::CPreMove()
{
}

CPreMove::~CPreMove()
{
}


// ��˧���ƶ���������˫������
// ��ʼ��Ϊ��˧�ƶ���Ŀ���nDst = *pMove
// ���еķǷ����ƶ����Ϸ��������ƶ���һ���Ϸ����������Լ�������Ӧ�����б�
void CPreMove::InitKingMoves(unsigned char KingMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nKingMoveDir[4] = { -16, -1, 1, 16 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										//�ھŹ���
		{
			pMove = KingMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nKingMoveDir[i];
				if( nCityIndex[n] )
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// ��ʾ�������ƶ�
		}
	}
}


// ��ʼ�������ŷ�Ԥ�������飺
// ��ֵΪ��/�е����λ��x/y*16������Ŀ����ӵĹ�ʽΪ��nDst(y|x) = (nSrc & 0xF0) | x; ���� nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// ��һ���뽫��ʿ���������ı�ʾΪ���ӵľ���λ�÷�ʽ��ͬ
// ���еķǷ����ƶ����Ϸ��������ƶ���һ���Ϸ����������Լ�������Ӧ�����б�
void CPreMove::InitRookMoves(unsigned char xRookMoves[][512][12], unsigned char yRookMoves[][1024][12], unsigned char xRookCapMoves[][512][4], unsigned char yRookCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	// ��ʼ�����ĺ����ƶ�
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xRookMoves[x][m];
			pCapMove = xRookCapMoves[x][m];
			for(n=x-1; n>=3; n--)					// ��������
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// �������ӣ���������
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)					// ��������
			{
				*(pMove++) = unsigned char( n );
				if( m & ( 1 << (n-3) ) )			// �������ӣ���������
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;
		}
	}

	// ��ʼ�����������ƶ�
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yRookMoves[y][m];
			pCapMove = yRookCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// �������ӣ���������
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}
			for(n=y+1; n<13; n++)
			{
				*(pMove++) = unsigned char( n<<4 );
				if( m & ( 1 << (n-3) ) )			// �������ӣ���������
				{
					(*pCapMove++) = *(pMove-1);
					break;
				}
			}
			
			*pMove    = 0;
			*pCapMove = 0;
		}
	}
}


// ��ʼ���ڵ��ŷ�Ԥ�������飺
// ��ֵΪ��/�е����λ��x/y*16������Ŀ����ӵĹ�ʽΪ��nDst(y|x) = (nSrc & 0xF0) | x; ���� nDst(y|x) = y | (nSrc & 0xF);  // x = *pMove; y = *pMove;
// ��һ���뽫��ʿ���������ı�ʾΪ���ӵľ���λ�÷�ʽ��ͬ
// ���еķǷ����ƶ����Ϸ��������ƶ���һ���Ϸ����������Լ�������Ӧ�����б�
void CPreMove::InitCannonMoves(unsigned char xCannonMoves[][512][12], unsigned char yCannonMoves[][1024][12], unsigned char xCannonCapMoves[][512][4], unsigned char yCannonCapMoves[][1024][4])
{
	int m, n, x, y;
	unsigned char *pMove, *pCapMove;

	//��ʼ���ڵĺ����ƶ�
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			pMove    = xCannonMoves[x][m];
			pCapMove = xCannonCapMoves[x][m];
			for(n=x-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// �����ڼ���
				{
					n--;							// ��Խ�ڼ���
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			// �������Ӳ��ƶ�
				{
					*(pMove++) = unsigned char( n );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=x+1; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// �����ڼ���
				{
					n++;							// ��Խ�ڼ���
					break;
				}
				*(pMove++) = unsigned char( n );
			}
			for(; n<12; n++)
			{
				if( m & ( 1 << (n-3) ) )			// �������Ӳ��ƶ�
				{
					*(pMove++) = unsigned char( n );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;	
		}
	}

	//��ʼ���ڵ������ƶ�
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			pMove    = yCannonMoves[y][m];
			pCapMove = yCannonCapMoves[y][m];
			for(n=y-1; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//�����ڼ���
				{
					n--;							//��Խ�ڼ���
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n>=3; n--)
			{
				if( m & ( 1 << (n-3) ) )			//�������Ӳ��ƶ�
				{
					*(pMove++) = unsigned char( n<<4 );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			for(n=y+1; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//�����ڼ���
				{
					n++;							//��Խ�ڼ���
					break;
				}
				*(pMove++) = unsigned char( n<<4 );
			}
			for(; n<13; n++)
			{
				if( m & ( 1 << (n-3) ) )			//�������Ӳ��ƶ�
				{
					*(pMove++) = unsigned char( n<<4 );
					*(pCapMove++) = *(pMove-1);
					break;
				}
			}

			*pMove    = 0;
			*pCapMove = 0;
		}
	}
}


// ��ʼ������ŷ�Ԥ�������飺
// ��ֵ��ʼ��Ϊ���ƶ��ľ���λ�á�
// ���Ϸ��ƶ��������ȵ��ƶ����Լ������ӵ��ƶ��������⡣
void CPreMove::InitKnightMoves(unsigned char KnightMoves[][12])
{
	int i, m, n;
	unsigned char *pMove;
	int nHorseMoveDir[] = { -33, -31, -18, -14, 14, 18, 31, 33 };
	
	for(m=0x33; m<0xCC; m++)
	{
		if( nBoardIndex[m] )
		{
			pMove = KnightMoves[m];
			for(i=0; i<8; i++)
			{	
				n = m + nHorseMoveDir[i];
				if( nBoardIndex[n] )						// ����������	
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}

// ��ʼ������ŷ�Ԥ�������飺
// ��ֵ��ʼ��Ϊ���ƶ��ľ���λ�á�
// ���Ϸ��ƶ��������۵��ƶ����Լ������ӵ��ƶ��������⡣
void CPreMove::InitBishopMoves(unsigned char BishopMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nBishopMoveDir[] = { -34, -30, 30, 34 };
	
	for(m=0x33; m<0xCC; m++)
	{
		if( nBoardIndex[m] )
		{
			pMove = BishopMoves[m];
			for(i=0; i<4; i++)
			{	
				n = m + nBishopMoveDir[i];
				if( nBoardIndex[n] && (m^n)<128 )			// (m^n)<128, ��֤�󲻻����
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;
		}
	}
}


// ��ʼ��ʿ���ŷ�Ԥ�������飺
// ��ֵ��ʼ��Ϊʿ�ƶ��ľ���λ�á�
// ���Ϸ��ƶ����Լ������ӵ��ƶ��������⡣
void CPreMove::InitGuardMoves(unsigned char GuardMoves[][8])
{
	int i, m, n;
	unsigned char *pMove;
	int nGuardMoveDir[] = { -17, -15, 15, 17 };

	for(m=0x36; m<0xC9; m++)
	{
		if( nCityIndex[m] )										// �ھŹ���
		{
			pMove = GuardMoves[m];
			for(i=0; i<4; i++)
			{
				n = m + nGuardMoveDir[i];
				if( nCityIndex[n] )								// �ھŹ���
					*(pMove++)  = unsigned char( n );
			}
			*pMove = 0;		// ��ʾ�������ƶ�
		}
	}
}


// ��ʼ��������ŷ�Ԥ�������飺
// ��ֵ��ʼ��Ϊ�ƶ��ľ���λ�á�
// ���Ϸ��ƶ����Լ������ӵ��ƶ��������⡣
void CPreMove::InitPawnMoves(unsigned char PawnMoves[][256][4])
{
	int i, m, n, Player;
	unsigned char *pMove;
	int PawnMoveDir[2][3] = { {16, -1, 1}, {-16, -1, 1} };							// �ڣ������ң�  �죺������

	for(Player=0; Player<=1; Player++)												//Player=0, ���䣻 Player=1, ���
	{
		for(m=0x33; m<0xCC; m++)
		{
			if( nBoardIndex[m] )
			{
				pMove = PawnMoves[Player][m];
				for(i=0; i<3; i++)
				{
					if( i>0 && ((!Player && m<128) || (Player && m>=128)) )			//δ���ӵı��������һζ�
						break;

					n = m + PawnMoveDir[Player][i];
					if( nBoardIndex[n] )										   //�����̷�Χ��
						*(pMove++)  = unsigned char( n );
				}
				*pMove = 0;
			}
		}
	}	
}


// *****************************************************************************************************************
// *****************************************************************************************************************

// ����λ����λ�����̣����������ƶ��ͷǳ����ƶ�
void CPreMove::InitBitRookMove( unsigned short xBitRookMove[][512], unsigned short yBitRookMove[][1024])
{
	int x, y, m, n, index;

	// ���ĺ����ƶ�
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitRookMove[x][m] = 0;

			// �����ƶ�
			for(n=x-1; n>=3; n--)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// �������ӣ���������
					break;
			}

			// �����ƶ�
			for(n=x+1; n<12; n++)
			{
				index = 1<<(n-3);
				xBitRookMove[x][m] |= index;
				if( m & index )						// �������ӣ���������
					break;
			}
		}
	}

	// ���������ƶ�
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitRookMove[y][m] = 0;

			// �����ƶ�
			for(n=y-1; n>=3; n--)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// �������ӣ���������
					break;
			}

			// �����ƶ�
			for(n=y+1; n<13; n++)
			{
				index = 1<<(n-3);
				yBitRookMove[y][m] |= index;
				if( m & index )						// �������ӣ���������
					break;
			}
		}
	}
}


// �ڵ�λ����λ�����̣����������ƶ��ͷǳ����ƶ�
void CPreMove::InitBitCannonMove( unsigned short xBitCannonMove[][512], unsigned short yBitCannonMove[][1024] )
{
	int x, y, m, n, index;

	// �ڵĺ����ƶ�
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitCannonMove[x][m] = 0;

			// �����ƶ�
			for(n=x-1; n>=3; n--)				// �ǳ����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �����ڼ���
				{
					n--;						// ��Ծ�ܼ���
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n>=3; n--)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}

			// �����ƶ�
			for(n=x+1; n<12; n++)				// �ǳ����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �����ڼ���
				{
					n++;						// ��Ծ�ܼ���
					break;
				}
				xBitCannonMove[x][m] |= index;
			}
			for( ; n<12; n++)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					xBitCannonMove[x][m] |= index;
					break;
				}				
			}
		}
	}

	// �ڵ������ƶ�
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitCannonMove[y][m] = 0;

			// �����ƶ�
			for(n=y-1; n>=3; n--)				// �ǳ����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �����ڼ���
				{
					n--;						// ��Ծ�ܼ���
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n>=3; n--)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					yBitCannonMove[y][m] |= index;
					break;
				}				
			}

			// �����ƶ�
			for(n=y+1; n<13; n++)				// �ǳ����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �����ڼ���
				{
					n++;						// ��Ծ�ܼ���
					break;
				}
				yBitCannonMove[y][m] |= index;
			}
			for( ; n<13; n++)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					yBitCannonMove[y][m] |= index;
					break;
				}				
			}
		}
	}
}

void CPreMove::InitBitSupperCannon( unsigned short xBitSupperCannon[][512], unsigned short yBitSupperCannon[][1024] )
{
	int x, y, m, n, index;

	// �ڵĺ����ƶ�
	for(x=3; x<12; x++)
	{
		for(m=0; m<512; m++)
		{
			xBitSupperCannon[x][m] = 0;

			// �����ƶ�
			index = 0;
			for(n=x-1; n>=3; n--)				// �ǳ����ƶ�
			{
				if( m & (1<<(n-3)) )			// �����ڼ���
				{
					index ++;
					if( index >= 2 )			// ��Ծ�����ڼ���
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}

			// �����ƶ�
			index = 0;
			for(n=x+1; n<12; n++)				// �ǳ����ƶ�
			{
				if( m & (1<<(n-3)) )			// �����ڼ���
				{
					index ++;
					if( index >= 2 )			// ��Ծ�����ڼ���
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<12; n++)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					xBitSupperCannon[x][m] |= index;
					break;
				}				
			}
		}
	}

	// �ڵ������ƶ�
	for(y=3; y<13; y++)
	{
		for(m=0; m<1024; m++)
		{
			yBitSupperCannon[y][m] = 0;

			// �����ƶ�
			index = 0;
			for(n=y-1; n>=3; n--)				// �ǳ����ƶ�
			{
				if( m & (1<<(n-3)) )			// �����ڼ���
				{
					index ++;
					if( index >= 2 )			// ��Ծ�����ڼ���
					{
						n --;
						break;
					}
				}
			}
			for( ; n>=3; n--)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}

			// �����ƶ�
			index = 0;
			for(n=y+1; n<13; n++)				// �ǳ����ƶ�
			{
				if( m & (1<<(n-3)) )			// �����ڼ���
				{
					index ++;
					if( index >= 2 )			// ��Ծ�����ڼ���
					{
						n ++;
						break;
					}
				}
			}
			for( ; n<13; n++)					// �����ƶ�
			{
				index = 1<<(n-3);
				if( m & index )					// �������ӣ���������
				{
					yBitSupperCannon[y][m] |= index;
					break;
				}				
			}
		}
	}
}