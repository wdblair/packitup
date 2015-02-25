import datetime
import hashlib
import sys
import subprocess
import os
import binascii

# requires python 2.7.9

def main():
	debug = False
	
	#generate host id
	p = subprocess.Popen(["uname", "-n"], stdout=subprocess.PIPE)
	nodename, err = p.communicate()
	nodename = nodename.rstrip('\n')

	p = subprocess.Popen(["lsb_release", "-cs"], stdout=subprocess.PIPE)
	codename, err = p.communicate()
	codename = codename.rstrip('\n')

	my_env=os.environ
	lang = my_env["LANG"]

	hostid = nodename+codename+lang

	salt = "Uj_y6L*-mhc@77d"
  
        # This key derivation algorithm is also in openssl, so we can use it here. 
        dk = hashlib.pbkdf2_hmac('sha256', hostid, salt, 10000)
	return binascii.hexlify(dk)

print main()
