////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Դ�ļ���main.cpp                                                                                 //
// *******************************************************************************************************//
// �й�����ͨ������----�򵥲�������ʾ����֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����ucci) //
// ���ߣ� IwfWcf                                                                                        //
// *******************************************************************************************************//
// ���ܣ�                                                                                                 //
// 1. ����̨Ӧ�ó������ڵ�                                                                              //
// 2. ͨ��ucciЭ����������֮�����ͨѶ                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string.h>
#include "ucci.h"
#include "FenBoard.h"
#include "Search.h"

int main(int argc, char* argv[])
{
	int n;
	const char *BoolValue[2] = { "false", "true" };
	const char *ChessStyle[3] = { "solid", "normal", "risky" };
	char *BackSlashPtr;
	char BookFile[1024];
	CommEnum IdleComm;
	CommDetail Command;
	int ThisMove;
	
	printf("*******************************IwfWcf's Chess AI*********************************\n");
	printf("** ���ߣ�IwfWcf                                                             **\n");
	printf("** ֧�֡��й�����ͨ������Э�顷(Universal Chinese Chess Protocol�����UCCI) **\n");
	printf("******************************************************************************\n");
	printf("�����ucciָ��......\n");

	// �������"ucci"ָ��
	if(BootLine() == e_CommUcci)
	{
		// Ѱ���������ڵ�Ŀ¼argv[0]�����Ұ�"BOOK.DAT"Ĭ��Ϊȱʡ�Ŀ��ֿ⿪�ֿ�
		BackSlashPtr = strrchr(argv[0], '\\');
		if (BackSlashPtr == 0) 
			strcpy(BookFile, "BOOK.DAT");
		else
		{
			strncpy(BookFile, argv[0], BackSlashPtr + 1 - argv[0]);
			strcpy(BookFile + (BackSlashPtr + 1 - argv[0]), "BOOK.DAT");
		}

		// ����CSearch�࣬���캯����ʼ��һЩ��ز���
		//a.��ʼ���ŷ�Ԥ��������
		//b.��ʼ��Hash������21+1=22��Hash��64M
		//c.�����ʷ������
		CSearch ThisSearch;

		// ��ʾ��������ơ��汾�����ߺ�ʹ����
		printf("\n");
		printf("id name IwfWcf's Chess AI\n");
		fflush(stdout);
		printf("id copyright ��Ȩ����(C) 2005-2012\n");
		fflush(stdout);
		printf("id author IwfWcf\n");
		fflush(stdout);
		printf("id user δ֪\n\n");
		fflush(stdout);

		// ��ʾ����ucciָ��ķ�����Ϣ����ʾ������֧�ֵ�ѡ��
		// option batch %d
		printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
		fflush(stdout);

		// option debug �����������ϸ��������Ϣ�����������ĵ���ģʽ��
		printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
		fflush(stdout);

		// ָ�����ֿ��ļ������ƣ���ָ��������ֿ��ļ����÷ֺš�;���������粻������ʹ�ÿ��ֿ⣬���԰�ֵ��ɿ�
		ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
		if(ThisSearch.bUseOpeningBook)
			printf("option bookfiles type string default %s\n", BookFile);
		else
			printf("option bookfiles type string default %s\n", 0);
		fflush(stdout);

		// �оֿ�����
		printf("option egtbpaths type string default null\n");
		fflush(stdout);

		// ��ʾHash��Ĵ�С
		printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);
		fflush(stdout);

		// ������߳���
		printf("option threads type spin default %d\n", 0);
		fflush(stdout);

		// ����ﵽ��Ȼ���ŵİ�غ���
		printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
		fflush(stdout);

		// ���
		printf("option repetition type spin default %d 1999��桶�й����徺������\n", e_RepetitionChineseRule);
		fflush(stdout);

		// ���Ųü��Ƿ��
		printf("option pruning type check %d\n", ThisSearch);
		fflush(stdout);

		// ��ֵ������ʹ�����
		printf("option knowledge type check %d\n", ThisSearch);
		fflush(stdout);

		// ָ��ѡ����ϵ����ͨ����0,1,2,3�ĸ����𡣸���ֵ�����Ӽ�һ����Χ�ڵ��������������ÿ���߳�����ͬ���塣
		printf("option selectivity type spin min 0 max 3 default %d\n", ThisSearch.nSelectivity);
		fflush(stdout);

		// ָ������ķ��ͨ����solid(����)��normal(����)��risky(ð��)����
		printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[ThisSearch.nStyle]);
		fflush(stdout);		

		// copyprotection ��ʾ��Ȩ�����Ϣ(���ڼ�飬��Ȩ��Ϣ��ȷ���Ȩ��Ϣ����)�� 
		printf("copyprotection ok\n\n");
		fflush(stdout);

		// ucciok ����ucciָ������һ��������Ϣ����ʾ�����Ѿ�������UCCIЭ��ͨѶ��״̬��
		printf("ucciok\n\n");
		fflush(stdout);


		// �趨��׼����ͳ�ʼ����
		ThisSearch.OutFile = stdout;	// ��׼���
		ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, "rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1");
		ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);
		printf("position fen rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR r - - 0 1\n\n");
		fflush(stdout);
		

		// ��ʼ����ִ��UCCI����
		do 
		{
			IdleComm = IdleLine(Command, ThisSearch.Debug);
			switch (IdleComm) 
			{
				// isready ��������Ƿ��ھ���״̬���䷴����Ϣ����readyok����ָ����������������ġ�ָ����ջ��������Ƿ�����������ָ�
				// readyok �������洦�ھ���״̬(���ɽ���ָ���״̬)���������洦�ڿ���״̬����˼��״̬��
				case e_CommIsReady:
					printf("readyok\n");
					fflush(stdout);
					break;

				// stop �ж������˼����ǿ�Ƴ��š���̨˼��û������ʱ�����ø�ָ������ֹ˼����Ȼ������������档
				case e_CommStop:
					ThisSearch.bStopThinking = 1;
					//printf("nobestmove\n");
					printf("score 0\n");
					fflush(stdout);
					break;

				// position fen ���á��������̡��ľ��棬��fen��ָ��FEN��ʽ����moves�������������߹����ŷ�
				case e_CommPosition:
					// �����洫����Fen��ת��Ϊ�����Ϣ
					ThisSearch.fen.FenToBoard(Board, Piece, ThisSearch.Player, ThisSearch.nNonCapNum, ThisSearch.nCurrentStep, Command.Position.FenStr);
					ThisSearch.InitBitBoard(ThisSearch.Player, ThisSearch.nCurrentStep);

					// �������ߵ���ǰ����Ҫ��Ϊ�˸����ŷ���¼������ѭ����⡣
					for(n=0; n<Command.Position.MoveNum; n++)
					{
						ThisMove = Move(Command.Position.CoordList[n]);
						if( !ThisMove )
							break;

						ThisSearch.MovePiece( ThisMove );
						ThisSearch.StepRecords[ThisSearch.nCurrentStep-1] |= ThisSearch.Checking(ThisSearch.Player) << 24;
					}

					ThisSearch.nBanMoveNum = 0;
					break;

				// banmoves Ϊ��ǰ�������ý��֣��Խ�������޷�����ĳ������⡣�����ֳ������ʱ�����ֿ��Բٿؽ��������淢������ָ�
				case e_CommBanMoves:
					ThisSearch.nBanMoveNum = Command.BanMoves.MoveNum;
					for(n=0; n<Command.BanMoves.MoveNum; n++)
						ThisSearch.BanMoveList[n] = Move(Command.BanMoves.CoordList[n]);
					break;

				// setoption ����������ֲ���
				case e_CommSetOption:
					switch(Command.Option.Type) 
					{
						// setoption batch %d
						case e_OptionBatch:
							ThisSearch.bBatch = (Command.Option.Value.Check == e_CheckTrue);
							printf("option batch type check default %s\n", BoolValue[ThisSearch.bBatch]);
							fflush(stdout);
							break;

						// setoption debug %d �����������ϸ��������Ϣ�����������ĵ���ģʽ��
						case e_OptionDebug:
							ThisSearch.Debug = (Command.Option.Value.Check == e_CheckTrue);
							printf("option debug type check default %s\n", BoolValue[ThisSearch.Debug]);
							fflush(stdout);
							break;

						// setoption bookfiles %s  ָ�����ֿ��ļ������ƣ���ָ��������ֿ��ļ����÷ֺš�;���������粻������ʹ�ÿ��ֿ⣬���԰�ֵ��ɿ�
						case e_OptionBookFiles:
							strcpy(BookFile, Command.Option.Value.String);
							printf("option bookfiles type string default %s\n", BookFile);
							fflush(stdout);
							break;

						// setoption egtbpaths %s  ָ���оֿ��ļ������ƣ���ָ������оֿ�·�����÷ֺš�;���������粻������ʹ�òоֿ⣬���԰�ֵ��ɿ�
						case e_OptionEgtbPaths:
							// ����Ŀǰ��֧�ֿ��ֿ�
							printf("option egtbpaths type string default null\n");
							fflush(stdout);
							break;

						// setoption hashsize %d  ��MBΪ��λ�涨Hash��Ĵ�С��-1��ʾ�������Զ�����Hash��1��1024MB
						// �󱤽����и�Bug��ÿ����������ʱ���������Ӧ�ڿ��ֿ��ǰ��
						case e_OptionHashSize:
							// -1MB(�Զ�), 0MB(�Զ�), 1MB(16), 2MB(17), 4MB(18), 8MB(19), 16MB(20), 32MB(21), 64MB(22), 128MB(23), 256MB(24), 512MB(25), 1024MB(26)
							if( Command.Option.Value.Spin <= 0)
								n = 22;		// ȱʡ����£������Զ�����(1<<22)*16=64MB�������������˫����һ�롣
							else
							{
								n = 15;											// 0.5 MB = 512 KB �Դ�Ϊ����
								while( Command.Option.Value.Spin > 0 )
								{
									Command.Option.Value.Spin >>= 1;			// ÿ�γ���2��ֱ��Ϊ0
									n ++;
								}
							}								

							// Ӧ�����ڴ�����ƣ������Զ�����ʱ��Hash���СΪ�����ڴ��1/2��
							ThisSearch.m_Hash.DeleteHashTable();					// ����ʹ��delete������ɵ�Hash��
							ThisSearch.m_Hash.NewHashTable(n > 26 ? 26 : n, 12);	// Ϊ��������µ�Hash��
							printf("option hashsize type spin default %d MB\n", ThisSearch.m_Hash.nHashSize*2*sizeof(CHashRecord)/1024/1024);	// ��ʾʵ�ʷ����Hash���С����λ��MB
							fflush(stdout);

							ThisSearch.m_Hash.ClearHashTable();
							ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
							break;

						// setoption threads %d	      ������߳�����Ϊ�ദ���������������
						case e_OptionThreads:
							// ThisSearch.nThreads = Command.Option.Value.Spin;		// 0(auto),1,2,4,8,16,32
							printf("option drawmoves type spin default %d\n", 0);
							fflush(stdout);
							break;

						// setoption drawmoves %d	  �ﵽ��Ȼ���ŵĻغ���:50,60,70,80,90,100�����Ѿ��Զ�ת��Ϊ��غ���
						case e_OptionDrawMoves:
							ThisSearch.NaturalBouts = Command.Option.Value.Spin;
							printf("option drawmoves type spin default %d\n", ThisSearch.NaturalBouts);
							fflush(stdout);
							break;

						// setoption repetition %d	  ����ѭ������棬Ŀǰֻ֧�֡��й��������1999��
						case e_OptionRepetition:
							// ThisSearch.nRepetitionStyle = Command.Option.Value.Repetition;
							// e_RepetitionAlwaysDraw  ��������
							// e_RepetitionCheckBan    ��ֹ����
							// e_RepetitionAsianRule   ���޹���
							// e_RepetitionChineseRule �й�����ȱʡ��
							printf("option repetition type spin default %d", e_RepetitionChineseRule);
							printf("  ����Ŀǰ֧��1999��桶�й����徺������\n");
							fflush(stdout);
							break;

						// setoption pruning %d����������ǰ�ü����Ƿ��
						case e_OptionPruning:
							ThisSearch.bPruning = Command.Option.Value.Scale;
							printf("option pruning type check %d\n", ThisSearch);
							fflush(stdout);
							break;

						// setoption knowledge %d����ֵ������ʹ��
						case e_OptionKnowledge:
							ThisSearch.bKnowledge = Command.Option.Value.Scale;
							printf("option knowledge type check %d\n", ThisSearch);
							fflush(stdout);
							break;

						// setoption selectivity %d  ָ��ѡ����ϵ����ͨ����0,1,2,3�ĸ�����
						case e_OptionSelectivity:
							switch (Command.Option.Value.Scale)
							{
								case e_ScaleNone:
									ThisSearch.SelectMask = 0;
									break;
								case e_ScaleSmall:
									ThisSearch.SelectMask = 1;
									break;
								case e_ScaleMedium:
									ThisSearch.SelectMask = 3;
									break;
								case e_ScaleLarge:
									ThisSearch.SelectMask = 7;
									break;
								default:
									ThisSearch.SelectMask = 0;
									break;
							}
							printf("option selectivity type spin min 0 max 3 default %d\n", ThisSearch.SelectMask);
							fflush(stdout);
							break;

						// setoption style %d  ָ������ķ��ͨ����solid(����)��normal(����)��risky(ð��)����
						case e_OptionStyle:
							ThisSearch.nStyle = Command.Option.Value.Style;
							printf("option style type combo var solid var normal var risky default %s\n", ChessStyle[Command.Option.Value.Style]);
							fflush(stdout);
							break;						

						// setoption loadbook  UCCI����ElephantBoard��ÿ���½����ʱ���ᷢ������ָ��
						case e_OptionLoadBook:
							ThisSearch.m_Hash.ClearHashTable();
							ThisSearch.bUseOpeningBook = ThisSearch.m_Hash.LoadBook(BookFile);
							
							if(ThisSearch.bUseOpeningBook)
								printf("option loadbook succeed. %s\n", BookFile);		// �ɹ�
							else
								printf("option loadbook failed! %s\n", "Not found file BOOK.DAT");				// û�п��ֿ�
							fflush(stdout);
							printf("\n\n");
							fflush(stdout);
							break;

						default:
							break;
					}
					break;

				// Prepare timer strategy according to "go depth %d" or "go ponder depth %d" command
				case e_CommGo:
				case e_CommGoPonder:
					switch (Command.Search.Mode)
					{
						// �̶����
						case e_TimeDepth:
							ThisSearch.Ponder = 2;
							ThisSearch.MainSearch(Command.Search.DepthTime.Depth);
							break;

						// ʱ���ƣ� ����ʱ�� = ʣ��ʱ�� / Ҫ�ߵĲ���
						case e_TimeMove:							
							ThisSearch.Ponder = (IdleComm == e_CommGoPonder ? 1 : 0);
							printf("%d\n", Command.Search.TimeMode.MovesToGo);
							ThisSearch.MainSearch(127, Command.Search.DepthTime.Time * 1000 / Command.Search.TimeMode.MovesToGo, Command.Search.DepthTime.Time * 1000);
							break;

						// ��ʱ�ƣ� ����ʱ�� = ÿ�����ӵ�ʱ�� + ʣ��ʱ�� / 20 (��������ֻ���20���ڽ���)
						case e_TimeInc:
							ThisSearch.Ponder = (IdleComm == e_CommGoPonder ? 1 : 0);
							ThisSearch.MainSearch(127, (Command.Search.DepthTime.Time + Command.Search.TimeMode.Increment * 20) * 1000 / 20, Command.Search.DepthTime.Time * 1000);
							break;

						default:
							break;
					}
					break;
			}
		} while (IdleComm != e_CommQuit);

		printf("bye\n");
		fflush(stdout);
	}

	return 0;
}

