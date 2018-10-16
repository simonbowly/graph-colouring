'''
Downloads and compiles Culberson graph colouring algorithms:
http://webdocs.cs.ualberta.ca/~joe/Coloring/Colorsrc/index.html
Compiled binaries should end up in color/bin/ to be found by algorithms.py

Downloads and extracts evolved graph instances:
http://katesmithmiles.wixsite.com/home/matilda
Instances should end up in graphs/instances/ to be found by evaluate.py
'''

import contextlib
import os
import subprocess
import tarfile
import urllib.request
import zipfile


print('Downloading and building Culberson graph algorithms...')
os.makedirs('color/bin', exist_ok=True)
urllib.request.urlretrieve(
    'http://www.cs.ualberta.ca/~joe/Coloring/Colorsrc/color.tar.gz',
    'color.tar.gz')
with contextlib.closing(tarfile.open('color.tar.gz')) as archive:
    archive.extractall('color')
subprocess.check_call(['make', 'all'], cwd='color')

print('Downloading and extracting evolved graph instances...')
urllib.request.urlretrieve(
    'http://users.monash.edu.au/~ksmiles/matilda/graphs.zip',
    'graphs.zip')
with contextlib.closing(zipfile.ZipFile('graphs.zip')) as archive:
    archive.extractall()
