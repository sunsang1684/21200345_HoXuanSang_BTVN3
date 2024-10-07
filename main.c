#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 64
#define HEIGHT 64

void read_image_from_txt(const char *filename, int image [WIDTH][HEIGHT]){
    FILE *file =fopen(filename, "r");
    if (file == NULL) { 
        printf("Can not open file %s\n", filename);
        exit(1);
    }

    for (int i = 0; i < HEIGHT; i++){
        for (int j = 0; j < WIDTH; j++){
            if(fscanf(file, "%d", &image[i][j]) != 1){
                printf("Error reading data at pixel [%d, %d]\n", i, j);
                fclose(file);
                exit(1);
            }
        }
    }
    fclose(file);
}

void write_image_to_txt(const char *filename, int image[WIDTH][HEIGHT]){
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Can not open file %s to write\n", filename);
        exit(1);
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fprintf(file, "%d ", image[i][j]);
        }
        fprintf(file, "\n");  // New line after each row
    }

    fclose(file);
}


#define KERNEL_SIZE 5
int K_H[KERNEL_SIZE][KERNEL_SIZE] = { 
    {0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0},
    {-1, -1, -1, -1, -1},
    {0, 0, 0, 0, 0}
};
    
int K_P[KERNEL_SIZE][KERNEL_SIZE] = { 
    {0, 0, 0, 1, 0},
    {0, 1, 1, 0, -1},
    {0, 1, 0, -1, 0},
    {1, 0, -1, -1, 0},
    {0, -1, 0, 0, 0}
};

int K_V[KERNEL_SIZE][KERNEL_SIZE] = { 
    {0, 1, 0, -1, 0},
    {0, 1, 0, -1, 0},
    {0, 1, 0, -1, 0},
    {0, 1, 0, -1, 0},
    {0, 1, 0, -1, 0}
};

int K_M[KERNEL_SIZE][KERNEL_SIZE] = { 
    {0, -1, 0, 0, 0},
    {1, 0, -1, -1, 0},
    {0, 1, 0, -1, 0},
    {0, 1, 1, 0, -1},
    {0, 0, 0, 1, 0}
};

// pixel-by-pixel spatial filtering operations
void apply_spatial_filter(int width, int height, int image[width][height], int kernel[KERNEL_SIZE][KERNEL_SIZE], int output[width][height]){
    int half_size = KERNEL_SIZE / 2;

    // Iterate through each pixel of the image
    for(int x = half_size; x < width - half_size; x++){
        for(int y = half_size; y < height - half_size; y++){
            double result = 0.0;
            // Iterate through each element in the 5x5 kernel
            for(int i = -half_size; i <= half_size; i++){
                for(int j = -half_size; j <= half_size; j++){
                    result += kernel[i + half_size][j + half_size] * image[x + i][y + j];
                
                }
            }
            
            // Assign the result to the output pixel (round to integer)
            output[x][y] = (int) abs(result);
        }
    }
}

void compute_preliminary_edge_flags(int width, int height, 
                                int I_H[width][height], 
                                int I_V[width][height], 
                                int I_P[width][height], 
                                int I_M[width][height], 
                                int flag_H[width][height],
                                int flag_V[width][height],
                                int flag_P[width][height],
                                int flag_M[width][height]){
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            // Lấy giá trị gradient theo các hướng
            int G_H = I_H[x][y]; 
            int G_V = I_V[x][y]; 
            int G_P = I_P[x][y]; 
            int G_M = I_M[x][y]; 
            //
            int max_gradient = G_H;
            if (G_V > max_gradient) max_gradient = G_V;
            if(G_P > max_gradient) max_gradient = G_P;
            if(G_M > max_gradient) max_gradient = G_M;
            //
            flag_H[x][y] = (G_H == max_gradient) ? 1 : 0;
            flag_V[x][y] = (G_P == max_gradient) ? 1 : 0;
            flag_P[x][y] = (G_V == max_gradient) ? 1 : 0;
            flag_M[x][y] = (G_M == max_gradient) ? 1 : 0;
        }
    }
}


