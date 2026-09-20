#this is how you comment out a line
header = "-" * 30 + "\n\n"+ "-" * 30
print (header)
print("Many thanks to learnpythonthehardway.org. Some of this code is directly copied from this awesome course.")
print (header)

#to add a get out of jail free card:
print ("Hit RETURN to continue, CTRL-C to abort.")
input()

print (header)


#to quote multiple lines:
message = """\nhello and welcome
to my\n
gameshow"""

#and to print
print(message)

print (header)

#can also print as follows:
x = 1
y = "hello"
print ("There are", x, y)
print ("There are %d %s things!" % (x, y)) #d=data, s=string
z = "There are %d %s things!..again"
print (z % (x, y))

print (header)

#to combine the variables
a='a'
b='b'
c='c'
d='d'
print(a+b+c+d)

print(header)
#to ask questions from user
print("how old are you?")
age = input()
print ("so you are %s year(s) old!" %(age))
#or this way
#print("how old are you?")
#print("You are {0} year(s) old!".format(input()))
#or this way...
y = input("What is your Name? ")
print ("Hello %s" % (y))


#print(header)
#to use arguments when calling the script
#ie, python intro.py grayninja
#of course, uncomment below to use

#from sys import argv
#self, script, user_name = argv
#print ('hi %s, this is %s script' % (user_name, script))


print(header)
#to read text, files
filename = input("Please enter a filename (with path if in separate folder): ")
txt = open(filename)
print (txt.read())


#more text stuff..uncomment to run
#filename = input("Please enter a filename (with path if in separate folder): ")
#txt = open(filename, 'w')
#txt.truncate() #this will delete the contents in your filename
##now we will ask for text, then add to file
#line1 = input("line 1: ")
#line2 = input("line 2: ")
#line3 = input("line 3: ")
#txt.write(line1)
#txt.write("\n")
#txt.write(line2)
#txt.write("\n")
#txt.write(line3)
#txt.write("\n")
#txt.close() #closes file



#or to save to a new file
#filename = input("Please enter a filename (with path if in separate folder): ")
#txt = open(filename)
#old_file = txt.read()
#new_filename = input("Please enter the name of the new file: ")
#new_txt = open(new_filename, 'w')
#new_txt.write(old_file)


print(header)
print("Test to see if a port is open")
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sitename = input('Enter a website: ')
portno = input('Enter a Port Number: ')
result = s.connect_ex(sitename, int(portno))
#print(result)
if result = 0:
	print("%d is open!" % (portno)
else:
	print("%d is closed :(" % (portno)
#0 means the port is open
