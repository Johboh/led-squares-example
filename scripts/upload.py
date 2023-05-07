#!/bin/python3

import os, sys, glob, struct, argparse, requests

ROOT_PATH = os.path.dirname(os.path.realpath(__file__))
sys.path.append("%s/shared" % ROOT_PATH)
from shared.Converter import Converter

WIDTH_IN_PIXELS = 56
HEIGHT_IN_PIXELS = 40

converter = Converter(WIDTH_IN_PIXELS, HEIGHT_IN_PIXELS)

def addMediaHeader(data, number_of_frames):
    data += struct.pack("<H", number_of_frames)

def addFrameHeader(data, delay_in_ms):
    data += struct.pack("<H", delay_in_ms)

def writeBytesToTempFile(data, path):
    print("Will write %d bytes" % (len(data)))
    with open(path, 'wb') as f:
        bytes_objs = bytes(data)
        f.write(bytes_objs)

def upload(filename):
    with open(filename, 'rb') as f:
        files = {'file': f}
        print(requests.post(args.base_url + "/upload", files=files))

parser = argparse.ArgumentParser(description='Upload static image or gif')
parser.add_argument('-d', '--delays')
parser.add_argument('-u', '--base_url', required = True)
parser.add_argument('-f', '--filename', required = True)
parser.add_argument('tempdir')
args = parser.parse_args()

data = []

# We assume that if we have delays specified, its an animation.
if args.delays != None:
    print("Processing animation...")
    # Get all images.
    list_of_files = sorted( filter( os.path.isfile, glob.glob(args.tempdir + '/*.bmp') ), key=lambda x: int(os.path.basename(x).split("_")[1].split(".")[0]))

    delays = []
    with open(args.delays, 'rb') as f:
        delays = f.read().splitlines()

    addMediaHeader(data, len(list_of_files))
    for index, file_path in enumerate(list_of_files):
        addFrameHeader(data, int(delays[index])*10)
        rows = converter.read_rows(file_path)
        converter.repack_sub_pixels(data, rows)

    print("Stats: %d number of frames, delays: %s" % (len(list_of_files), list(map(lambda x: int(x)*10, delays))))
else:
    print("Processing static image...")
    addMediaHeader(data, 1)
    addFrameHeader(data, 0)
    rows = converter.read_rows(os.path.join(args.tempdir, "output.bmp"))
    converter.repack_sub_pixels(data, rows)

path = os.path.join(args.tempdir, args.filename)
writeBytesToTempFile(data, path)

upload(path)