void compute_threshold(int width, int height, int image[width][height] ,int threshold[width][height]){
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){

            int values[40];
            int index = 0;
            //
            for (int n = -2; n <= 1; n++){
                for (int m = -2; m <= 2; m++){
                    if(x + n >=0 && x + n + 1 < width && y + m >= 0 && y + m < height){
                        values[index++] = abs(image[x + n + 1][y + m] - image[x + n][y + m]);
                    }
                }
            }
            //
            for (int n = -2; n <= 1; n++){
                for (int m = -2; m <= 2; m++){
                    if(x + m >=0 && x + m < width && y + n >= 0 && y + n + 1 < height){
                        values[index++] = abs(image[x + m][y + n + 1] - image[x + m][y + n]);
                    }
                }
            }
            //Bubble Sort
            for (int i = 0; i < index; i++) {
                for (int j = i + 1; j < index; j++){
                    if(values[i] > values[j]){
                        int temp = values[i];
                        values[i] = values[j];
                        values[j] = temp;
                    }
                }
            }
            // Compute median
            int median;
            if(index % 2 == 0){
                median = (values[index/2 - 1] + values[index/2]) / 2;
            } else {
                median = values[index/2];
            }

            //threshold
            threshold[x][y] = median * 5;
        }
    }
}

void compute_directional_edge_map(int width, int height,
                                int flag_H[width][height], 
                                int flag_V[width][height], 
                                int flag_P[width][height], 
                                int flag_M[width][height], 
                                int I_H[width][height],
                                int I_V[width][height],
                                int I_P[width][height],
                                int I_M[width][height],
                                int F_H[width][height],
                                int F_V[width][height],
                                int F_P[width][height],
                                int F_M[width][height],
                                int threshold[width][height]){
    for (int x = 0; x < width; x++){
        for (int y = 0; y < height; y++){
            F_H[x][y] = (I_H[x][y] > threshold[x][y]) ? flag_H[x][y] : 0;
            F_V[x][y] = (I_V[x][y] > threshold[x][y]) ? flag_V[x][y] : 0;
            F_P[x][y] = (I_P[x][y] > threshold[x][y]) ? flag_P[x][y] : 0;
            F_M[x][y] = (I_M[x][y] > threshold[x][y]) ? flag_M[x][y] : 0;
        }
    }
}

#define CELL_SIZE 16
#define NUM_CELLS 4

// 
void count_edge_flags(int width, int height, int F[width][height], int feature_vector[NUM_CELLS * NUM_CELLS]){
    int index = 0;

    //
    for (int a = 0; a < NUM_CELLS; a++){
        for (int b =0; b < NUM_CELLS; b++){
            int count =0;
            //
            for (int i = a* CELL_SIZE; i < (a + 1) * CELL_SIZE; i++){
                for (int j = b * CELL_SIZE; j < (b + 1) * CELL_SIZE; j++){
                    if(F[i][j] == 1){
                        count++;
                    }
                }
            }
            feature_vector[index++] = count;
        }
    }
}

//
void compute_APED_feature_vector(int width, int height, int F_H[width][height], int F_V[width][height], int F_P[width][height], int F_M[width][height], int final_vector[64]){
    int H_vector[16], V_vector[16], P_vector[16], M_vector[16];

    //
    count_edge_flags(width, height, F_H, H_vector);
    count_edge_flags(width, height, F_V, V_vector);
    count_edge_flags(width, height, F_P, P_vector);
    count_edge_flags(width, height, F_M, M_vector);

    //
    for (int i = 0; i < 16; i++){
        final_vector[i] = H_vector[i];
        final_vector[i + 16] = P_vector[i];
        final_vector[i + 32] = V_vector[i];
        final_vector[i + 48] = M_vector[i];
    }
}

#define NUM_FILES 8
#define VECTOR_SIZE 64

// Hàm tính khoảng cách Manhattan giữa 2 vector
int manhattan_distance(int vector1[VECTOR_SIZE], int vector2[VECTOR_SIZE]) {
    int distance = 0;
    for (int i = 0; i < VECTOR_SIZE; i++) {
        distance += abs(vector1[i] - vector2[i]);
    }
    return distance;
}

