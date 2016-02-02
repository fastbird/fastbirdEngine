#!/usr/bin/env python
# GIMP plugin to export layers as PNGs, then run Sprite Sheet Packer on the
# exported images to generate a texture atlas and map.
# Author: Pravin Kumar (Aralox) <aralox@gmail.com>
# Copyright 2012 Pravin Kumar
# License: GPL v3+
# Version 1.0
# For the methods export_layers, get_layers_to_export and format_filename:
#   Original Author: Chris Mohler <cr33dog@gmail.com> (Copyright 2009)

import os;
import subprocess
from gimpfu import *

def export_to_ssp(img, path, only_visible, flatten, sspack_dir, texname, mapname):

    #create a folder to save exported layers in
    img_dir = os.path.join(path,"Exported Layers")
    if not os.path.exists(img_dir):
        os.makedirs(img_dir)

    #export layers to files
    imglist_name = os.path.join(os.getcwd() , "ImageList.txt")
    imglist_file = open(imglist_name, 'w')
    export_layers(img, img_dir, only_visible, flatten, imglist_file)
    imglist_file.close()

    #call sprite sheet packer (command line)
    sspack = os.path.join(sspack_dir, 'sspack.exe')
    if os.path.exists(sspack):

        param_img = '/image:' + os.path.join(path, texname) + '.png'   #eg. /image:C:\Users\Pravin\Desktop\test.png
        param_map = '/map:' + os.path.join(path, mapname) + '.txt'    #eg. /map:C:\Users\Pravin\Desktop\test_map.skin
        param_maxw = '/mw:4096' #default
        param_maxh = '/mh:4096' #default
        param_pad = '/pad:1'    #default
        param_imglist = '/il:' + imglist_name   #eg /il:C:\Users\Pravin\AppData\Roaming\SpriteSheetPacker\FileList.txt 
        subprocess.call([sspack, param_img, param_map, param_maxw, param_maxh, param_pad, param_imglist])        
    else:
        pdb.gimp_message("sspack.exe was not found in that directory. Texture atlas and map were not created.")

    #save configurations for convenience
    config = open("GimpToSSP.cfg", 'w')
    config.write(sspack_dir)
    config.close()

    return


def get_sspack_dir():

    filename = "GimpToSSP.cfg"

    #Gets previously saved location of sspack from config file if available
    if os.path.exists(filename):
        config = open(filename, "r")
        sspack_dir = config.read()
        config.close()
        return sspack_dir
    else:
        return os.getcwd()


#original version also had regex commands in here to clean out whitespace
def format_filename(layer):
    layername = layer.name.decode('utf-8')
    filename = layername + '.png'
    return filename


def get_layers_to_export(img, only_visible):
    layers = []

    #Add to list of layers to export depending on visibility
    for layer in img.layers:
        if only_visible and layer.visible:
            layers.append(layer)
        if not only_visible:
            layers.append(layer)

    return layers


#Hide all layers, then show each layer one at a time and save image
#Layer groups are treated as one layer. I cant find enough documentation on layer group functions to do anything else right now
def export_layers(img, path, only_visible, flatten, file):

    #Use a duplicate image cos we dont want to mess up orig layers (visiblity/flattening)
    dupe = img.duplicate()
    savelayers = get_layers_to_export(dupe, only_visible)

    #Hide all layers first
    for layer in dupe.layers:
        layer.visible = 0 

    #Show each layer at a time, and save
    for layer in dupe.layers:
        if layer in savelayers:
            layer.visible = 1

            #Generate filename, and write it to a log of exported files (for sspack later)
            filename = format_filename(layer)
            fullpath = os.path.join(path, filename);
            file.write(fullpath + '\n')

            #use a a dupe again, so if we want to flatten (replace alpha with back color) we dont mess it up for the others
            tmp = dupe.duplicate()  
            if (flatten):
                tmp.flatten()

            #see the procedure browser under the help menu in gimp, for info on this function
            pdb.file_png_save(tmp, layer, fullpath, filename, 0, 9, 1, 1, 1, 1, 1)


#see gimpfu.py for info on register()
register(
    proc_name = "export-to-sprite-sheet-packer",
    blurb =     "Export layers and run Sprite Sheet Packer",
    help =      "Export layers to png files, and run Sprite Sheet Packer to generate a texture atlas image and map",
    author =    "Pravin Kumar aka. Aralox",
    copyright = "Pravin Kumar aka. Aralox",
    date =      "April 2012",
    imagetypes = "*",      # Alternatives: use RGB, RGB*, GRAY*, INDEXED etc.
    function =  export_to_ssp,
	menu =      "<Image>/File/E_xport Layers",
    label =     "E_xport and run Sprite Sheet Packer...",

    #params are tuples of the form (type, name, description, default [, extra])
    params = [
        #Image export parameters
        (PF_IMAGE, "img", "Image", None),
        (PF_DIRNAME, "path", "Save PNGs here", os.getcwd()),
        (PF_BOOL, "only_visible", "Only Visible Layers?", True),
        (PF_BOOL, "flatten", "Flatten Images? (Replaces alpha channel with background color)", False),

        #Sprite sheet packer parameters
        (PF_DIRNAME, "sspack_dir", "Directory of Sprite Sheet Packer", get_sspack_dir()),
        (PF_STRING, "texname", "Texture Atlas Name (this png will be referenced inside map)", "atlas"),
        (PF_STRING, "mapname", "Map Name (skin)", "map")
    ],

    results =   []
	#other parameters of register(): domain, on_query, on_run
	)

main()

#Here for gimp module documentation: http://developer.gimp.org/api/2.0/index.html
#Go to help->procedure browser in Gimp for help on pdb functions
#Note about 'drawables' from http://www.gimp.org/docs/plug-in/sect-image.html:
# "We have all sorts of silly things like masks, channels, and layers,
# but they're all just a bunch of pixels that can be drawn on, so we treat them much the same
# and lump them all in to the category of "drawables". 
# And an image, then, is just what you get when you put some drawables together."