import cv2
import os

image_folder = 'd:/repos/Tango_outputs/experimenting/10_exp_200_steps/selected' # make sure to use your folder
video_path = 'd:/repos/Tango_outputs/experimenting/10_exp_200_steps/mygeneratedvideo.avi'

images = [img for img in os.listdir(image_folder)
            if img.endswith(".jpg") or
                img.endswith(".jpeg") or
                img.endswith("png")]


frame = cv2.imread(os.path.join(image_folder, images[0]))

# setting the frame width, height width
# the width, height of first image
height, width, layers = frame.shape

video = cv2.VideoWriter(video_path, 0, 1, (width, height))

# Appending the images to the video one by one
for image in images:
    video.write(cv2.imread(os.path.join(image_folder, image)))

# Deallocating memories taken for window creation
cv2.destroyAllWindows()
video.release()