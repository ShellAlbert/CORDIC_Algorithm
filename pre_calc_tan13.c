#include <stdio.h>
#include <math.h>
// #define M_PI 3.1415926
int main(void)
{
    int i;
    float lookup_table[13];
    float x,y;
    float left_angle;
    float dest_angle_degree=45.0;

    //1.generate lookup table.
    for(i=1;i<=13;i++)
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

    printf("time\tradian\tdegree\n");
    for(i=0;i<13;i++)
    {
        double radian=lookup_table[i];
        double degree=radian*180.0/M_PI;
        printf("%d\t%.2f\t%.2f\n",i+1,radian,degree);
    }

    //initial value.
    x=1.0f; y=0.0f;

    //convert degree to radian.
    left_angle=dest_angle_degree*M_PI/180.0;
    printf("degree %.2f -> radian %.2f\n",dest_angle_degree, left_angle);

    //rotate 13 times.
    for(i=0;i<13;i++)
    {
        //if left angle >=0, then clockwise rotation.
        //if left angle <0, then anti-clockwise rotation.
        float direction=(left_angle>=0)?(1.0f):(-1.0f);

        //pre-processing.
        double factor=pow(2.0,-i);
        //x_now=x_prev - d*(y_now*pow(2,-(i-1)));
        //y_now=y_prev + d*(x_now*pow(2,-(i-1)));
        double x_new=x - direction * y * factor;
        double y_new=y + direction * x * factor;
        double left_angle_new=left_angle - direction * lookup_table[i];
        printf("left angle : %.2f -> %.2f\n",left_angle,left_angle_new);
        //update.
        x=x_new;
        y=y_new;
        left_angle=left_angle_new;
    }

    //compensation.
    x=x*0.60725;
    y=y*0.60725;

    float radian_test=dest_angle_degree*M_PI/180.0;
    printf("cos(45)=%.2f(%.2f),sin(45)=%.2f(%.2f)\n",x,cos(radian_test),y,sin(radian_test));

    return 0;
}
