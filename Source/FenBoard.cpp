////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Դ�ļ���FenBoard.cpp                                                                                   //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� wying                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ��fen��ת��Ϊ������Ϣ                                                                               //
// 2. ��������Ϣת��Ϊfen��                                                                               //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include "FenBoard.h"


static const char PieceChar[14] = { 'k', 'r', 'c', 'h', 'b', 'a', 'p', 'K', 'R', 'C', 'H', 'B', 'A', 'P' };


CFenBoard::CFenBoard(void)
{
	// �趨Ϊ��ʼ���棬������
	strcpy(FenStr, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
}

CFenBoard::~CFenBoard(void)
{
}


// ���������ַ���ת��Ϊ������
// ȱǷ��ֻ��ת����д�ַ�(��ɫ����)��������ɫ���ӣ��ɽ���-32��Сд��Ϊ��д
int CFenBoard::FenToPiece(char fen) 
{
	if( fen>='a' && fen<='z' )
		fen -= 32;

	switch (fen) 
	{
		case 'K':			//King��˧
			return 0;
		case 'R':			//Rook��
			return 1;
		case 'C':			//Cannon��
			return 2;
		case 'N':			//Knight��
		case 'H':			//Horse
			return 3;
		case 'B':			//Bishop��
		case 'E':			//Elephant
			return 4;
		case 'A':			//Advisorʿ
		case 'G':			//Guard
			return 5;		
		default:
			return 6;		//Pawn����
	}
}


char* CFenBoard::BoardToFen(const int *Board, int Player, const unsigned int nNonCapNum, const unsigned int nCurrentStep, unsigned int *StepRecords)
{
	int x,y;
	unsigned int m,n,p=0;

	strcpy(FenStr, "");
	//char *FenStr = "";

	// ��������
	for(y=3; y<13; y++)
	{
		m = 0;												//�ո������
		for(x=3; x<12; x++)
		{
			n = Board[ (y<<4) | x ];
			if( n )
			{
				if(m > 0)									//����ո���
					FenStr[p++] = char(m + '0');
				FenStr[p++] = PieceChar[nPieceType[n]];		//���������ַ�
				m = 0;
			}
			else
				m ++;
		}

		if(m > 0)											//����ո���
			FenStr[p++] = char(m + '0');		
		FenStr[p++] = '/';									//�����зָ���
	}

	// ȥ�����һ��'/'
	FenStr[--p] = '\0';

	// " �ƶ��� - - ��ɱ�Ӱ�غ��� ��ǰ��غ���"
	//strcat(FenStr, Player ? " r " : " b ");
	//strcat(FenStr, itoa(10, FenStr, nNonCapNum));
	//strcat(FenStr, " ");
	//strcat(FenStr, itoa(10, FenStr, nCurrentStep));
	char str[32];
	sprintf(str, " %c - - %u %u", Player?'r':'b', nNonCapNum, nCurrentStep);
	//FenStr += strlen(FenStr);
	strcat(FenStr, str);
	p = (unsigned int)strlen(FenStr);
	
	// Save Moves
	if(nCurrentStep>1)
	{
		strcat(FenStr, " moves");
		p += 6;

		for(m=1; m<nCurrentStep; m++)
		{
			x = (StepRecords[m] & 0xFF00) >> 8;		// ��ʼλ��
			y =  StepRecords[m] & 0xFF;				// ��ֹλ��

			FenStr[p++] = ' ';
			FenStr[p++] = char(x & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(x >> 4 ) + '0';
			FenStr[p++] = char(y & 0xF) -  3 + 'a';
			FenStr[p++] = 12 - char(y >> 4 ) + '0';
		}

		// ����
		FenStr[p] = '\0';
	}

	return FenStr;
}


// ��Fen��ת��Ϊ������Ϣ��Board[256], Piece[48], Player, nNonCapNum, nCurrentStep
// ע�⣺Ϊ�˼ӿ������ٶȣ��˺���δ��������Ϣ�ĺϷ������κμ��飬����Fen�������ǺϷ��ġ�
// ���磺ÿ��������Ŀ����9����������Ŀ��������λ�÷Ƿ��ȵȡ�
int CFenBoard::FenToBoard(int *Board, int *Piece, int &Player, unsigned int &nNonCapNum, unsigned int &nCurrentStep, const char *FenStr)
{
	unsigned int m, n;
	int BlkPiece[7] = { 16, 17, 19, 21, 23, 25, 27 };
	int RedPiece[7] = { 32, 33, 35, 37, 39, 41, 43 };

	// ��������������������	
	for(m=0; m<256; m++)
		Board[m] = 0;
	for(m=0; m<48; m++)
		Piece[m] = 0;

	// ��ȡ����λ����Ϣ��ͬʱ����ת��Ϊ��������Board[256]����������Piece[48]
	int x = 3;
	int y = 3;
	char chess = *FenStr;
	while( chess != ' ')							// ���ķֶα��
	{
		if(*FenStr == '/')							// ���б��
		{
			x = 3;									// ����ʼ
			y ++;									// ��һ��
			if( y >= 13 )
				break;
		}
		else if(chess >= '1' && chess <= '9')		// ���ֱ�ʾ�ո�(������)����Ŀ
		{
			n = chess - '0';						// �������ӵ���Ŀ
			for(m=0; m<n; m++) 
			{
				if(x >= 12)
					break;
				x++;
			}
		} 
		else if (chess >= 'a' && chess <= 'z')		// ��ɫ����
		{
			m = FenToPiece( chess - 32 );			// 'A' - 'a' = -32, Ŀ�ľ��ǽ�Сд�ַ�ת��Ϊ��д�ַ�
			if(x < 12) 
			{
				n = BlkPiece[m];
				Board[ Piece[n] = (y<<4)|x ] = n;
				BlkPiece[m] ++;
			}
			x++;
		}
		else if(chess >= 'A' && chess <= 'Z')		// ��ɫ����
		{
			m = FenToPiece( chess );				// �˺���ֻ��ʶ���д�ַ�
			if(x < 12) 
			{
				n = RedPiece[m];
				Board[ Piece[n] = (y<<4)|x ] = n;
				RedPiece[m] ++;
			}
			x++;
		}
		
		// Next Char
		chess = *(++FenStr);
		if( chess == '\0' )
			return 0;
	}

	// ��ȡ��ǰ�ƶ���Player: b-�ڷ��� !black = white = red  �췽
	if(*(FenStr++) == '\0')
		return 1;
	Player = *(FenStr++) == 'b' ? 0:1;

	// Skip 2 Reserved Keys
	if(*(FenStr++) == '\0')    return 1;		// ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	if(*(FenStr++) == '\0')    return 1;      // '-'
	if(*(FenStr++) == '\0')    return 1;      // ' '
	

	// �����治���ͳ�����ǰ���ŷ�����ȫ����
	// �����֣�����ʹ������Ĳ���	
	nNonCapNum   = 0;
	nCurrentStep = 1;

	return 1;

}