#include "monitor.h"
#include <iostream>
#include <stdio.h>
#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <ostream>
#include <fstream>
#include <unistd.h>
using namespace std;


unsigned int NumProducersLow = 2;
unsigned int NumLowMsgs = 5;
unsigned int NumProducersHigh = 2;
unsigned int NumHighMsgs = 5;
unsigned int QueueLength = 5;

class Bufor : public Monitor
{
	public:
		Bufor(unsigned int QueueLength) : QueueLength(QueueLength) {}
		
		void PutMsgLow(string msg)
		{			
				enter();

				if(tempHighMsgs < NumProducersHigh * NumHighMsgs)
				{
					wait(Priority);
				}

				if(Msgs.size() == QueueLength)
				{
					wait(NotFull);
				}
				
				vector<string>::iterator it;
				it = Msgs.begin();
				Msgs.insert(it, msg);
				
				if(Msgs.size() == 1)
				{
					signal(NotEmpty);
				}
					
				leave();
		}	

		void PutMsgHigh(string msg)
		{
				enter();
				
				if(Msgs.size() == QueueLength)
				{					
					wait(NotFull);
				}

				Msgs.push_back(msg);
				tempHighMsgs ++;
				
				if(Msgs.size() == 1)
				{
					signal(NotEmpty);
				}
					
				leave();			
		}
		
		string GetMsgg()
		{
			enter();
			
			if(Msgs.size() == 0)
			{
				wait(NotEmpty);
			}
						
			string msg = Msgs.back();
			Msgs.pop_back();

			if(tempHighMsgs >= NumProducersHigh * NumHighMsgs)
			{
				signal(Priority);
			}
			
			if(Msgs.size() == QueueLength - 1)
			{
				signal(NotFull);
			}
			
			leave();
			
			return msg;
		}
	
	protected:
		
		Condition NotFull;
		Condition NotEmpty;
		Condition Priority;
		
		std::vector<string> Msgs;
		
		const unsigned int QueueLength;
		int tempHighMsgs = 0;
};

Bufor Buff(QueueLength);

void ConsumerP(int interwal)
{
	while(1)
	{	
		usleep(interwal);
		string msg = Buff.GetMsgg();		
		cout << "recieved  [ " << msg << " ]" << endl;
	}
}

void ProducentLowP(int Id)
{
	system("gnome-terminal");
	string terminal = "/dev/pts/" + to_string(Id);
	sleep(1);
	ofstream term(terminal, ios_base::out);
	term << endl;

	for(int i = 0; i < NumLowMsgs; i++)
	{
		sleep(1);
		string msg = "L customer " + to_string(Id) + "- msg: " + to_string(i);
		term << "sent [ " << msg << " ]" << endl;
		Buff.PutMsgLow(msg);
	}
}

void ProducentHighP(int Id)
{
	system("gnome-terminal");
	string terminal = "/dev/pts/" + to_string(Id);
	sleep(1);
	ofstream term(terminal, ios_base::out);
	term << endl;

	for(int i = 0; i < NumHighMsgs; i++)
	{
		sleep(1);
		string msg = "H customer " + to_string(Id) + "- msg: " + to_string(i);
		term << "sent [ " << msg << " ]" << endl;
		Buff.PutMsgHigh(msg);
	}
}

int main(int ArgC, char ** ArgV)
{	
	if(ArgC != 7)
	{
		printf("%s: LowProducers, LowMsgs, HighProducers, HighMsgs, BufferLngth, CnsumerInterwal\n", ArgV[0]);
		return 1;
	}

	int interwal = 500000;

	NumProducersLow = atoi(ArgV[1]);
	NumLowMsgs = atoi(ArgV[2]);
	NumProducersHigh = atoi(ArgV[3]);
	NumHighMsgs = atoi(ArgV[4]);
	QueueLength = atoi(ArgV[5]);
	interwal = atoi(ArgV[6]);
	
	printf(
		"Number of low priority producers: %d\n"
		"Number of low priority messages: %d\n"
		"Number of high priority producers: %d\n"
		"Number of high priority messages: %d\n"
		"Buffer size: %d\n"
		"Reader interwal: %d\n"
		"\n=======================================\n\n"
		,
		NumProducersLow, NumLowMsgs, NumProducersHigh, NumHighMsgs, QueueLength, interwal
	);
		
	thread tL [NumProducersLow];
	for (int i = 1; i <= NumProducersLow; ++i) 
	{
 		tL[i - 1] = thread(ProducentLowP, i);
	}

	thread tH [NumProducersHigh];
	for (int i = NumProducersLow + 1; i <= NumProducersLow + NumProducersHigh; ++i) 
	{
 		tH[i - NumProducersLow - 1] = thread(ProducentHighP, i);
	}
	
	thread ConsumerT(ConsumerP, interwal);




	//cleanup	
	for (int i = 0; i < NumProducersLow; ++i) 
	{
		tL[i].join();
	}

	for (int i = 0; i < NumProducersHigh; ++i) 
	{
		tH[i].join();
	}

	ConsumerT.join();	
	
	
	return 0;
}
