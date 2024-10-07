import numpy as np
from PIL import Image

# Đọc dữ liệu từ file .txt chứa các pixel
def read_image_from_txt(filename, width, height):
    image = np.zeros((height, width), dtype=np.uint8)
    
    with open(filename, 'r') as f:
        for i in range(height):
            line = f.readline().strip().split()
            for j in range(width):
                image[i, j] = int(line[j])
    
    return image

# Chuyển đổi mảng pixel thành ảnh và lưu lại
def save_image_from_txt(txt_file, output_image, width, height):
    image_array = read_image_from_txt(txt_file, width, height)
    
    # Tạo hình ảnh từ mảng
    image = Image.fromarray(image_array)
    
    # Lưu hình ảnh
    image.save(output_image)
    print(f"Image saved as {output_image}")

# Danh sách các tệp .txt để xử lý
txt_files = [
    'preliminary_edge_flag_H.txt',
    'preliminary_edge_flag_V.txt',
    'preliminary_edge_flag_P.txt',
    'preliminary_edge_flag_M.txt'
]

# Thay đổi tên file đầu ra theo tên file txt
for txt_file in txt_files:
    # Tạo tên file đầu ra từ tên file txt
    output_image = txt_file.replace('.txt', '.png')
    save_image_from_txt(txt_file, output_image, width=64, height=64)
