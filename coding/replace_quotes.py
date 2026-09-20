### the point of this is to fix jacked up json files
import json			##allows importing/manipulation of json
from pprint import pprint  	##allows better readability from json


### showing how this method works in it's simplist form
s = "\'hi\'"   			#got to add the \
print s
print s.replace("\'", '\"')	#notice the switcharoo with the quotes


###i need to lookup how to save directly to a new file...should be easy enough
###anyway, this temporarily stores the changed document into variable x,
###then saves it to the new file. doesn't have to be json
originalFile = open("replace_quotes.json", "r")
x = originalFile.read().replace("\'", '\"')
originalFile.close()

print x				#prints out the updated "'s

updatedFile = open("replace_quotes_new.json", "w") 	#creates new file
updatedFile.write(x)		#then writes to file
updatedFile.close()