// Đọc ảnh từ file và tính vector đặc trưng
void process_file(const char *filename, int feature_vector[VECTOR_SIZE]) {
    int image[WIDTH][HEIGHT];
    read_image_from_txt(filename, image);
    
    int I_H[WIDTH][HEIGHT], I_V[WIDTH][HEIGHT], I_P[WIDTH][HEIGHT], I_M[WIDTH][HEIGHT];
    int flag_H[WIDTH][HEIGHT], flag_V[WIDTH][HEIGHT], flag_P[WIDTH][HEIGHT], flag_M[WIDTH][HEIGHT];
    int threshold[WIDTH][HEIGHT], F_H[WIDTH][HEIGHT], F_V[WIDTH][HEIGHT], F_P[WIDTH][HEIGHT], F_M[WIDTH][HEIGHT];

    // Áp dụng bộ lọc không gian
    apply_spatial_filter(WIDTH, HEIGHT, image, K_H, I_H);
    apply_spatial_filter(WIDTH, HEIGHT, image, K_V, I_V);
    apply_spatial_filter(WIDTH, HEIGHT, image, K_P, I_P);
    apply_spatial_filter(WIDTH, HEIGHT, image, K_M, I_M);

    // Tính toán cờ biên sơ bộ
    compute_preliminary_edge_flags(WIDTH, HEIGHT, I_H, I_V, I_P, I_M, flag_H, flag_V, flag_P, flag_M);
    
    // Tính toán ngưỡng
    compute_threshold(WIDTH, HEIGHT, image, threshold);
    
    // Tính toán bản đồ biên
    compute_directional_edge_map(WIDTH, HEIGHT, flag_H, flag_V, flag_P, flag_M, I_H, I_V, I_P, I_M, F_H, F_V, F_P, F_M, threshold);
    
    // Tính toán vector đặc trưng
    compute_APED_feature_vector(WIDTH, HEIGHT, F_H, F_V, F_P, F_M, feature_vector);
}

int main() {
    const char *input_filenames[NUM_FILES] = {
        "image_source_gs/64x64-Angelina Jolie.txt", 
        "image_source_gs/64x64-Cristiano Ronaldo.txt",
        "image_source_gs/64x64-Jerry Seinfeld.txt",
        "image_source_gs/64x64-Kendall Jenner.txt", 
        "image_source_gs/64x64-Leonardo DiCaprio.txt",
        "image_source_gs/64x64-Lionel Messi.txt", 
        "image_source_gs/64x64-Michael Jeffrey Jordan.txt",
        "image_source_gs/64x64-Robin Williams.txt", 
    };

    // Mảng lưu trữ vector đặc trưng của các file đầu vào
    int feature_vectors[NUM_FILES][VECTOR_SIZE];
    
    // Đọc và tính toán vector đặc trưng cho từng file đầu vào
    for (int i = 0; i < NUM_FILES; i++) {
        process_file(input_filenames[i], feature_vectors[i]);
        printf("Processed feature vector for file: %s\n", input_filenames[i]);
    }

    // Đọc và tính toán vector đặc trưng của ảnh cần kiểm tra
    int test_vector[VECTOR_SIZE];
    const char *test_filename = "image_test_gs/64x64-Jerry Seinfeld.txt";
    process_file(test_filename, test_vector);
    printf("Processed feature vector for test file: %s\n", test_filename);
    printf("\n");
    // So sánh vector đặc trưng của ảnh cần kiểm tra với các vector đặc trưng đầu vào
    int min_distance = 2147483647; // Khoảng cách lớn nhất có thể (INT_MAX)
    int best_match_index = -1;
    for (int i = 0; i < NUM_FILES; i++) {
        int distance = manhattan_distance(test_vector, feature_vectors[i]);
        printf("Distance to %s: %d\n", input_filenames[i], distance);
        if (distance < min_distance) {
            min_distance = distance;
            best_match_index = i;
        }
    }
    printf("\n");
    printf("The test image matches to %s\n", input_filenames[best_match_index]);

    return 0;
}
