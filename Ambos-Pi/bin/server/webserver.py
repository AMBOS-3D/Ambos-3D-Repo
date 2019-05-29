from flask import Flask
from flask import jsonify
from flask import request
from flask import send_file
from picamera import PiCamera
from time import sleep
import subprocess

subprocess.call(['pkill', 'AMBOS_PbL'])
sleep(1)
subprocess.Popen(['mate-terminal', '--command', '/home/ambos/AMBOS-PbL/bin/AMBOS_PbL /home/ambos/AMBOS-PbL/data/order/current.json'])


app = Flask(__name__)

@app.route('/')
def hello_world():
	return jsonify(result='Hello World')

@app.route('/running/')
def running():
	return jsonify(result='true')

@app.route('/api/Image/<int:pid>.jpg')
def get_image(pid):
	subprocess.call(['pkill', 'AMBOS_PbL'])
	sleep(2)
	#init Camera
	camera = PiCamera()
	camera.resolution = (640, 480)
	#get Image
	camera.capture('/home/ambos/AMBOS-PbL/bin/server/img.jpg')
	camera.close()
	sleep(1)
	subprocess.Popen(['mate-terminal', '--command', '/home/ambos/AMBOS-PbL/bin/AMBOS_PbL /home/ambos/AMBOS-PbL/data/order/current.json'])
	return send_file('/home/ambos/AMBOS-PbL/bin/server/img.jpg', 
			attachment_filename='img.jpg', 
			mimetype='image/jpeg')

@app.route('/api/updateOrder/<uuid>', methods=['GET', 'POST'])
def update_order(uuid):
	subprocess.call(['pkill', 'AMBOS_PbL'])
	sleep(2)
	with open("/home/ambos/AMBOS-PbL/data/order/current.json","wb") as fw:
		fw.write(request.get_data())
	sleep(1)
	subprocess.Popen(['mate-terminal', '--command', '/home/ambos/AMBOS-PbL/bin/AMBOS_PbL /home/ambos/AMBOS-PbL/data/order/current.json'])
	return "OK"
	

if __name__ == '__main__':
	app.run(debug=False,host="192.168.101.1")
