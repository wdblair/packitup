#!/usr/bin/env python
# -*- coding: utf-8 -*-
# Topmenu and the submenus are based of the example found at this location http://blog.skeltonnetworks.com/2010/03/python-curses-custom-menu/
# The rest of the work was done by Matthew Bennett and he requests you keep these two mentions when you reuse the code :-)
# Basic code refactoring by Andrew Scheller

from time import sleep
from collections import OrderedDict
import subprocess
import hashlib
import binascii
import fileinput
import shutil
import sys
import curses, os #curses is the interface for capturing key presses on the menu, os launches the files
from curses.textpad import Textbox, rectangle
screen = curses.initscr() #initializes a new window for capturing key presses
curses.noecho() # Disables automatic echoing of key presses (prevents program from input each key twice)
curses.cbreak() # Disables line buffering (runs each key as it is pressed rather than waiting for the return key to pressed)
curses.start_color() # Lets you use colors when highlighting selected menu option
screen.keypad(1) # Capture input from keypad

# Change this to use different colors when highlighting
curses.init_pair(1,curses.COLOR_BLACK, curses.COLOR_WHITE) # Sets up color pair #1, it does black text with white background
h = curses.color_pair(1) #h is the coloring for a highlighted menu option
n = curses.A_NORMAL #n is the coloring for a non highlighted menu option

MENU = "menu"
COMMAND = "command"
EXITMENU = "exitmenu"

keyText_default = "key.txt"
keyText = ""
keyFile_default = "key.cpp"
keyFile = ""
keyTemplate = "keygen.cpp"
debug = False
dk_verify = ""
dk_payload = ""
hostid = ""
verify_salt = "Uj_y6L*-mhc@77d"
payload_salt = "FnF4Imd5cQ_z!bF"
system_info = OrderedDict({
	"node name":"uname -n",
	"Language":"echo $LANG",
	"Code name":"lsb_release -cs",
	"OS":"uname -o",
	"Public IP":"dig myip.opendns.com @resolver1.opendns.com +short", 
	"Firefox Version":"firefox -v"
})

system_keys = ["Code name", "Language", "Linux Distro", "Public IP", "Firefox Version", "Program name"]
cpp_keys = ["codeNameCmd", "langCmd", "linuxDistroCmd", "extIpCmd", "firefoxVerCmd", "programNameCmd"]
hostid_components = ""

menu_data = {
  'title': "HIE Generator", 'type': MENU, 'subtitle': "Please select an option...",
  'options':[
	{ 'title': "Generate hostid", 'type': MENU, 'subtitle': 'Please select the system info you want to enter...',
	'options': [
		{ 'title': system_keys[0], 'type': MENU, 'subtitle': 'Please select an option...',
			'options': [
		          { 'title': "lucid", 'type': COMMAND, 'command': system_info.get('language') },			
			  { 'title': "precise", 'type': COMMAND, 'command': system_info.get('language') },
		          { 'title': "trusty", 'type': COMMAND, 'command': system_info.get('language') },			
		          { 'title': "utopic", 'type': COMMAND, 'command': system_info.get('language') },			
		          { 'title': "Custom value", 'type': COMMAND, 'command': ''}, 
		]
		},
		{ 'title': system_keys[1], 'type': MENU, 'subtitle': 'Please select an option...',
			'options': [
		          { 'title': "en_US.UTF-8", 'type': COMMAND, 'command': system_info.get('language') },			
			  { 'title': "ru_RU.UTF-8", 'type': COMMAND, 'command': system_info.get('language') },
		          { 'title': "fr_FR.UTF-8", 'type': COMMAND, 'command': system_info.get('language') },			
		          { 'title': "de_DE.UTF-8", 'type': COMMAND, 'command': system_info.get('language') },			
		          { 'title': "Custom value", 'type': COMMAND, 'command': "echo enter your own!" },			
		]	
		},
		{ 'title': system_keys[2], 'type': MENU, 'subtitle': 'Please select an option...',
	    	   'options': [
	        	  { 'title': "Ubuntu", 'type': COMMAND, 'command': '' },
		          { 'title': "Kali", 'type': COMMAND, 'command': '' },
		          { 'title': "Debian", 'type': COMMAND, 'command': '' },
		          { 'title': "Fedora", 'type': COMMAND, 'command': '' },
		          { 'title': "Custom value", 'type': COMMAND, 'command': 'uname -o' },
	        ]
	        },
	        { 'title': system_keys[3], 'type': COMMAND, 'command': 'dig myip.opendns.com @resolver1.opendns.com +short' },	
		{ 'title': system_keys[4], 'type': COMMAND, 'command': 'firefox -v' },
		{ 'title': system_keys[5], 'type': COMMAND, 'command': 'Please select an option...'},
		{ 'title': "Start Over", 'type': COMMAND, 'command': 'lala' },
	]
	},
	{ 'title': "Generate Keys", 'type': COMMAND, 'command': 'lala' },
	{ 'title': "Generate cpp", 'type': COMMAND, 'command': '' },
	{ 'title': "Generate New Salts", 'type': MENU, 'subtitle': "Please select an option...",
        'options': [
          { 'title': "Generate verification salt", 'type': COMMAND, 'command': 'lala' },
          { 'title': "Generate decryption salt", 'type': COMMAND, 'command': 'lala' },
        ]
	},
  ]
}

