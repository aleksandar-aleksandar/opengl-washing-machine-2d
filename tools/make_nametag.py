"""
Generates the name tag texture (textures/nametag.png): your name and student ID
on a dark, semi-transparent background.

1. Put your info in the two lines below
2. Run from the project folder:   py tools/make_nametag.py

Needs Pillow (py -m pip install --user pillow) and uses the Arial font that comes with Windows.
"""

import os
from PIL import Image, ImageDraw, ImageFont

NAME = "Ime Prezime"        # <-- your first and last name
STUDENT_ID = "SV 00/2025"   # <-- your student ID

WIDTH, HEIGHT = 640, 160
BACKGROUND = (0, 0, 0, 150)          # black, about 60% opaque
TEXT_COLOR = (255, 255, 255, 255)    # white

if __name__ == "__main__":
    image = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))   # start fully transparent
    draw = ImageDraw.Draw(image)

    draw.rounded_rectangle((0, 0, WIDTH - 1, HEIGHT - 1), radius=24, fill=BACKGROUND)

    name_font = ImageFont.truetype("C:/Windows/Fonts/arialbd.ttf", 56)   # Arial Bold
    id_font = ImageFont.truetype("C:/Windows/Fonts/arial.ttf", 44)       # Arial
    draw.text((WIDTH / 2, 55), NAME, font=name_font, fill=TEXT_COLOR, anchor="mm")
    draw.text((WIDTH / 2, 115), STUDENT_ID, font=id_font, fill=TEXT_COLOR, anchor="mm")

    path = os.path.join(os.path.dirname(__file__), "..", "textures", "nametag.png")
    image.save(path)
    print("wrote", os.path.normpath(path))
