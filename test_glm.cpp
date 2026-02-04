#include <iostream>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main() {
    // 创建一个平移矩阵
    glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(100.0f, 200.0f, 0.0f));
    
    std::cout << "GLM Translation matrix (100, 200, 0):" << std::endl;
    std::cout << "GLM uses column-major storage: matrix[col][row]" << std::endl;
    std::cout << std::endl;
    
    // 打印矩阵内容
    std::cout << "Column 0: [" << trans[0][0] << ", " << trans[0][1] << ", " << trans[0][2] << ", " << trans[0][3] << "]" << std::endl;
    std::cout << "Column 1: [" << trans[1][0] << ", " << trans[1][1] << ", " << trans[1][2] << ", " << trans[1][3] << "]" << std::endl;
    std::cout << "Column 2: [" << trans[2][0] << ", " << trans[2][1] << ", " << trans[2][2] << ", " << trans[2][3] << "]" << std::endl;
    std::cout << "Column 3: [" << trans[3][0] << ", " << trans[3][1] << ", " << trans[3][2] << ", " << trans[3][3] << "]" << std::endl;
    
    std::cout << std::endl;
    std::cout << "As row-major (logical view):" << std::endl;
    for (int row = 0; row < 4; ++row) {
        std::cout << "Row " << row << ": [";
        for (int col = 0; col < 4; ++col) {
            std::cout << trans[col][row];
            if (col < 3) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
    
    // 测试变换一个点
    glm::vec4 point(10.0f, 20.0f, 0.0f, 1.0f);
    glm::vec4 result = trans * point;
    std::cout << std::endl;
    std::cout << "Transform point (10, 20, 0, 1):" << std::endl;
    std::cout << "Result: (" << result.x << ", " << result.y << ", " << result.z << ", " << result.w << ")" << std::endl;
    std::cout << "Expected: (110, 220, 0, 1)" << std::endl;
    
    return 0;
}
