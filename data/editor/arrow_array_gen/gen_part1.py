from PIL import Image

def rotate_and_save_image(image_path):
    # Load image
    img = Image.open(image_path)

    for i in range(90):
        # Rotate image
        rotated_img = img.rotate(-i, expand=False, center=(img.width/2, img.height/2))

        # Save rotated image
        rotated_img.save(f"output_{i}.png")

# Specify the path to your image
image_path = "input.png"
rotate_and_save_image(image_path)