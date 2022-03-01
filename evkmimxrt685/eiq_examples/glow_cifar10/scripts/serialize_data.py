import tensorflow as tf
import numpy as np


def preprocess(X, scale, offset):
    X = X.astype(np.float)

    # Preprocess for fp
    X -= 127.5  # [0, 255] -> [-127.5, 127.5]
    X /= 127.5  # [-127.5, 127.5] -> [-1, 1]

    # Preprocess for quant
    X /= scale
    X += offset

    return X.astype(np.int8)


if __name__ == '__main__':
    _, (X_test, y_test) = tf.keras.datasets.cifar10.load_data()

    interpreter = tf.lite.Interpreter(model_path='cifarnet_quant_int8.tflite')
    scale, offset = interpreter.get_input_details()[0]['quantization']
   
    # # Keep only some images
    X_test = X_test[:100]
    y_test = y_test[:100]

    # Preprocess
    X_test = preprocess(X_test, scale, offset).reshape(X_test.shape[0], 32, 32, 3)
    y_test = y_test.astype(np.int32)

    # Save binaries
    X_test.tofile('io/input.bin')
    y_test.tofile('io/output.bin')
