import cv2 as cv
import numpy as np
import os


def read_image(img_path):
    img = cv.imread(img_path, -1)
    return img

def read_img_as_gray(img_path):
    img = read_image(img_path)
    if not (img.ndim == 2 or img.shape[2] == 1):
        img = cv.cvtColor(img, cv.COLOR_BGR2GRAY)
    return img

class ImgSpikePairs:
    def __init__(self, w, h, spike_pairs):
        self.type = 0
        self.n_neurons = w * h
        self.time_max = 256
        self.w = w
        self.h = h
        self.spike_pairs = spike_pairs


def encode_spikes_with_roc(img):
    assert img.ndim == 2 or img.shape[2] == 1

    h = img.shape[0]
    w = img.shape[1]

    img_flat = np.reshape(img, (img.shape[0] * img.shape[1]))
    spike_pairs = []

    for i in range(img_flat.shape[0]):
        spike_pairs.append((i, int(255 - img_flat[i])))

    return ImgSpikePairs(w, h, spike_pairs)


def write_spike_pairs(spike_pairs, out_path):
    os.makedirs(os.path.basename(out_path), exist_ok=True)
    with open(out_path, 'wb') as fp:
        fp.write(spike_pairs.time_max.to_bytes(4, 'little'))
        fp.write(spike_pairs.n_neurons.to_bytes(4, 'little'))

        fp.write(spike_pairs.type.to_bytes(4, 'little'))
        fp.write(spike_pairs.h.to_bytes(4, 'little'))
        fp.write(spike_pairs.w.to_bytes(4, 'little'))

        fp.write(len(spike_pairs.spike_pairs).to_bytes(4, 'little'))
        for spike_pair in spike_pairs.spike_pairs:
            fp.write(spike_pair[0].to_bytes(4, 'little'))
            fp.write(spike_pair[1].to_bytes(4, 'little'))


def main():
    img_path = "d:/datasets/MNIST/extracted/img_train/9999.png"
    out_path = "d:/datasets/MNIST/encoding/img_train/9999.bin"
    img = read_img_as_gray(img_path)
    encoding = encode_spikes_with_roc(img)
    write_spike_pairs(encoding, out_path)


if __name__ == '__main__':
    main()
