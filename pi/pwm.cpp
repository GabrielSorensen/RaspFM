//
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <ctime>
#include <algorithm>
#include "GPIOClass.h"

using namespace std;


void pulse(GPIOClass pin, int cycles);
void wait(double seconds);
clock_t timer;

int main (int argc, char *argv[]) {
	// lets assume that the way to run this is
	// pwm.exe [rising/falling/sine/constant] [time to complete task]
	if (argc != 4) {
		cout << "Usage: pwm [rising/falling/sine/constant] [time to complete task] [top brightness percent]" << endl;
		return -1;
	}
	timer = clock();
	string type = argv[1];
	transform(type.begin(), type.end(), type.begin(), :: tolower);
	double time_to_complete = strtod ( argv[2] );
	double brightness = strtod( argv[3] );
	GPIOClass* out1 = new GPIOClass("4");
	GPIOClass* in2 = new GPIOClass("17");
	string val1, val2;

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

	}
	if (type == "falling") {

	}
	if (type == "sine") {

	}
	if (type == "constant") {
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			Pulse(out1, ????????????????????);
		}
	}
}


void Pulse(GPIOClass pin, int cycles) {
	bool running = true;
	while (running) {
		pin.setval_gpio("1"); // turn the pin on
		Wait(cycles * 1/100); // sleep for number of cycles / 1/100 sec
		pin.setval_gpio("0"); // turn the pin off
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
