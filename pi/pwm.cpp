//
//#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cmath>
#include "GPIOClass.h"

using namespace std;

void Pulse(GPIOClass* pin, double cycles);
void Wait(double seconds);
void morseCoder(GPIOClass* pin1, string s);
void morseBlink(GPIOClass* pin1, char c);
void morseReciever(GPIOClass* pin);

clock_t timer;
double time_to_complete;
double resolution = 50;

#define PI 4*atan(1)

int main (int argc, char *argv[]) {
	string type = argv[1];
	transform(type.begin(), type.end(), type.begin(), :: tolower);

	GPIOClass* out1 = new GPIOClass("4");
	GPIOClass* in2 = new GPIOClass("17");
	// lets assume that the way to run this is
	// pwm.exe [rising/falling/sine/constant]
	if (argc != 2) {
		cout << "Usage: pwm [rising/falling/sine/constant/blink/morse]" << endl;
		return -1;
	}
	if (type == "morse") {
		string in;
		cout << "Type sentence to translate to morse code. Or type recieve to listen" << endl;
		getline(cin, in);
		cout << "enter how long 1 pulse is in 1/x secconds (10 seems good): " << endl;
		double r;
		cin >> r;
		resolution = r;
		if (in == "recieve" || in == "listen") {
			morseReciever(in2);
		}
		morseCoder(in2, in);
		return 0;
	}
	while (time_to_complete <= 0) {
		cout << "Input How Long To Run (in seconds)" << endl;
		cin >> time_to_complete;
	}

	out1->export_gpio();
	in2->export_gpio();

	out1->setdir_gpio("out");
	in2->setdir_gpio("in");

	cout << "Pins are setup." << endl;
	cout << "resolution of cpu" << CLOCKS_PER_SEC << endl;
	// avoiding flickering will be at 100hz
	// aka turn on and off 100 times a sec
	// a cycle of 0 is off
	// a cycle of 100 is on

	if (type == "rising") {
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		double t = time_to_complete;
		int Up = 0;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			if(!Up){
				if(t!=0){
					--t;
				}
				else{
					Up = 1;
					++t;
				}
			}
			else{
				if(t!=time_to_complete){
					++t;
				}
				else{
					Up = 0;
					--t;
				}
			}
			Pulse(out1, sin((PI/2)/t));
			Wait(sin((PI/2)/t));
		}
	}
	if (type == "falling") {
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			for (double i = 0; i < 1; i += resolution) {
				Pulse(out1, 1/resolution);
				Wait(i/resolution);
			}
		}
	}
	if (type == "sine") {
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			for (double i = 0; i < 1; i += resolution) {
				Pulse(out1, 1/resolution);
				cout << sin(i)/resolution << endl;
				Wait(sin(i)/resolution);
			}
		}
	}
	if (type == "constant") {
		out1->setval_gpio("1"); // turn the pin on
		Wait(time_to_complete); // sleep for number of cycles / 1/100 sec
		//cout << "Waiting during pulse" << endl;
		out1->setval_gpio("0"); // turn the pin off
	}
	if (type == "blink") { // aka. TESTR
		clock_t finish = clock() + time_to_complete * CLOCKS_PER_SEC;
		while (clock() < finish) {
			// pulse for however long we need to to achieve brightness.
			Pulse(out1, 1/resolution);
			Wait(4/resolution);
		}
	}

	cout << "Done." << endl;
}

//1 cycle is 1/100th of a second
//100 cycles is 1 sec
void Pulse(GPIOClass* pin, double cycles) {
	bool running = true;
	while (running) {
		pin->setval_gpio("1"); // turn the pin on
		Wait(cycles / resolution); // sleep for number of cycles / 1/100 sec
		//cout << "Waiting during pulse" << endl;
		pin->setval_gpio("0"); // turn the pin off
		running = false; // this is unnessesary but could be useful if modified a bit.
	}
}

void Wait ( double seconds )
{
	clock_t endwait;
	endwait = clock () + seconds * CLOCKS_PER_SEC ;
	while (clock() < endwait) {}
}

void morseReciever(GPIOClass* pin){
	string in, s;
	bool running = true;
	cout << "lets start." << endl;
	while (running) {
		pin->getval_gpio(s);
		in += s;
		cout << s;
		if(in.length() > 12){
			if (in.substr(in.length()-12) == "000000000000") {
				running = false;
			}
		}
	}
	cout << endl;
	int start, space;
	string charv;
	start = in.find("1");
	in = in.substr(start);

	space = in.find("0000000");
	vector<string> word, character;

	while(in.length() > 0){
		word.push_back(in.substr(0,space));
		in = in.substr(space+7);
		word.push_back("s");
	}

	for(int i =0; i<word.size(); i++){
		charv = in.at(i);
		if(in.at(i) != 's'){
			while(charv.length() > 0){
				string temp;
				temp = charv.substr(0, charv.find("000"));
				character.push_back(temp);
				charv = charv.substr(charv.find("000")+3);
			}
		}
		else{
			character.push_back(" ");
		}
	}
}

void morseCoder(GPIOClass* pin1, string s) {
	transform(s.begin(), s.end(), s.begin(), :: tolower);
	for (int i = 0; i < s.length(); i++) {
		if (s.at(i) == ' ') {
			Wait( 7 / resolution );
		} else {
			Wait(3/resolution); //wait
			morseBlink(pin1, s.at(i));
		}
	}
}

void morseBlink(GPIOClass* pin1, char c) {
	switch (c) {
	case 'a':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'b':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'c':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'd':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'e':
		Pulse(pin1, 1.0); //dot
		break;
	case 'f':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'g':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'h':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'i':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'j':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'k':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'l':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'm':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'n':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'o':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'p':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'q':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'r':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 's':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 't':
		Pulse(pin1, 3.0); //dash
		break;
	case 'u':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'v':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'w':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'x':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'y':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'z':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		break;
	case '1':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '2':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '3':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '4':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '5':
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '6':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '7':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '8':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '9':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '0':
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(3/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	}
}
