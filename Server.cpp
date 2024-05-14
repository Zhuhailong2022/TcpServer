#include<iostream>
#include "Task.hpp"
#include "MainThread.hpp"
#include "WorkerThread.hpp"
using namespace std;

int main()
{
	MainThread* MT = new MainThread;
	MT->run();
	while (true)
	{

	}
	return 0;
}
