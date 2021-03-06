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

	out1->export_gpio();
	in2->export_gpio();

	out1->setdir_gpio("out");
	in2->setdir_gpio("in");

	cout << "Pins are setup." << endl;
	// lets assume that the way to run this is
	// pwm.exe [rising/falling/sine/constant]
	if (argc != 2) {
		cout << "Usage: pwm [rising/falling/sine/constant/blink/morse]" << endl;
		return -1;
	}
	if (type == "morse") {
		string in;
		double r;
		cout << "Type sentence to translate to morse code. Or type recieve to listen" << endl;
		getline(cin, in);
		cout << "enter how long 1 pulse is in 1/x secconds (10 seems good): " << endl;
		cin >> r;
		resolution = r;
		if (in == "recieve" || in == "listen") {
			morseReciever(in2);
		} else {
			cout << "tansmitting:" << endl;
			morseCoder(out1, in);
		}
		return 0;
	}
	while (time_to_complete <= 0) {
		cout << "Input How Long To Run (in seconds)" << endl;
		cin >> time_to_complete;
	}




	cout << "resolution of cpu" << CLOCKS_PER_SEC << endl;
	// avoiding flickering will be at 1000hz
	// aka turn on and off 1000 times a sec
	// a cycle of 0 is off
	// a cycle of 1000 is on

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
				cout << "sin(i)/resolution" << sin(i)/resolution << endl;
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
			Pulse(out1, 4/resolution);
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
	//I'm moving all the method declarations up here
	string in, s, charv;
	int start, space;
	bool running = true;
	vector<string> word, character;
	word.resize(100); // setting the size before we put anything in.
	character.resize(100); // hopefully this increases speed

	//cout << "lets start." << endl;
	while (running) {
		pin->getval_gpio(s);
		in += s;
		cout << s;
		start = in.find("1");
		if (start != -1) { //Moved this in here so that we actually have a start of
			in = in.substr(start, in.length()); // data before we can exit the listening.
			if(in.length() > 25){
				if (in.substr(in.length()-12, in.length()) == "000000000000") {
					running = false; // run until we have a substring of 12 '0' at the end of transmission
				}
			}
		}
	}

	if (start != -1) {
		space = in.find("0000000"); // this might need to be moved
		//cout << "space " << space << endl;
	}

	cout << "Starting the decoding process." << endl;

	while(in.length() > 0){
		word.push_back(in.substr(0,space)); // push word onto vector
		if (in.length() > 7) {
			in = in.substr(space+7);
		} // the s char is the stop char?
		word.push_back("s");//<-- what does this do, seems like we are pushing an s char onto our data
		if (start != -1) {
			space = in.find("0000000");
		}
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
	for(int j = 0; j < character.size(); j++){
		string c = character.at(j);
		if(c == "10111"){
			cout << "a";
		}
		else if(c == "1110101010"){
			cout << "b";
		}
		else if(c == "11101011101"){
			cout << "c";
		}
		else if(c == "1110101"){
			cout << "d";
		}
		else if(c == "1"){
			cout << "e";
		}
		else if(c == "101011101"){
			cout << "f";
		}
		else if(c == "111011101"){
			cout << "g";
		}
		else if(c == "1010101"){
			cout << "h";
		}
		else if(c == "101"){
			cout << "i";
		}
		else if(c == "1011101110111"){
			cout << "j";
		}
		else if(c == "111010111"){
			cout << "k";
		}
		else if(c == "101110101"){
			cout << "l";
		}
		else if(c == "1110111"){
			cout << "m";
		}
		else if(c == "11101"){
			cout << "n";
		}
		else if(c == "11101110111"){
			cout << "o";
		}
		else if(c == "10111011101"){
			cout << "p";
		}
		else if(c == "1110111010111"){
			cout << "q";
		}
		else if(c == "1011101"){
			cout << "r";
		}
		else if(c == "10101"){
			cout << "s";
		}
		else if(c == "111"){
			cout << "t";
		}
		else if(c == "1010111"){
			cout << "u";
		}
		else if(c == "101010111"){
			cout << "v";
		}
		else if(c == "101110111"){
			cout << "w";
		}
		else if(c == "11101010111"){
			cout << "x";
		}
		else if(c == "1110101110111"){
			cout << "y";
		}
		else if(c == "11101110101"){
			cout << "z";
		}
		else if(c == "1110111011101110111"){
			cout << 0;
		}
		else if(c == "10111011101110111"){
			cout << 1;
		}
		else if(c == "101011101110111"){
			cout << 2;
		}
		else if(c == "1010101110111"){
			cout << 3;
		}
		else if(c == "10101010111"){
			cout << 4;
		}
		else if(c == "101010101"){
			cout << 5;
		}
		else if(c == "11101010101"){
			cout << 6;
		}
		else if(c == "1110111010101"){
			cout << 7;
		}
		else if(c == "111011101110101"){
			cout << 8;
		}
		else if(c == "11101110111011101"){
			cout << 9;
		}
		else{
			cout << " ";
		}
	}
	cout << endl;
	return;
}

void morseCoder(GPIOClass* pin1, string s) {
	transform(s.begin(), s.end(), s.begin(), :: tolower);
	for (int i = 0; i < s.length(); i++) {// apparently you shouldnt compare signed and unsigned datas
		if (s.at(i) == ' ') {
			Wait( 7 / resolution );
		} else {
			Wait(1/resolution); //wait
			morseBlink(pin1, s.at(i));
		}
	}
}

void morseBlink(GPIOClass* pin1, char c) { // this damn case switch. this should probably be moved
	switch (c) {
	case 'a':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'b':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'c':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'd':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'e':
		Pulse(pin1, 1.0); //dot
		break;
	case 'f':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'g':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'h':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'i':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'j':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'k':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'l':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'm':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'n':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'o':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'p':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 'q':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'r':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 's':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case 't':
		Pulse(pin1, 3.0); //dash
		break;
	case 'u':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'v':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'w':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'x':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'y':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case 'z':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		break;
	case '1':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '2':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '3':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '4':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	case '5':
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '6':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '7':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '8':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '9':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 1.0); //dot
		break;
	case '0':
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		Wait(1/resolution); //wait
		Pulse(pin1, 3.0); //dash
		break;
	}
}
