from flask import Flask, render_template, request, session
from flask_socketio import SocketIO, send, emit
import paho.mqtt.client as mqtt
from datetime import datetime

app = Flask(__name__)
app.secret_key = 'H140604:)'  # Ganti dengan secret key yang aman
socketio = SocketIO(app)

# List untuk menyimpan riwayat input dan waktu
history = []

# Konfigurasi MQTT
mqtt_broker = "broker.emqx.io"
mqtt_port = 1883
stepperControlTopic = "Hanif/stepper"  # Topik untuk stepper motor
ultrasonicTopic = "Hanif/sonic"  # Topik untuk sensor ultrasonik

# Callback ketika terhubung ke broker MQTT
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(ultrasonicTopic)

# Callback ketika pesan diterima dari broker MQTT
def on_message(client, userdata, msg):
    if msg.topic == ultrasonicTopic:
        distance = float(msg.payload.decode())
        percentage = calculate_percentage(distance)
        # Kirim data ke halaman web menggunakan SocketIO
        socketio.emit('distance_update', {'percentage': percentage})
        print(f"Ultrasonic distance: {distance} cm, Percentage: {percentage}%")

def calculate_percentage(distance):
    # Kalkulasi persentase berdasarkan jarak
    if distance >= 6:
        return 10
    elif distance <= 2:
        return 90
    else:
        return 100 - ((distance - 2) * 20)  # Linear interpolation between 90% and 10%

@app.route('/control_stepper', methods=['POST'])
def control_stepper():
    steps = request.form.get('steps')
    if steps:
        mqtt_client.publish(stepperControlTopic, steps)
        print(f"Stepper motor will move {steps} steps")

        # Tambahkan ke riwayat
        current_time = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        history.append({'steps': steps, 'time': current_time})
        session['history'] = history  # Simpan riwayat dalam session Flask
    
    return "Stepper command sent!"

# Inisialisasi MQTT Client
mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message
mqtt_client.connect(mqtt_broker, mqtt_port, 60)

# Jalankan MQTT Client dalam thread terpisah
mqtt_client.loop_start()

# Route untuk halaman utama
@app.route('/')
def index():
    return render_template('index.html')

@app.route('/feed')
def feed():
    return render_template('Feed.html')

# Route untuk halaman about
@app.route('/about')
def about():
    return render_template('about.html')

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
