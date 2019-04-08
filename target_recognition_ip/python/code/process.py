#!/usr/bin/python3

# This code assumes that you have a dataset directory with at least
# one jpeg image in the directory

# In case you want to see the intermediate steps
show_intermediate = 0


# Libraries
import numpy as np
import os, sys
import glob
from scipy import misc              #scipy has deprecated dependency on pillow : pip3 install pillow
import matplotlib.pyplot as plt
from scipy import ndimage
from scipy.ndimage.measurements import label
from scipy.ndimage.measurements import center_of_mass

###################################################################################
# Function

# Get the dir this program is running in
def get_script_path():
    return os.path.dirname(os.path.realpath(sys.argv[0]))


# Get files in dataset directory
def get_test_files():
    
    path = get_script_path()
    path = path.replace('code','dataset') + "/*.jpg"
    files =  (glob.glob(path))
    return files


# Load a JPEG into a numpy array
def load_image_into_rgb_array( path_to_input_image ):
    image = misc.imread(path_to_input_image)
    return image


# Convert an Image from RGB into YUV
def convert_to_YUV(image):

   R = image[:,:,0]
   G = image[:,:,1]
   B = image[:,:,2]

   Y = (R + 2*G + B) / 4
   U = R - G
   V = B - G

   image[:,:,0] = Y
   image[:,:,1] = U
   image[:,:,2] = V

   return image

# Find Skin in the Image ----> will probably need to play with tese parameters more
def filter_for_skin(image):

    U_max = 74
    U_min = 10
    V_max = 11
    V_min = -40

    Y_filtered = image[:,:,0] * 0    # according to research this value shouldn't matter much
    U_filtered = (image[:,:,1] < U_max) * image[:,:,1] * (image[:,:,1] > U_min)
    V_filtered = (image[:,:,2] < V_max) * image[:,:,2] * (image[:,:,2] > V_min)
       
    image[:,:,0] = Y_filtered;
    image[:,:,1] = U_filtered;
    image[:,:,2] = V_filtered;

    return image

# More skin filtering
def second_skin_filter(image):

    image = image[:,:,1]     #seems in my test image U is the only channel that matters
    return image


# Dilation Filter (fill in holes in shapes)
def dilation_filter(image):
    
    # https://blog.ostermiller.org/dilate-and-erode -> This blog has how to implement this
    # express mode for now
    image = ndimage.binary_opening(image, structure=np.ones((8,8))).astype(np.int)     
    return image
 
    
# Erode Filter (trim back the soft edges of shapes)
def erode_filter(image):

    eroded = ndimage.binary_erosion(image)
    temp = ndimage.binary_propagation(eroded, mask=image)
    return temp


# Find Connected Regions in Imagea
def find_connected_regions(image):

    structure = np.ones((3,3), dtype=np.int)
    labeled, ncomponents = label(image, structure)
    return labeled


# Find Centroids of Connected Regions -> default threshold
def find_centroids(image, threshold=25000):

    surface_areas = np.bincount(image.flat)[1:]
    worthwhile = np.argwhere(surface_areas > threshold)

    ncomponents =  len(np.unique(image)) - 1
    centroids = np.array(center_of_mass(image, labels=image, index=range(1,ncomponents+1)))
    targets = centroids[worthwhile][:,0,:]
    return targets



###################################################################################
if __name__ == "__main__":

    files = get_test_files()
    list_of_images = []

    # Build list of RGB Images
    for target in files:
        image = load_image_into_rgb_array(target)
        list_of_images.append(image)


    # Convert Images into YUV
    for i in range(len(list_of_images)):
        list_of_images[i] = convert_to_YUV(list_of_images[i])
    
    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()


    # Apply Skin Thresholding
    for i in range(len(list_of_images)):
        list_of_images[i] = filter_for_skin(list_of_images[i])

    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()


    # Additional Filtering for Skin
    for i in range(len(list_of_images)):
        list_of_images[i] = second_skin_filter(list_of_images[i])

    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()


    # Apply Dilation Filter to Image (fill in holes in shapes)
    for i in range(len(list_of_images)):
        list_of_images[i] = dilation_filter(list_of_images[i])

    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()
    

    # Apply Erode Filter to Image (trim shapes to edges)
    for i in range(len(list_of_images)):
        list_of_images[i] = erode_filter(list_of_images[i])

    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()


    # Identify Connected Regions in Image
    for i in range(len(list_of_images)):
        list_of_images[i] = find_connected_regions(list_of_images[i])

    if(show_intermediate):
        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()


    # Find Centroids of regions with a given area over a threshold
    list_of_image_targets = []

    for i in range(len(list_of_images)):
        list_of_image_targets.append(find_centroids(list_of_images[i]))

    if(1):

        targets = list_of_image_targets[0]
        print(targets)
        y = [int(i[0]) for i in targets]
        x = [int(i[1]) for i in targets]
       
        plt.scatter(x,y, s=150, marker='x', color='r')
        plt.gca().invert_xaxis()
        plt.gca().invert_yaxis()

        temp = list_of_images[0]
        plt.imshow(temp)
        plt.show()
   
  

    # For video we would probably do some time averaging on these targets (over 3-4 frames)
    # Would help deal with lighting flicker

