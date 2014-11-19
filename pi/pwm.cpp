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

	if (argc != 3) {
		cout << "Usage: pwm [rising/falling/sine/constant] [time to complete task]" << endl;
		return -1;
	}
	timer = clock();
	string type = argv[1];
	transform(type.begin(), type.end(), type.begin(), :: tolower);
	double time_to_complete = strtod (argv[2]);
	GPIOClass* in1 = new GPIOClass("4");
	GPIOClass* in2 = new GPIOClass("17");
	string val1, val2;

	in1->export_gpio();
	in2->export_gpio();

	in1->setdir_gpio("out");
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

	}

	while (1) {
		usleep(500000); // 0.5 sec
		in1->getval(val1);
		in2->getval(val2);
		cout << "Value of pin4: " << val1 << endl;
		cout << "Value of pin17: " << val2 << endl;


	}
}


void pulse(GPIOClass pin, int cycles) {
	bool running = true;
	while (running) {
		pin.setval_gpio("1"); // turn the pin on
			// sleep for number of cycles / 1/100 sec
		Wait(cycles * 1/100);
		running = false;
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
