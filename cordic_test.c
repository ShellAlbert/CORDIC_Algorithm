//gcc cordic_test.c -lm

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
// #define M_PI 3.1415926

//two approaches to generate this table.
//1. call math functions arctan() to generate dynamically.
//2. prepare the table by manual in FPGA project.
//  time    radian  degree
//  1       0.79    45.00
//  2       0.46    26.57
//  3       0.24    14.04
//  4       0.12    7.13
//  5       0.06    3.58
//  6       0.03    1.79
//  7       0.02    0.90
//  8       0.01    0.45
//  9       0.00    0.22
//  10      0.00    0.11
//  11      0.00    0.06
//  12      0.00    0.03
//  13      0.00    0.01

float lookup_table[13];
void cordic_prepare_table()
{
    //1.generate lookup table.
    for(int i=1;i<=13;i++)
    {
        //tan(theta_i)=2^(-(i-1)), i range: 1~13.
        //i=1, tan(theta_1)=2^(-0)=1, arctan(1)=45 degree.
        //i=2, tan(theta_2)=2^(-1)=0.5, arctan(0.5)=
        double value=pow(2,-(i-1));
        double radian=atan(value);
        //convert radian to degree.
        double degree=radian*180.0/M_PI;
        //lookup_table[i-1]=degree; //store in degree.
        lookup_table[i-1]=radian; //store in radian.

        printf("2^(-(%d-1))=%.5f(value),arctan(%.5f)=%.5f(radian), tan(deg(%.5f))=%.5f\n",i,value,value,radian,degree,value);
    }
    printf("prepared lookup table\n");
    printf("time\tradian\tdegree\n");
    for(int i=0;i<13;i++)
    {
        double radian=lookup_table[i];
        double degree=radian*180.0/M_PI;
        printf("%d\t%.2f\t%.2f\n",i+1,radian,degree);
    }
}

//iterate 13 times to approach the final value.
void cordic_calc(float angle_in_degree, float *cos_val, float *sin_val)
{
    //normalize angle to [0,360].
    float angle_normalize=fmod(angle_in_degree,360.0f);
    if(angle_normalize<0) {
        angle_normalize+=360.0f;
    }
    //define quadrant.
    int quadrant=0; //0:Q1, 1:Q2, 2:Q3, 3:Q4
    float dest_angle_degree=angle_normalize;

    if(angle_normalize>=0 && angle_normalize<90)
    {
        quadrant=0;
        dest_angle_degree=angle_normalize;
    }else if(angle_normalize>=90 && angle_normalize<180)
    {
        quadrant=1;
        dest_angle_degree=180.0f-angle_normalize; //Map to Q1 reference.
    }else if(angle_normalize>=180 && angle_normalize<270)
    {
        quadrant=2;
        dest_angle_degree=angle_normalize-180.0f; //Map to Q1 reference.
    }else{
        quadrant=3;
        dest_angle_degree=360.0f-angle_normalize; //Map to Q1 reference.
    }

    //convert degree to radian.
    float angle_radian=dest_angle_degree * M_PI / 180.0;
    //printf("Angle(degree %.2f -> radian %.2f)\n", dest_angle_degree, angle_radian);

    //initial value.
    float x=1.0f,y=0.0f;
    float angle_left=angle_radian;
    for(int i=0;i<13;i++)
    {
        //if angle lefted >=0, then rotate clockwise.
        //if angle lefted <0, then rotate anti-clockwise.
        float direction=(angle_left>=0)?(1.0f):(-1.0f);

        //pre-processing.
        //float factor=powf(2.0f,-i);

        // Replace powf(2.0f, -i) with bit shift logic
        // 2^i calculated via left shift
        int power_of_2 = 1 << i;

        // factor = 1.0 / 2^i
        float factor = 1.0f / (float)power_of_2;
    
        //calculation.
        float x_new=x-direction*y*factor;
        float y_new=y+direction*x*factor;
        float angle_left_new=angle_left-direction*lookup_table[i];

        //update for next approaching.
        x=x_new;
        y=y_new;
        angle_left=angle_left_new;
    }

    //after calculation, do K compensation.
    x=x*0.60725;
    y=y*0.60725;

    //post-processing, adjust signs based on original quadrant.
    //in Q1 (0-90): cos(+), sin(+)
    //in Q2 (90-180): cos(-), sin(+) -> Reference angle was (180-angle),so cos(ref) is positive, needs flip.
    //in Q3 (180-270): cos(-), sin(-)
    //in Q4 (270-360): cos(+), sin(-)
    float final_cos, final_sin;
    switch(quadrant)
    {
        case 0: //in Q1, 0 ~ 90.
            final_cos=x; final_sin=y; break;
        case 1: //in Q2, 90 ~ 180.
            final_cos=-x; final_sin=y; break;
        case 2: //in Q3, 180 ~ 270.
            final_cos=-x; final_sin=-y; break;
        case 3: //in Q4, 270 ~ 360.
            final_cos=x; final_sin=-y; break;
        default:
            final_cos=x; final_sin=y; break;
    }

    *cos_val=final_cos;
    *sin_val=final_sin;
    return;
}

