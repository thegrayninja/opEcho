import os			##allows us to run ping/hping3
import json			##allows importing of json
from pprint import pprint  	##allows better readability from json


####the json file was created by the following aws cli command:
####aws ec2 describe-instances >> instances_info.json

####also, you must have hping3 installed to test 443 connectivity


##importing/reading the .json file
with open('instances_info.json') as data_file:
	data = json.load(data_file)
#pprint(data)				##to view the entire json file



##the following will cycle through all of your instances
##and print out the PublicDnsNames

numOfInstances = len(data["Reservations"][0]["Instances"])  #counts how many instances there are
##finally..this took me way to long to realize I needed "Instances" :D

countOfInstances = 0
listOfInstances = []
while countOfInstances < numOfInstances:
	listOfInstances.append (data["Reservations"][0]["Instances"][0]["PublicDnsName"])
	print listOfInstances[countOfInstances]
	countOfInstances += 1


##the following will take the publicDNS names and attempt to ping them
##if you are like most people, ping will be disabled anyway
for i in listOfInstances:
	hostname = i
	response = os.system("ping -c 1 -w2 " + hostname + " > /dev/null 2>&1") 
		##thanks Manuel, via http://stackoverflow.com/questions/2953462/pinging-servers-in-python 
		##also, if it fails, pay attention to your spacing
	if response == 0:
		print hostname, 'ping is up!'
	else:
		print hostname, 'ping is down!'


##the following will take the publicDNS names and sends Syn packet to 443
##if in Kali, hping3 by default required to run via root
for i in listOfInstances:
	hostname = i
	response = os.system("hping3 -S -p 443 -c 5 " + hostname + " > /dev/null 2>&1") 
		##if it fails, pay attention to your spacing
	if response == 0:
		print hostname, '443 is up!'
	else:
		print hostname, '443 is down! are you running hping3 as root?'

##just a few more printout examples
print "Original Launch Date:", data["Reservations"][0]["Instances"][0]["LaunchTime"]
print "Security Groups:", data["Reservations"][0]["Instances"][0]["SecurityGroups"][0]["GroupName"]
print "Architecture:", data["Reservations"][0]["Instances"][0]["Architecture"]
