from flask import Flask
from flask import jsonify
from flask import request
from flask import send_file
from time import sleep
import subprocess, os
import cv2
import json 

# Instatiere App
app = Flask(__name__)

# Globale Variablen
HOME_DIR = "/home/ambos/workspace/Ambos-Pi"
O_FILE = "AMBOS_PbL"
ORDER = "current"

'''
Momentan wird aus der v1 Settingsdatei ausgelesen, welche Version gestartet werden soll.
Ist eher für Testzwecke gedacht, die in v2 dieser Webserver ersetzt wird.
''' 
with open(f"{HOME_DIR}/data/settings/settings.json", "r") as f:
	settings = json.load(f)
	VERSION = settings["ambos_version"]
	
# Version kann auch direkt so gesetzt werde
#VERSION = 2

# In v2 findet die Kommunikation über MQTT und nicht diesen Webserver statt
UNSUPPORTED_FUNCTIONS_IN_V2 = ["get_image", "update_order"]

def start_ambos():#
	"""
	Starte Ambos Applikation in einem neuen Terminal
	"""
	if VERSION == 1:
		pr = subprocess.Popen([f"gnome-terminal  --title Ambos -x  {HOME_DIR}/bin/{O_FILE} {VERSION} {HOME_DIR}/data/order/{ORDER}.json"], 
						 shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	else:
		pr = subprocess.Popen([f"gnome-terminal  --title Ambos -x  {HOME_DIR}/bin/{O_FILE} {VERSION}"], 
						 shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	return pr

def stop_ambos():
	"""
	Stoppe Ambos Applikation
	"""
	subprocess.call(['pkill', O_FILE])

def start_mqtt_broker():
	"""
	Starte MQTT Broker in einem neuen Terminal
	"""
	subprocess.Popen([f"gnome-terminal  --title Mosquitto -x mosquitto -v"], 
					  shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

def stop_mqtt_broker():
	"""
	Stoppe MQTT Broker
	"""
	subprocess.Popen([f"sudo pkill mosquitto"], shell=True)

@app.before_request
def before_request_func():
	"""
	Vor jedem request prüfe ob wir in V1 oder V2 sind
	und sperre Endpunkte welche es in V2 nicht mehr gibt
	"""
	if request.endpoint in UNSUPPORTED_FUNCTIONS_IN_V2 and VERSION>1:
		return "Only supported in Ambos V1", 501


@app.route('/')
def hello_world():
	return jsonify(result='Hello World')

@app.route('/running/')
def running():
	return jsonify(result='true')

# Lade das aktuelle Kamerabild, um Boxen einzuzeichenen
@app.route('/api/Image/<int:pid>.jpg')
def get_image(pid):
	stop_ambos()
	sleep(2)
	# Lade Kamera
	cap = cv2.VideoCapture(0, cv2.CAP_V4L2)
	cap.set(cv2.CAP_PROP_FRAME_WIDTH,1280)
	cap.set(cv2.CAP_PROP_FRAME_HEIGHT,720)
	# Lade Bild
	ret, frame = cap.read()
	cv2.imwrite(f"{HOME_DIR}/bin/server/img.jpg", frame)
	cap.release();
	sleep(1)
	start_ambos()
	return send_file(f"{HOME_DIR}/bin/server/img.jpg", 
			attachment_filename='img.jpg', 
			mimetype='image/jpeg')

# Route zum Updaten einer Order. Body is JSON als String
@app.route('/api/updateOrder/<uuid>', methods=['GET', 'POST'])
def update_order(uuid):
	stop_ambos()
	sleep(2)
	print("data: ", request.get_data())
	with open(f"{HOME_DIR}/data/order/current.json","wb") as fw:
		fw.write(request.get_data())
		
	sleep(1)
	start_ambos()
	return "OK"


if __name__ == '__main__':

	print(f"Webserver mit Ambos V{VERSION} gestartet...")
		
	# start ambos
	stop_ambos()
	sleep(1)
	pr = start_ambos()
	# in v>1 we need to start mqtt broker
	stop_mqtt_broker()
	if VERSION>1:
		sleep(1)
		start_mqtt_broker()
	
	# start webserver
	#app.run(debug=False,host="192.168.0.154", port=5000)
	app.run(debug=False,host="0.0.0.0", port=5000)


	
	
	