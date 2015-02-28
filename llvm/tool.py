import subprocess
import hashlib
import binascii

debug = False
dk_verify = ""
dk_payload = ""
hostid = ""
verify_salt = "Uj_y6L*-mhc@77d"
payload_salt = "FnF4Imd5cQ_z!bF"
system_info = dict({
	"node name":"uname -n",
	"code name":"lsb_release -cs",
	"language":"echo $LANG"
})

def get_new_salt():
	#generate a new salt if you want one 
	cmd = "cat /dev/urandom | tr -dc '0-9a-zA-Z!@$%^&*_+-' | head -c 15"
	new_salt, err = subprocess.Popen(cmd, shell=True, executable="/bin/bash", stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
	new_salt = new_salt.rstrip('\n')
	print "new_salt = " + new_salt


def generate_key():
	global hostid
	global dk_verify
	global dk_payload

	#ask user for these inputs
	if debug:
		hostid = "Ubuntu" + "Trusty" + "en_US.UTF-8"
	else:
		for desc in system_info:
			info = raw_input("Enter " + desc + ": ")
			hostid = hostid + info

	dk_verify = hashlib.pbkdf2_hmac('sha256', hostid, verify_salt, 10000)
	dk_payload = hashlib.pbkdf2_hmac('sha256', hostid, payload_salt, 10000)
	
	dk_verify = binascii.hexlify(dk_verify)
	dk_payload = binascii.hexlify(dk_payload)

	if debug:
		print "hostid = " + hostid
		print "dk_verify = " + dk_verify
		print "dk_payload = " + dk_payload

def output_to_file():
	#generate the keys and output to a file 
	f = open("key.txt", "w")
	f.write("hostid = '" + hostid + "'\n")
	f.write("verify_salt = '" + verify_salt + "'\n")
	f.write("payload_salt = '" + payload_salt + "'\n")
	f.write("dk_verify = '" + dk_verify + "'\n")
	f.write("dk_payload = '" + dk_payload + "'\n")
	f.close()

#get_new_salt()
generate_key()
output_to_file()
