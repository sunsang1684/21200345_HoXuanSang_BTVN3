from PIL import Image
import numpy as np
import os

# List of image names (without extensions)
image_names = [
    '64x64-Angelina Jolie',
    '64x64-Cristiano Ronaldo',
    '64x64-Jerry Seinfeld',
    '64x64-Kendall Jenner',
    '64x64-Leonardo DiCaprio',
    '64x64-Lionel Messi',
    '64x64-Michael Jeffrey Jordan',
    '64x64-Robin Williams',
    'bike'
]

# Directories for input and output
input_dir = 'image_test/'
output_dir_image = 'image_test_gs/'
output_dir_text = 'image_test_gs/'

# input_dir = 'image_source/'
# output_dir_image = 'image_source_gs/'
# output_dir_text = 'image_source_gs/'

# Ensure the output directories exist
os.makedirs(output_dir_image, exist_ok=True)
os.makedirs(output_dir_text, exist_ok=True)

for image_name in image_names:
    # Import images
    image_path = os.path.join(input_dir, f"{image_name}.jpg")
    image = Image.open(image_path)

    # Convert image to numpy array
    image_np = np.array(image)

    # Calculate gray scale
    gray_image = 0.299 * image_np[:, :, 0] + 0.587 * image_np[:, :, 1] + 0.114 * image_np[:, :, 2]

    # Translate result to uint8 (0-255)
    gray_image = gray_image.astype(np.uint8)

    # Save grayscale image
    gray_image_path = os.path.join(output_dir_image, f"{image_name}.png")
    Image.fromarray(gray_image).save(gray_image_path)

    # Save grayscale image as text
    gray_image_text_path = os.path.join(output_dir_text, f"{image_name}.txt")
    height, width = gray_image.shape
    with open(gray_image_text_path, 'w') as f:
        for i in range(height):
            for j in range(width):
                f.write(f"{gray_image[i][j]} ")
            f.write("\n")

    print(f"Processed {image_name}")

print("All images processed successfully!")
