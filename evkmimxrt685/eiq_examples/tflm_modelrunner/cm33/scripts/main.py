#!/usr/bin/env python3

from flask import Flask, request, Response, make_response
import base64
import collections
import json
import os

from modelrunner import Dut

import os
import time

BasePath = os.path.dirname(os.path.realpath(__file__))
if(not os.path.isdir("%s/logs" %BasePath)):
    os.mkdir("%s/logs" %BasePath)

from flask_cors import CORS


app = Flask(__name__)
CORS(app, supports_credentials=True)

model_info = {"inputs": [], "outputs": []}

@app.route("/serial/<serialId>/v1", methods = ["GET"])
@app.route("/serial/<serialId>/")
def v1(serialId):
    resp = {
            "engine": "TensorFlow Lite",
            "model_limits": {
                "block_size": 104857600,
                "max_layers": 512,
                "max_input_size": 607500,
                "max_model_size": 20971520,
                }
            }

    return resp

@app.route("/serial/<serialId>/v1", methods = ["PUT"])
def v1_put(serialId):
    resp = {
            "reply": "success"
            }
    bc = request.form.get("block_count", None)
    global block_count 
    global buf
    if bc:
        block_count = int(bc)
        buf = b''
        dut = Dut(serialId)
        dut.reset()
        del(dut)
        return resp
    for filename in request.files.keys():
        if filename == "block_content":
            buf += request.files[filename].stream.read()
            block_count = block_count - 1
        elif filename == "block_count":
            buf = request.files[filename].stream.read()
            block_count = int(buf)
            buf = b''
            dut = Dut(serialId)
            dut.reset()
            del(dut)
            return resp
    if block_count == 0:
       with open('%s/model.tflite' %BasePath, 'wb+') as fd:
           fd.write(buf)
       dut = Dut(serialId)
       dut.send_file("model_loadb" ,"%s/model.tflite" %BasePath)
       del(dut)

    return resp

@app.route("/serial/<serialId>/v1", methods = ["POST"])
def v1_post(serialId):
    outputs = request.args.getlist("output")
    run = request.args.get("run")
    tensor = None
    dut = Dut(serialId)
    for filename in request.files.keys():
        buf = request.files[filename].stream.read()
        with open('%s/tmp.input' %BasePath, 'wb+') as fd:
            fd.write(buf)
        tensor = "%s/tmp.input" %BasePath
        dut.send_file("tensor_loadb %s" %filename, tensor)
    param = request.full_path.split("?")[1].replace("&", " ")
    results = dut.send_cmd("run %s"%param)
    r = json.loads(results)

    del(dut)
    return r

@app.route("/serial/<serialId>/v1/model", methods = ["GET"])
def v1_model(serialId):
    dut = Dut(serialId)
    results = dut.send_cmd("model")
    r = json.loads(results)
    del(dut)
    return r

if __name__ == "__main__":
    app.run(host="0.0.0.0", debug = True, port = 10919)
