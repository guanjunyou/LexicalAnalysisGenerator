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
state = 2;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 1:
if (ch == 'b') {
state = 2;
}
else {
cout << "wrong expression!!!wrong state(" << state << ")" << endl;
return 0;
}
break;
case 2:
if (ch == '\n'){cout << "corrent expression!!!" << endl;
return 0;
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
