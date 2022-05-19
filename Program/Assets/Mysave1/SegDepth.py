import numpy as np
import cv2

img = cv2.imread("Fileeee_gta01.png")

gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

blurred = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

#blurred = cv2.GaussianBlur(gray, (5, 5), 0)

canny = cv2.Canny(blurred, 30, 100)
#img =  img + canny
cv2.imshow('Input', img)
cv2.imshow('Result', canny)

cv2.imwrite('Result.png', canny)

cv2.waitKey(0)