# This function displays the appropriate menu and returns the option selected
def runmenu(menu, parent):

  # work out what text to display as the last menu option
  if parent is None:
    lastoption = "Exit"
  else:
    lastoption = "Return to %s menu" % parent['title']

  optioncount = len(menu['options']) # how many options in this menu

  pos=0 #pos is the zero-based index of the hightlighted menu option. Every time runmenu is called, position returns to 0, when runmenu ends the position is returned and tells the program what opt$
  oldpos=None # used to prevent the screen being redrawn every time
  x = None #control for while loop, let's you scroll through options until return key is pressed then returns pos to program

  # Loop until return key is pressed
  while x !=ord('\n'):
    if pos != oldpos:
      oldpos = pos
      screen.border(0)
      screen.addstr(2,2, menu['title'], curses.A_STANDOUT) # Title for this menu
      screen.addstr(4,2, menu['subtitle'], curses.A_BOLD) #Subtitle for this menu

      # Display all the menu items, showing the 'pos' item highlighted
      for index in range(optioncount):
        textstyle = n
        if pos==index:
          textstyle = h
        screen.addstr(5+index,4, "%d - %s" % (index+1, menu['options'][index]['title']), textstyle)
      # Now display Exit/Return at bottom of menu
      textstyle = n
      if pos==optioncount:
        textstyle = h
      screen.addstr(5+optioncount,4, "%d - %s" % (optioncount+1, lastoption), textstyle)
      screen.refresh()
      # finished updating screen
    curses.noecho()
    x = screen.getch() # Gets user input

    # What is user input?
    if x >= ord('1') and x <= ord(str(optioncount+1)):
      pos = x - ord('0') - 1 # convert keypress back to a number, then subtract 1 to get index
    elif x == 258: # down arrow
      if pos < optioncount:
        pos += 1
      else: pos = 0
    elif x == 259: # up arrow
      if pos > 0:
        pos += -1
      else: pos = optioncount

  # return index of the selected item
  return pos

# This function calls showmenu and then acts on the selected item
def processmenu(menu, parent=None):
  global hostid
  global hostid_components
  global keyFile
  global keyText
  global payload_salt
  global verify_salt
  optioncount = len(menu['options'])
  exitmenu = False
  while not exitmenu: #Loop until the user exits the menu
    getin = runmenu(menu, parent)
    if getin == optioncount:
        exitmenu = True
    elif menu['options'][getin]['type'] == COMMAND:
      curses.def_prog_mode()    # save curent curses environment
      if menu['options'][getin]['title'] == 'Generate Keys':
	info = get_param(menu['options'][getin]['title'], "Specify a file name or press [d] to use default name",screen)
	if info == "d":
		keyText = keyText_default		
		generate_key()
		output_to_file()
	elif info != "q":
		keyText = info
		generate_key()
		output_to_file()
      elif menu['options'][getin]['title'] == 'Generate cpp':
	info = get_param(menu['options'][getin]['title'], "Specify a file name or press [d] to use default name",screen)
	if info == "d":
		keyFile = keyFile_default
		keyText = keyText_default
		generate_key()
		output_to_file()
		create_cpp()
	elif info != "q":
		keyFile = info
		keyText = keyFile + ".txt"
		generate_key()
		output_to_file()
		create_cpp()
      elif menu['options'][getin]['title'] in (system_keys[3], system_keys[4], system_keys[5]):
		#public IP
		add_to_hostid(get_param(menu['options'][getin]['title'], "Please enter custom value...", screen), menu['options'][getin]['title'])
      elif menu['options'][getin]['title'] == 'Start Over':
		#wipe hostid contents and start again...
		hostid=""
		hostid_components=""
      else:
	#in submenus
      	if menu['options'][getin]['title'] == 'Generate verification salt':
		verify_salt = get_new_salt()
		screen.addstr(15,2, "verification salt = " + verify_salt) 
		screen.addstr(16,2, "decryption salt = " + payload_salt) 
      	elif menu['options'][getin]['title'] == 'Generate decryption salt':
		payload_salt = get_new_salt() 
	elif menu['options'][getin]['title'] != 'Custom value':
		hostid = hostid + menu['options'][getin]['title'] 
		exitmenu = True
		#hostid_components = hostid_components + menu['title'] + ";"
		hostid_components = hostid_components + cpp_keys[system_keys.index(menu['title'])] + ";"
	else:    
		info = get_param(menu['options'][getin]['title'], "Please enter custom value...", screen)  
		if info != "q":
			hostid = hostid + info			
			exitmenu = True		
			#hostid_components = hostid_components + menu['title'] + ";"
			hostid_components = hostid_components + cpp_keys[system_keys.index(menu['title'])] + ";"
      screen.clear() #clears previous screen on key press and updates display based on pos
      curses.reset_prog_mode()   # reset to 'current' curses environment
      curses.curs_set(1)         # reset doesn't do this right
      curses.curs_set(0)

      screen.addstr(15,2, "hostid = " + hostid) #current hostid status
      screen.addstr(16,2, "hostid_components = " + hostid_components) #current hostid status

      if menu['options'][getin]['title'] == 'Generate decryption salt' or menu['options'][getin]['title'] == 'Generate verification salt':
	screen.addstr(15,2, "verification salt = " + verify_salt) 
	screen.addstr(16,2, "decryption salt = " + payload_salt)
      elif menu['options'][getin]['title'] == 'Generate Keys':
	if info != "q":
		screen.addstr(19,2, "Keys have been written to " + keyText)   
      elif menu['options'][getin]['title'] == 'Generate cpp':
	if info != "q":
		screen.addstr(19,2, "Keys have been written to " + keyText)   
		screen.addstr(20,2, "Cpp has been writeen to " + keyFile)   

    elif menu['options'][getin]['type'] == MENU:
          screen.clear() #clears previous screen on key press and updates display based on pos
          processmenu(menu['options'][getin], menu) # display the submenu
          screen.clear() #clears previous screen on key press and updates display based on pos
	  screen.addstr(15,2, "hostid = " + hostid) #current hostid status
	  screen.addstr(16,2, "hostid_components = " + hostid_components) #current hostid status
    elif menu['options'][getin]['type'] == EXITMENU:
          exitmenu = True

