import json					##allows importing of json
from pprint import pprint  	##allows better readability from json


####the json file was created by the following aws cli command:
####aws ec2 describe-security-groups --group-names >> group_names.json


##importing/reading the .json file
with open('group_names.json') as data_file:
	data = json.load(data_file)
#pprint(data)				##to view the entire json file


totalSG = (len(data["SecurityGroups"]))  ##provides number of SecGroups
print "There are a total of", totalSG, "Service Groups."


##the following is to simply print a list of each GroupName
##dictSG is not necessary, just thought I'd build a list of
##all group names...i'm sure it'll be handy later
listSG = 0
dictSG = []
while listSG < totalSG:
	dictSG.append (data["SecurityGroups"][listSG]["GroupName"])
	print dictSG[listSG]
	listSG += 1


##the following will print the IpPermissions for SG4. This can
##be implemented in a similar while function as the SG GroupName
print "Current IP Permissions set for", data["SecurityGroups"][3]["GroupName"], "Service Group"
pprint (data["SecurityGroups"][3]["IpPermissions"])
 


