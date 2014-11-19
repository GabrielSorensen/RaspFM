//
//#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <algorithm>
#include "GPIOClass.h"

using namespace std;

void Pulse(GPIOClass* pin, double cycles);
void Wait(double seconds);
clock_t timer;
double time_to_complete;

int main (int argc, char *argv[]) {
	string type = argv[1];
	transform(type.begin(), type.end(), type.begin(), :: tolower);
	// lets assume that the way to run this is
	// pwm.exe [rising/falling/sine/constant]
	if (argc != 2) {
		cout << "Usage: pwm [rising/falling/sine/constant]" << endl;
		return -1;
	}

	while (time_to_complete <= 0) {
		cout << "Input How Long To Run (in seconds)" << endl;
		cin >> time_to_complete;
	}

	GPIOClass* out1 = new GPIOClass("4");
	GPIOClass* in2 = new GPIOClass("17");

	out1->export_gpio();
	in2->export_gpio();

	out1->setdir_gpio("out");
	in2->setdir_gpio("in");

	cout << "Pins are setup." << endl;
	// avoiding flickering will be at 100hz
	// aka turn on and off 100 times a sec
	// a cycle of 0 is off
	// a cycle of 100 is on

	if (type == "rising") {
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			Pulse(out1, time_to_complete * 100.0);
		}
	}
	if (type == "falling") {

	}
	if (type == "sine") {

	}
	if (type == "constant") {

	}
}

//1 cycle is 1/100th of a second
//100 cycles is 1 sec
void Pulse(GPIOClass* pin, double cycles) {
	bool running = true;
	while (running) {
		pin->setval_gpio("1"); // turn the pin on
		Wait(cycles * 1/100); // sleep for number of cycles / 1/100 sec
		pin->setval_gpio("0"); // turn the pin off
		running = false; // this is unnessesary but could be useful if modified a bit.
	}
	return;
}

void Wait ( double seconds )
{
	clock_t endwait;
	endwait = clock () + seconds * CLOCKS_PER_SEC ;
	while (clock() < endwait) {}
	return;
}
