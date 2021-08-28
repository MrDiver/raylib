#include "raylib.h"
#include <stdio.h>

const int PRECISION = 3;
const int MAX_PRINT_SIZE = 20;
enum MathPrintMode PRINT_MODE = MINI;
const char* getMathFormatString(){
    switch(PRINT_MODE){
        case SCIENTIFIC:
            return "%-+#*.*e";
        case MINI:
            return "%-*.*G";
        default:
        case SHORT:
            return "%-+*.*f";
    }
}

int getMaxPrintedSize(float *m, int size){
    // Create variables and buffers
    int max = 0;
    int tmp = 0;
    char buffer[MAX_PRINT_SIZE+1];

    // get format string
    char fmt_string[16];
    snprintf(fmt_string,16,"%s%%n",getMathFormatString());
//     printf("FORMAT: %s\n",fmt_string);
    for(int i = 0; i < size; i++){
        snprintf(buffer,MAX_PRINT_SIZE+1, fmt_string, 0, PRECISION,m[i],&tmp);
        if(tmp>max)max=tmp;
//         printf("TEST = %s\n", buffer);
    }
    return max;
}

void printMatrixPro(float *m, int sx, int sy){
    FILE* out = stdout;
    // Create Buffer for format characters to avoid overflows
    char buffer[MAX_PRINT_SIZE+1];
    // Get format string for mode
    const char * fmt_string = getMathFormatString();
    const int PADDING = getMaxPrintedSize(m,sx*sy);

    fprintf(out, "[");
    for(int y = 0; y < sy; y++){
        fprintf(out, "%*s[ ",y!=0,"");
        for(int x = 0; x < sx; x++){
            snprintf(buffer, MAX_PRINT_SIZE+1, fmt_string, PADDING+1, PRECISION,m[y*sx+x]);
            fprintf(out,"%s", buffer);
        }
        fprintf(out, "]%s",y!=(sy-1)?"\n":"");
    }
    fprintf(out, "]\n\n");
}

void mathPrintM(Matrix m){
    printMatrixPro(&m.m0,4,4);
}

void mathPrintV4(Vector4 v){
    printMatrixPro(&v.x,4,1);
}

void mathPrintV3(Vector3 v){
    printMatrixPro(&v.x,3,1);
}

void mathPrintV2(Vector2 v){
    printMatrixPro(&v.x,2,1);
}
