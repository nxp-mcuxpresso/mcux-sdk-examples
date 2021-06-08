#!/usr/bin/python

# Copyright (c) 2016 Freescale Semiconductor, Inc.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

from __future__ import print_function

import sys
import os
import random
import argparse
from serial import SerialException

try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"middleware\multicore\erpc\erpc_python\setup.py\" script.")
    sys.exit()

from service import erpc_matrix_multiply

# make input function independent on Python version
input_fn = raw_input if sys.version_info[:2] <= (2, 7) else input

# get matrix size defined in IDL
matrix_size = erpc_matrix_multiply.common.matrix_size

## Create matrix with specified size and random data
def fillMatrix(size, maxRandomValue):
    return [[random.randrange(maxRandomValue) for x in range(size)] for e in range(size)]

## Print matrix
def printMatrix(matrix):
    for i in range(len(matrix)):
        for j in range(len(matrix[0])):
            print('%04d' % matrix[i][j], end=" ")
        print()


###############################################################################
# Client
###############################################################################

## Run client on specified transport layer
def runClient(transport):
    # define maximum random value matrix can have
    MAX_VALUE = 50

    # create matrix multiply eRPC service
    clientManager = erpc.client.ClientManager(transport, erpc.basic_codec.BasicCodec)
    client = erpc_matrix_multiply.client.MatrixMultiplyServiceClient(clientManager)

    while True:
        # create matrices with random values
        matrix1 = fillMatrix(matrix_size, MAX_VALUE)
        matrix2 = fillMatrix(matrix_size, MAX_VALUE)

        # print matrices to the console
        print('\r\nMatrix #1\r\n=========')
        printMatrix(matrix1)

        print('\r\nMatrix #2\r\n=========')
        printMatrix(matrix2)

        # create result matrix as eRPC reference object
        resultMatrix = erpc.Reference()

        # send request to the server
        print('\r\neRPC request is sent to the server')
        client.erpcMatrixMultiply(matrix1, matrix2, resultMatrix)

        # print result matrix's value
        print('\r\nResult matrix\r\n=========')
        printMatrix(resultMatrix.value)

        # wait for key press
        input_fn('\r\nPress Enter to initiate the next matrix multiplication')

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":

    try:
        # get RPMSG device
        rpmsgDev = ""
        if len(sys.argv) <= 1:
            rpmsgDev = "/dev/ttyRPMSG"
        else:
            rpmsgDev = sys.argv[1]
        if not os.path.exists(rpmsgDev):
            raise Exception("RPMSG device not found. Missing right '/dev/ttyRPMSGxxx' cmd parameter.")

        # initialize ttyRPMSG transport layer
        transport = erpc.transport.SerialTransport(rpmsgDev, 115200)
        print('Selected ttyRPMSG Transport')

        # run server/client with ttyRPMSG transport layer
        runClient(transport)
    except SerialException:
        print('Error!')