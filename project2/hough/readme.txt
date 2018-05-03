This collection of files can be used to implement the Hough transform for the Project 2 assignment. This code taken from http://rosettacode.org/wiki/Example:Hough_transform/C.  Files are ...

main.c: example main that includes the RGB image segment and calls houghTransform()

hough.h: header for hough and loads imageXX.h file
hough.c: contains houghTransform()

image10.h: 10x10 segment of pencils.jpg configured to store in flash memory
image40.h: 40x40 segment of pencils.jpg configured to store in flash memory
image60.h: 60x60 segment of pencils.jpg configured to store in flash memory

testload.h: a test file with printable characters to test the storing and retrieving of data stored in flash memory
