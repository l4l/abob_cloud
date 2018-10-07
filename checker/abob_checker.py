#!/bin/python2

from pwn import *
from PIL import Image
from random import randrange
from struct import pack
from hashlib import sha256
from StringIO import StringIO

FLAG_SIZE = 48

def gen_matrix():
  return [(randrange(0, 256), randrange(0, 256), randrange(0, 256), randrange(0, 256)) for i in range(FLAG_SIZE * FLAG_SIZE)]

def gen_image():
  img = Image.new('RGBA', (FLAG_SIZE, FLAG_SIZE), color='white')
  img.putdata(gen_matrix())
  return img

def get_flag(img_dat):
  (width, height) = (FLAG_SIZE, FLAG_SIZE)
  folding = [0 for i in range(width * 4)]
  for i in range(width):
    for j in range(height):
      for byte in range(4):
        c = img_dat[i * height * 4 + j * 4 + byte]
        folding[i * 4 + byte] ^= ord(c)

  flag = []
  for i in range(width):
    k = 0
    for byte in range(4):
      k ^= folding[i * 4 + byte]
    flag.append(chr(k))
  return ''.join(flag)

def push(host, ip, raw_img):
  with remote(host, ip) as r:
    r.send(pack('<I', len(raw_img)))
    r.send(raw_img)

def pull(host, ip, raw_img):
  h = sha256(raw_img).hexdigest()
  with remote(host, ip, typ='udp') as r:
    r.send(h)
    return r.recvn(FLAG_SIZE)
  return None

def get_raw_img(img):
  s = StringIO()
  img.save(s, 'png')
  s.seek(0)
  return s.buf