int main(void)
{
    int fd_cos,fd_sin;
    char buffer[128];
    int line_no=0;
    float pre_calculated_degree[]={0,15,30,45,60,75,90,105,120,135,150,165,180,195,210};
    int table_size=sizeof(pre_calculated_degree)/sizeof(pre_calculated_degree[0]);

    //prepare table.
    cordic_prepare_table();

    for(int i=0;i<table_size; i++)
    {
        float angle_in_radian=pre_calculated_degree[i]*M_PI/180.0;

        float cos_val, sin_val;
        cordic_calc(pre_calculated_degree[i], &cos_val, &sin_val);
        printf("math: cos(degree(%.2f))=%.2f,sin(degree(%.2f))=%.2f cordic: cos(degree(%.2f))=%.2f,sin(degree(%.2f))=%.2f\n",
               pre_calculated_degree[i],cos_val,pre_calculated_degree[i],sin_val, \
               pre_calculated_degree[i],cos(angle_in_radian),pre_calculated_degree[i],sin(angle_in_radian));
    }

    fd_cos=open("cos0_360.dat",O_WRONLY|O_CREAT|O_TRUNC,0644);
    fd_sin=open("sin0_360.dat",O_WRONLY|O_CREAT|O_TRUNC,0644);
    if(fd_cos<0 || fd_sin<0)
    {
        printf("failed to open file,quit.\n");
        return -1;
    }
    printf("comparison between math and cordic from 0 degree to 360 degree\n");
    printf("degree\tradian\tmath_cos(),\tcordic_cos()\tmath_sin()\tcordic_sin()\n");
    for(float i=0;i<=360*1;i+=1)
    {
        //convert degree to radian for math functions.
        float angle_in_degree=i;
        float angle_in_radian=i*M_PI/180.0;

        //call math functions.
        float math_cos=cos(angle_in_radian);
        float math_sin=sin(angle_in_radian);

        //call cordic algorithm.
        float cordic_cos, cordic_sin;
        cordic_calc(angle_in_degree, &cordic_cos, &cordic_sin);

        printf("%.2f\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n",
               angle_in_degree,angle_in_radian, \
               math_cos, cordic_cos, \
               math_sin, cordic_sin \
               );
        
        //here, cos and sin value are already in the range of [-1,1].
        //we add a DC offset to eliminate negative value, its range is [0,2].
        float dc_offset=1.0f;

        //now the range is [0,2], we need to scale it to [0,65535].
        //65535/2=32767.5
        //but in a 16-bits ADC, the range is [0,65535].
        //so we need to scale the value to the range of [0,65535].
        float cos_scaled=(cordic_cos+dc_offset)*32767.5f;
        float sin_scaled=(cordic_sin+dc_offset)*32767.5f;

        //write cos value to file for plotting.
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d\t%.2f\n",line_no,/*cordic_cos*/cos_scaled);
        write(fd_cos,buffer,strlen(buffer));

        //write sin value to file for plotting.
        memset(buffer,0,sizeof(buffer));
        sprintf(buffer,"%d\t%.2f\n",line_no,/*cordic_sin*/sin_scaled);
        write(fd_sin,buffer,strlen(buffer));
        line_no++;
    }
    close(fd_cos);
    close(fd_sin);
    return 0;
}
