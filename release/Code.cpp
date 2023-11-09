#include <iostream>
#include <cstdio>
using namespace std;

int main() {
int state = 0;
while (true) {
char ch = getchar();

switch (state) {
case 0:
if (ch == 'a') {
state = 1;
}
else if (ch == 'b') {
state = 1;
}
else if (ch == 'd') {
state = 5;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 1:
if (ch == 'd') {
state = 5;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 2:
if (ch == 'd') {
state = 2;
}
else if (ch == '\n'){cout << "corrent expression!!!" << endl;
return 0;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 3:
if (ch == 'E') {
state = 4;
}
else if (ch == 'd') {
state = 3;
}
else if (ch == 'e') {
state = 4;
}
else if (ch == '\n'){cout << "corrent expression!!!" << endl;
return 0;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 4:
if (ch == 'a') {
state = 7;
}
else if (ch == 'b') {
state = 7;
}
else if (ch == 'd') {
state = 2;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 5:
if (ch == 'E') {
state = 4;
}
else if (ch == 'c') {
state = 6;
}
else if (ch == 'd') {
state = 5;
}
else if (ch == 'e') {
state = 4;
}
else if (ch == '\n'){cout << "corrent expression!!!" << endl;
return 0;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 6:
if (ch == 'd') {
state = 3;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 7:
if (ch == 'd') {
state = 2;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
default:
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
}

cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
