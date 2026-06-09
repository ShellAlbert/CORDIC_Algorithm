
#include <stdio.h>
#include <math.h>

// CORDIC迭代次数，通常16-32次足以达到高精度
#define CORDIC_ITERATIONS 30

// 预计算的arctan(2^-i)表，单位为弧度
// 这些值对应于每次微旋转的角度
static const double cordic_angles[] = {
    0.785398163397448,  // atan(1)     = 45 deg
    0.463647609000806,  // atan(1/2)   = 26.565 deg
    0.244978663126864,  // atan(1/4)   = 14.036 deg
    0.124354994546761,  // atan(1/8)   = 7.125 deg
    0.062418809995957,  // atan(1/16)  = 3.576 deg
    0.031239833430268,  // atan(1/32)  = 1.789 deg
    0.015623728620477,  // atan(1/64)  = 0.895 deg
    0.007812341060101,  // atan(1/128) = 0.448 deg
    0.003906230131967,  // atan(1/256) = 0.224 deg
    0.001953122516479,  // atan(1/512) = 0.112 deg
    0.000976562189559,  // atan(1/1024)= 0.056 deg
    0.000488281211195,  // atan(1/2048)= 0.028 deg
    0.000244140620149,  // atan(1/4096)= 0.014 deg
    0.000122070311894,  // atan(1/8192)= 0.007 deg
    0.000061035156174,  // atan(1/16384)= 0.0035 deg
    0.000030517578116,  // atan(1/32768)= 0.00175 deg
    0.000015258789061,  // ...
    0.000007629394531,
    0.000003814697266,
    0.000001907348633,
    0.000000953674316,
    0.000000476837158,
    0.000000238418579,
    0.000000119209290,
    0.000000059604645,
    0.000000029802322,
    0.000000014901161,
    0.000000007450581,
    0.000000003725290,
    0.000000001862645
};

// CORDIC增益常数 K = product(cos(arctan(2^-i))) for i=0 to infinity
// 对于30次迭代，K ≈ 0.607252935
#define CORDIC_GAIN 0.607252935008881

typedef struct {
    double x;
    double y;
    double z; // 当前剩余角度
} CordicState;

/**
 * CORDIC算法核心函数 (Rotation Mode)
 * 计算给定角度z的sin(z)和cos(z)
 * 初始向量设为 (K, 0)，这样最终结果不需要再乘以K进行归一化
 * 或者初始向量设为 (1, 0)，最后结果乘以K
 * 这里采用初始向量 (1, 0)，最后输出时处理增益
 */
void cordic_compute(double angle_rad, double *sin_val, double *cos_val) {
    double x = 1.0; // 初始X坐标
    double y = 0.0; // 初始Y坐标
    double z = angle_rad; // 目标角度

    int i;
    for (i = 0; i < CORDIC_ITERATIONS; i++) {
        double sigma;
        // 确定旋转方向：如果剩余角度z >= 0，则逆时针旋转(sigma=1)，否则顺时针(sigma=-1)
        if (z >= 0) {
            sigma = 1.0;
        } else {
            sigma = -1.0;
        }

        // 计算移位后的值: x * 2^-i 和 y * 2^-i
        // 使用 ldexp 或者直接除法，在硬件中这是移位操作
        double x_shifted = x / (1 << i); // 注意：对于大i，1<<i可能溢出int，但在double除法中通常没问题，或者使用 pow(2, i)
        // 更安全的移位模拟：
        double factor = pow(2.0, -i);
        double x_new = x - sigma * y * factor;
        double y_new = y + sigma * x * factor;
        double z_new = z - sigma * cordic_angles[i];

        x = x_new;
        y = y_new;
        z = z_new;
    }

    // 补偿增益 K
    *cos_val = x * CORDIC_GAIN;
    *sin_val = y * CORDIC_GAIN;
}

/**
 * 使用CORDIC计算反正切 (Vectoring Mode)
 * 输入: (x, y)
 * 输出: 角度 theta = atan2(y, x)
 * 目标是将y旋转至0，累计旋转的角度即为theta
 */
double cordic_atan2(double x, double y) {
    double z = 0.0; // 累计角度
    int i;

    // 简单的象限处理可以在此添加，这里假设主要处理第一四象限或通用情况
    // 为了简化演示，我们假设输入已经在合理范围，或者通过初始判断调整
    // 完整的atan2需要处理象限，这里演示核心迭代

    for (i = 0; i < CORDIC_ITERATIONS; i++) {
        double sigma;
        if (y < 0) {
            sigma = -1.0; // 顺时针旋转，使y趋向0
        } else {
            sigma = 1.0;  // 逆时针旋转
        }

        double factor = pow(2.0, -i);
        double x_new = x + sigma * y * factor;
        double y_new = y - sigma * x * factor;
        double z_new = z + sigma * cordic_angles[i];

        x = x_new;
        y = y_new;
        z = z_new;
    }

    return z;
}

int main() {
    printf("=== CORDIC Algorithm Demonstration in C ===\n\n");

    // 测试1: 计算 Sin 和 Cos
    double test_angles_deg[] = {0, 30, 45, 60, 90, 180, -45};
    int num_tests = sizeof(test_angles_deg) / sizeof(test_angles_deg[0]);

    printf("--- Testing Sin/Cos Calculation ---\n");
    printf("%-10s | %-15s | %-15s | %-15s | %-15s\n",
           "Angle(deg)", "Cordic Sin", "Math Sin", "Cordic Cos", "Math Cos");
    printf("-----------------------------------------------------------------------\n");

    for (int k = 0; k < num_tests; k++) {
        double deg = test_angles_deg[k];
        double rad = deg * M_PI / 180.0;

        double c_sin, c_cos;
        cordic_compute(rad, &c_sin, &c_cos);

        double m_sin = sin(rad);
        double m_cos = cos(rad);

        printf("%-10.2f | %-15.10f | %-15.10f | %-15.10f | %-15.10f\n",
               deg, c_sin, m_sin, c_cos, m_cos);
    }

    printf("\n--- Testing Atan2 Calculation ---\n");
    printf("%-10s | %-10s | %-15s | %-15s | %-10s\n",
           "X", "Y", "Cordic Ang(deg)", "Math Ang(deg)", "Error");
    printf("-----------------------------------------------------------------------\n");

    double test_x[] = {1, 0, -1, 0, 1, 1};
    double test_y[] = {0, 1, 0, -1, 1, -1};
    int num_atan_tests = sizeof(test_x) / sizeof(test_x);

    for (int k = 0; k < num_atan_tests; k++) {
        double x = test_x[k];
        double y = test_y[k];

        double c_ang_rad = cordic_atan2(x, y);
        double c_ang_deg = c_ang_rad * 180.0 / M_PI;

        double m_ang_rad = atan2(y, x);
        double m_ang_deg = m_ang_rad * 180.0 / M_PI;

        double error = fabs(c_ang_deg - m_ang_deg);

        printf("%-10.2f | %-10.2f | %-15.10f | %-15.10f | %-10.10f\n",
               x, y, c_ang_deg, m_ang_deg, error);
    }

    return 0;
}
