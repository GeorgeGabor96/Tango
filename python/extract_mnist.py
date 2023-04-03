import os
import cv2 as cv
import numpy as np


MNIST_FOLDER = "D:/datasets/MNIST"
OUT_FOLDER = "D:/datasets/MNIST/extracted"
os.makedirs(OUT_FOLDER, exist_ok=True)
OUT_FOLDER_IMG = os.path.join(OUT_FOLDER, 'img')
os.makedirs(OUT_FOLDER_IMG, exist_ok=True)


def extract(data_file, label_file):
    data = open(data_file, 'rb').read()
    labels = open(label_file, 'rb').read()
    data = data[16:]
    labels = labels[8:]

    labels_file = 'train_labels.txt'
    with open(labels_file, 'w') as fp:
        for label in labels:
            fp.write(f'{label}\n')

    for idx in range(len(labels)):
        img = np.empty((28 * 28,), dtype=np.uint8)
        for i in range(28*28):
            img[i] = data[i]
        data = data[28*28:]

        img = np.reshape(img, (28, 28))
        cv.imwrite(os.path.join(OUT_FOLDER_IMG, f"{idx}.png"), img)


def main():
    train_data_file = os.path.join(MNIST_FOLDER, "train-images.idx3-ubyte")
    train_label_file = os.path.join(MNIST_FOLDER, "train-labels.idx1-ubyte")
    extract(train_data_file, train_label_file)


if __name__ == '__main__':
    main()
