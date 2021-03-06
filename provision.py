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
import pathlib
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

print('Removing some badly specified instances...')
for ind in [3662] + [6029 + i for i in range(10)] + [6139 + i for i in range(4)]:
    with contextlib.suppress(FileNotFoundError):
        os.remove(f'graphs/instances/g{ind:04d}.col')

print('Downloading and building Lewis graph algorithms...')
urllib.request.urlretrieve(
    'http://www.rhydlewis.eu/resources/gCol.zip',
    'gCol.zip')
with contextlib.closing(zipfile.ZipFile('gCol.zip')) as archive:
    archive.extractall()
os.rename('gCol', 'lewis')
makefile = pathlib.Path('lewis/Makefile')
makefile.write_text(makefile.read_text().replace('IteratedGreedy ', ''))
subprocess.check_call(['make', 'all'], cwd='lewis')
