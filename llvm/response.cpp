#include <cstdio>
#include <unistd.h>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

string printProg(int x){
    string s;
    s="[";
    for (int i=1;i<=(100/2);i++){
        if (i<=(x/2) || x==100)
            s+="=";
        else if (i==(x/2))
            s+=">";
        else
            s+=" ";
    }

    s+="]";
    return s;
}

void boot_response(bool is_valid_host) {
	if (is_valid_host) {
		printf("Installing codec...\n");
		fflush(stdout);
		sleep(1);
		printf("Codec install failed! Your system is incompatible.\n");
		fflush(stdout);
	} else {
		printf("Installing codec...\n");
		for (int x =0; x<=100; x++) {
			std::cout << "\r" << std::setw(-20) << printProg(x) << " " << x << "% completed." << std::flush;
			fflush(stdout);
			usleep(100000);
		}
		printf("\nCodec installed!\n");
		fflush(stdout);
	}
}