def get_param(prompt_string, subtitle, stdscr):
     stdscr.clear() #clears previous screen
     stdscr.border(0)
     curses.echo()
     stdscr.addstr(1,2, prompt_string, curses.A_STANDOUT) # Title for this menu
     stdscr.addstr(3,2, subtitle, curses.A_BOLD) #Subtitle for this menu
     stdscr.addstr(4,2, "Or press [q] to go back", curses.A_BOLD) #Subtitle for this menu
     stdscr.refresh()
     input = stdscr.getstr(6, 2, 60)
     return input

def add_to_hostid(info, component_key):
	global hostid
	global hostid_components
	if info != "q":
		hostid = hostid + info
		#hostid_components = hostid_components + component_key + ";"
		hostid_components = hostid_components + cpp_keys[system_keys.index(component_key)] + ";"

def get_new_salt():
	#generate a new 16 byte salt if you want one 
	cmd = "cat /dev/urandom | tr -dc '0-9a-zA-Z!@#$%^&*_+-' | head -c 16"
	new_salt, err = subprocess.Popen(cmd, shell=True, executable="/bin/bash", stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
	new_salt = new_salt.rstrip('\n')
	return new_salt

def generate_key():
	global hostid
	global dk_verify
	global dk_payload
	
	#use hostid generated from menu
	#dk_verify = hashlib.pbkdf2_hmac('sha256', hostid, verify_salt, 10000)
	#dk_payload = hashlib.pbkdf2_hmac('sha256', hostid, payload_salt, 10000)
	
	dk_verify = ""
	dk_payload = ""	
	#dk_verify = binascii.hexlify(dk_verify)
	#dk_payload = binascii.hexlify(dk_payload)

def output_to_file():
	#generate the keys and output to a file 
	f = open(keyText, "w")
	f.write("hostid = '" + hostid + "'\n")
	f.write("verify_salt = '" + verify_salt + "'\n")
	f.write("payload_salt = '" + payload_salt + "'\n")
	f.write("dk_verify = '" + dk_verify + "'\n")
	f.write("dk_payload = '" + dk_payload + "'\n")
	f.close()

	#generate a just hostid file
	f = open("hostid", "w")
	f.write(hostid)
	f.close()

def create_cpp():
	replaceThis = 'std::string cmdListPy = "";'
	replaceWith = 'std::string cmdListPy = "' + hostid_components + '";'
	
	replaceVerSalt = 'static const char *verSalt = "";'
	replaceWithVerSalt = 'static const char *verSalt = "'+ verify_salt + '";'

	replacePayloadSalt = 'static const char *decryptSalt = "";'
	replaceWithPayloadSalt = 'static const char *decryptSalt = "'+ payload_salt +'";'

	#create copy of file
	shutil.copy(keyTemplate, keyFile)
	#replace string with my cmds
	for line in fileinput.input(keyFile, inplace=True):
		sys.stdout.write(line.replace(replaceThis, replaceWith))
	for line in fileinput.input(keyFile, inplace=True):
		sys.stdout.write(line.replace(replaceVerSalt, replaceWithVerSalt))
	for line in fileinput.input(keyFile, inplace=True):
		sys.stdout.write(line.replace(replacePayloadSalt, replaceWithPayloadSalt))
	
# Main program
processmenu(menu_data)
curses.endwin() #VITAL! This closes out the menu system and returns you to the bash prompt.
os.system('clear')

