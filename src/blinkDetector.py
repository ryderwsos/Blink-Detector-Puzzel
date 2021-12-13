# blinkDetector.py
# Written by Aryan Prasad
# Adapted from https://www.pyimagesearch.com/2017/04/24/eye-blink-detection-opencv-python-dlib/

from scipy.spatial import distance
from imutils.video import VideoStream
from imutils import face_utils
import numpy as np
import imutils
import cv2
import dlib
import RPi.GPIO as GPIO

# initialize Board Pin 12, starting with an output of low
pin_out = 12
GPIO.setmode(GPIO.BOARD)
GPIO.setup(pin_out, GPIO.OUT, initial = GPIO.LOW)

# calculates the "openness" of the eye
def eye_aspect_ratio(eye):
	A = distance.euclidean(eye[1], eye[5]) # vertical distance between points on eyelid
	B = distance.euclidean(eye[2], eye[4]) # same as above but with different points
	C = distance.euclidean(eye[0], eye[3]) # horizontal length of eye 

	ear = (A + B)/(2.0*C)
	return ear

# critical value for EAR to be below for a blink to register
EAR_THRESHOLD = .2

# reduces noise by returning true if at least 2 of 3 previous detections have been below the threshold
def check_hist(EAR, EAR_hist):
	indicator = 0 
	for i in EAR_hist:
		if i < EAR_THRESHOLD:
			indicator += 1
	return (EAR < EAR_THRESHOLD + .05 and indicator > 1)

# initialize dlib's detector with this file containing a trained model
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")

# allows for easy seperation of landmarks for left and right eye
(lStart, lEnd) = face_utils.FACIAL_LANDMARKS_IDXS["left_eye"]
(rStart, rEnd) = face_utils.FACIAL_LANDMARKS_IDXS["right_eye"]

# initialize the Pi Camera with this string
camConfig = 'nvarguscamerasrc !  video/x-raw(memory:NVMM), width=3264, height=2464, format=NV12, framerate=28/1 ! nvvidconv flip-method='+str(2)+' ! video/x-raw, width='+str(320)+', height='+str(240)+', format=BGRx ! videoconvert ! video/x-raw, format=BGR ! appsink'
cam = cv2.VideoCapture(camConfig)
fileStream = False # letting the Jetson know that we are not reading input frmo a file

EAR_history = [1, 1, 1] 

# print("Confirming that the program has run.")

# infinite loop that detects blinks
while True:
	# filler captures the first (extraneous) output of cam.read()
	filler, frame = cam.read()
	# retouches the frame to simplify detection
	frame = imutils.resize(frame, width = 450)
	frame = cv2.rotate(frame, cv2.ROTATE_180)
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	# creates a list of detected faces in the frame
	faces = detector(gray, 0)

	# loop through detected faces
	for (i, face) in enumerate(faces):
		# detects facial landmarks and converts them to a NumPy array
		shape = predictor(gray, face)
		shape = face_utils.shape_to_np(shape)

		# calculates EAR for both eyes and then averages it
		leftEye = shape[lStart:lEnd]
		rightEye = shape[rStart:rEnd]
		leftEAR = eye_aspect_ratio(leftEye)
		rightEAR = eye_aspect_ratio(rightEye)

		EAR = (leftEAR + rightEAR)/2.0
		
		# sends output only if EAR (current or historical) is sufficiently low
		if EAR < EAR_THRESHOLD or (check_hist(EAR, EAR_history)):
			GPIO.output(pin_out, GPIO.HIGH)
			# print("Blink DETECTED with EAR: " + str(EAR))
		else:
			GPIO.output(pin_out, GPIO.LOW)
			# print("Blink not detected with EAR: " + str(EAR))

		# updates EAR history
		EAR_history[0] = EAR_history[1]
		EAR_history[1] = EAR_history[2]
		EAR_history[2] = EAR
	
	# allows for exiting the loop - more of a formality
	if cv2.waitKey(1) == ord("q"):
		break

# cleanup
cam.release()
GPIO.cleanup()
