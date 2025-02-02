import cv2
import os
import argparse


def get_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--image_folder", required=True, type=str)
    parser.add_argument("--video_path", required=True, type=str)
    return parser.parse_args()


def main():
    args = get_args()
    images = [img for img in os.listdir(args.image_folder) if img.endswith(".jpg") or img.endswith(".jpeg") or img.endswith(".png")]

    frame = cv2.imread(os.path.join(args.image_folder, images[0]))

    height, width, layers = frame.shape
    video = cv2.VideoWriter(args.video_path, 0, 1, (width, height))

    for image in images:
        video.write(cv2.imread(os.path.join(args.image_folder, image)))

    cv2.destroyAllWindows()
    video.release()


if __name__ == '__main__':
    main()
