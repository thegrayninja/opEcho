###This script has been created to view if the number of Security Groups
###have changed since the previous check. 


import json			##allows importing of json
from pprint import pprint  	##allows better readability from json
import smtplib


####the json file was created by the following aws cli command:
####aws ec2 describe-security-groups --group-names >> group_names.json


##importing/reading the .json file
with open('group_names.json') as data_file:
	data = json.load(data_file)
#pprint(data)			##to view the entire json file


totalSG = (len(data["SecurityGroups"]))  ##provides number of SecGroups
print "There are a total of", totalSG, "Service Groups."


originalFile = open("sgchanges_alert.txt", "r")
x = originalFile.read()
originalFile.close()

if int(x) == totalSG:
	print "They are the same"
else:
	print "***CHANGE***\nPrevious SG count was %s, but the current count is %s!\n************" % (x, totalSG)

originalFile = open("sgchanges_alert.txt", "w")
originalFile.write("%s" % (totalSG))
originalFile.close()
