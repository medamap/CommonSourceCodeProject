import sys
import os

from PIL import Image

args = sys.argv
if len(args) != 2:
    quit();

sourceFile = args[1]
machineName = sourceFile.split(".")[0]

sourceImage = Image.open(sourceFile)

#mipmap-hdpi   48x48
#mipmap-mdpi   72x72
#mipmap-xhdpi  96x96
#mipmap-xxhdpi 144x144
#mipmap-xxxhdpi	192 x 192

os.mkdir(machineName)

img_mdpi = sourceImage.resize((48, 48))
os.mkdir(machineName + '/mipmap-mdpi')
img_mdpi.save(machineName + '/mipmap-mdpi/ic_launcher.png')

img_hdpi = sourceImage.resize((72, 72))
os.mkdir(machineName + '/mipmap-hdpi')
img_hdpi.save(machineName + '/mipmap-hdpi/ic_launcher.png')

img_xhdpi = sourceImage.resize((96, 96))
os.mkdir(machineName + '/mipmap-xhdpi')
img_xhdpi.save(machineName + '/mipmap-xhdpi/ic_launcher.png')

img_xxhdpi = sourceImage.resize((144, 144))
os.mkdir(machineName + '/mipmap-xxhdpi')
img_xxhdpi.save(machineName + '/mipmap-xxhdpi/ic_launcher.png')

img_xxxhdpi = sourceImage.resize((192, 192))
os.mkdir(machineName + '/mipmap-xxxhdpi')
img_xxxhdpi.save(machineName + '/mipmap-xxxhdpi/ic_launcher.png')
