# Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
# Copyright 2016 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

#
# Generated by erpcgen 1.13.0 on Thu Apr 18 09:37:38 2024.
#
# AUTOGENERATED - DO NOT EDIT
#

# Abstract base class for MatrixMultiplyService
class IMatrixMultiplyService(object):
    SERVICE_ID = 1
    ERPCMATRIXMULTIPLY_ID = 1

    def erpcMatrixMultiply(self, matrix1, matrix2, result_matrix):
        raise NotImplementedError()


