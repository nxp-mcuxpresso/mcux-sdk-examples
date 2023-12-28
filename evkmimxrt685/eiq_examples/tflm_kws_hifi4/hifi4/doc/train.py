# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import os
import pathlib

import matplotlib.pyplot as plt
import numpy as np
import tensorflow as tf
import tensorflow_datasets as tfds

from tensorflow.keras import callbacks, layers
from tensorflow.keras import models

waveform_ds = tfds.audio.SpeechCommands(data_dir='data')
waveform_ds.download_and_prepare(download_dir='data')

commands = waveform_ds.info.features['label'].names
print('Commands:', commands)

def get_spectrogram(waveform):
  # Padding for files with less than 16000 samples
  zero_padding = tf.zeros([16000] - tf.shape(waveform), dtype=tf.float32)
  
  # Concatenate audio with padding so that all audio clips will be of the 
  # same length
  waveform = tf.cast(waveform, tf.float32) / (1 << 15)
  equal_length = tf.concat([waveform, zero_padding], 0)
  stfts = tf.signal.stft(
      equal_length, frame_length=640, frame_step=320, fft_length=1024)
  
  spectrograms = tf.abs(stfts)

  # Warp the linear scale spectrograms into the mel-scale.
  num_spectrogram_bins = stfts.shape[-1]
  lower_edge_hertz, upper_edge_hertz, num_mel_bins = 20.0, 4000.0, 40
  linear_to_mel_weight_matrix = tf.signal.linear_to_mel_weight_matrix(
      num_mel_bins, num_spectrogram_bins, 16000, lower_edge_hertz,
      upper_edge_hertz)
  mel_spectrograms = tf.tensordot(
      spectrograms, linear_to_mel_weight_matrix, 1)
  mel_spectrograms.set_shape(spectrograms.shape[:-1].concatenate(
      linear_to_mel_weight_matrix.shape[-1:]))

  # Compute a stabilized log to get log-magnitude mel-scale spectrograms.
  log_mel_spectrograms = tf.math.log(mel_spectrograms + 1e-6)

  # Compute MFCCs from log_mel_spectrograms and take the first 10.
  mfccs = tf.signal.mfccs_from_log_mel_spectrograms(
      log_mel_spectrograms)[..., :10] 

  return mfccs

def get_spectrogram_and_label_id(item):
  spectrogram = get_spectrogram(item['audio'])
  spectrogram = tf.expand_dims(spectrogram, -1)
  label_id = item['label']
  return spectrogram, label_id

train_ds = waveform_ds.as_dataset(split='train').map(
    get_spectrogram_and_label_id, num_parallel_calls=tf.data.AUTOTUNE)

val_ds = waveform_ds.as_dataset(split='validation').map(
    get_spectrogram_and_label_id, num_parallel_calls=tf.data.AUTOTUNE)

test_ds = waveform_ds.as_dataset(split='test').map(
    get_spectrogram_and_label_id, num_parallel_calls=tf.data.AUTOTUNE)

batch_size = 64
train_ds = train_ds.batch(batch_size)
val_ds = val_ds.batch(batch_size)

train_ds = train_ds.cache().prefetch(tf.data.AUTOTUNE)
val_ds = val_ds.cache().prefetch(tf.data.AUTOTUNE)

for spectrogram, _ in train_ds.take(1):
  input_shape = spectrogram.shape[1:]
print('Input shape:', input_shape)
num_labels = len(commands)

def setup_model():
  model = models.Sequential([
      layers.Input(shape=input_shape),

      layers.DepthwiseConv2D((10, 4), strides=(2, 2), padding='same', depth_multiplier=64),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.DepthwiseConv2D((3, 3), padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.Conv2D(64, 1, padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.DepthwiseConv2D((3, 3), padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.Conv2D(64, 1, padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.DepthwiseConv2D((3, 3), padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),
      
      layers.Conv2D(64, 1, padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),

      layers.DepthwiseConv2D((3, 3), padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),

      layers.Conv2D(64, 1, padding='same'),
      layers.BatchNormalization(),
      layers.Activation('relu'),

      layers.AveragePooling2D((25, 5), strides=(2, 2)),

      layers.Flatten(),
      layers.Dense(num_labels),
      layers.BatchNormalization(),

      layers.Softmax()
  ])

  return model

def train_model(model, epochs):
  model.compile(
      optimizer=tf.keras.optimizers.Adam(),
      loss=tf.keras.losses.SparseCategoricalCrossentropy(from_logits=True),
      metrics=['accuracy'],
  )

  checkpoint_best = tf.keras.callbacks.ModelCheckpoint('checkpoint',
      monitor='val_accuracy', verbose=1, save_best_only=True)
  checkpoint = tf.keras.callbacks.ModelCheckpoint('checkpoint/{epoch:02d}-{val_accuracy:.4f}',
      monitor='val_accuracy', verbose=1, save_best_only=False)

  EPOCHS = epochs
  history = model.fit(
      train_ds, 
      validation_data=val_ds,
      epochs=EPOCHS,
      callbacks=[checkpoint_best, checkpoint]
  )

  metrics = history.history
  plt.plot(history.epoch, metrics['loss'], metrics['val_loss'])
  plt.legend(['loss', 'val_loss'])
  plt.savefig('history.png')

if not os.path.exists('checkpoint'):
  model = setup_model()
  model.summary()
  train_model(model, epochs=10)

model = tf.keras.models.load_model('checkpoint')

def test_model():
  test_audio = []
  test_labels = []

  for audio, label in test_ds:
    test_audio.append(audio.numpy())
    test_labels.append(label.numpy())

  test_audio = np.array(test_audio)
  test_labels = np.array(test_labels)

  y_pred = np.argmax(model.predict(test_audio), axis=1)
  y_true = test_labels

  test_acc = sum(y_pred == y_true) / len(y_true)
  print(f'Test set accuracy: {test_acc:.0%}')

test_model()

def representative_dataset_gen():
  for audio, label in val_ds:
    yield [audio]

converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.representative_dataset = representative_dataset_gen

tflite_model = converter.convert()
with open('model.tflite', 'wb') as f:
  f.write(tflite_model)

def test_tflite_model():
  test_labels = []
  test_output = []

  interpreter = tf.lite.Interpreter(model_path="model.tflite")
  interpreter.allocate_tensors()

  input_details = interpreter.get_input_details()
  output_details = interpreter.get_output_details()

  input_shape = input_details[0]['shape']
  input_index = input_details[0]['index']
  output_index = output_details[0]['index']

  for audio, label in test_ds:
    test_labels.append(label.numpy())

    interpreter.set_tensor(input_index, np.reshape(audio, input_shape))

    interpreter.invoke()

    output_data = interpreter.get_tensor(output_index)
    test_output.append(output_data[0])

  test_labels = np.array(test_labels)
  test_output = np.array(test_output)

  y_pred = np.argmax(test_output, axis=1)
  y_true = test_labels

  test_acc = sum(y_pred == y_true) / len(y_true)
  print(f'Quantized model accuracy: {test_acc:.0%}')

test_tflite_model()
