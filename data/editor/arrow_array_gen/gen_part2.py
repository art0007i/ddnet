from PIL import Image

def create_texture_atlas():
    # Get the dimensions of one of the images
    single_img = Image.open("output_0.png")
    img_width, img_height = single_img.size

    # Create a new image with dimensions 16 times the width and height of a single image
    atlas_width = img_width * 16
    atlas_height = img_height * 16
    texture_atlas = Image.new('RGBA', (atlas_width, atlas_height))

    # Loop through and paste each image into the correct position in the grid
    for i in range(90):
        img = Image.open(f"output_{i}.png")
        row = i // 16  # Integer division to get the row number
        col = i % 16  # Remainder to get the column number
        x = col * img_width
        y = row * img_height
        texture_atlas.paste(img, (x, y))

    # Save the texture atlas
    texture_atlas.save("texture_atlas.png")

# Call the function to create the texture atlas
create_texture_atlas()