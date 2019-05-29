import sys
import requests

req = requests.request('GET', 'http://192.168.101.1:5000/running')
if req.status_code == 200:
	print("true python")
	sys.exit(0)
else:
	print("false python")
	sys.exit(1)
