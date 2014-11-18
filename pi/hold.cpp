//
#include <unistd.h>
#include "GPIOClass.h"

using namespace std;


int main () {
	GPIOClass* in1 = new GPIOClass("4");
	GPIOClass* in2 = new GPIOClass("17");
	string val1, val2;

	in1->export_gpio();
	in2->export_gpio();

	in1->setdir_gpio("in");
	in2->setdir_gpio("in");

	cout << "Pins are setup." << endl;


	while (1) {
		usleep(5000000); // 0.5 sec
		in1->getval(val1);
		in2->getval(val2);
		cout << "Value of pin4: " << val1 << endl;
		cout << "Value of pin17: " << val2 << endl;


	}
}
