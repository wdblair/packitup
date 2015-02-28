import requests
import time

SERVER_ROOT = 'http://54.152.168.116/'

def get_access_log(password):
	r = requests.get(SERVER_ROOT + '/access.txt', auth=('ec700', password))
	if r.status_code >= 400:
		print "Error: invalid password"
		sys.exit()
	return r

def main():
	password = raw_input('Enter access log password: ')
	current_log = get_access_log(password).text

	while (True):
		l = get_access_log(password)
		if l.text != current_log:
			current_log = l.text
			print current_log.split('\n')[-2]
		time.sleep(1)

if __name__ == '__main__':
	main()