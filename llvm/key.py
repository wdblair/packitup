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

	verify_salt = "FAK@$P[';wea!e2"
	payload_salt = "Uj_y6L*-mhc@77d"
  
    # This key derivation algorithm is also in openssl, so we can use it here. 
	dk_verify = hashlib.pbkdf2_hmac('sha256', hostid, verify_salt, 10000)
	dk_payload = hashlib.pbkdf2_hmac('sha256', hostid, payload_salt, 10000)

	return binascii.hexlify(dk_payload) + dk_verify

print main()
