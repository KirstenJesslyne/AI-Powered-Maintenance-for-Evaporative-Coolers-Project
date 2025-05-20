{\rtf1\ansi\ansicpg1252\cocoartf2709
\cocoatextscaling0\cocoaplatform0{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
{\*\expandedcolortbl;;}
\paperw11900\paperh16840\margl1440\margr1440\vieww11520\viewh8400\viewkind0
\pard\tx720\tx1440\tx2160\tx2880\tx3600\tx4320\tx5040\tx5760\tx6480\tx7200\tx7920\tx8640\pardirnatural\partightenfactor0

\f0\fs24 \cf0 import paho.mqtt.client as mqtt\
from pymongo import MongoClient\
import json\
from datetime import datetime\
\
# Koneksi ke MongoDB\
mongo_client = MongoClient("mongodb://localhost:27017/")\
db = mongo_client["mqtt_database"]\
collection = db["sensor_data"]\
\
# Callback saat pesan diterima\
def on_message(client, userdata, msg):\
    try:\
        payload = msg.payload.decode()\
        data = \{\
            "timestamp": datetime.utcnow(),\
            "topic": msg.topic,\
            "payload": payload\
        \}\
        collection.insert_one(data)\
        print(f"Data inserted: \{data\}")\
    except Exception as e:\
        print(f"Error inserting data: \{e\}")\
\
# Setup MQTT client\
mqtt_client = mqtt.Client()\
mqtt_client.on_message = on_message\
\
mqtt_client.connect("localhost", 1883)\
mqtt_client.subscribe("test/topic")  # Ganti dengan topik kamu\
\
mqtt_client.loop_forever()}