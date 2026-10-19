#ifndef DEFINES_HPP
#define DEFINES_HPP

#define DEBUG 1

#define SET_PARAMETER_VALUE \
if(index<MATRIX_SIZE*2){\
    int row=index/2;\
    int col=index%2;\
inputMatrix(row,col)=value;\
}else if(index<MATRIX_SIZE*(MATRIX_SIZE+2)){\
    int newIndex=index-MATRIX_SIZE*2;\
    int row=newIndex/MATRIX_SIZE;\
    int col=newIndex%MATRIX_SIZE;\
    weightMatrix(row,col)=value;\
}else if(index<MATRIX_SIZE*(MATRIX_SIZE+4)){\
    int newIndex=index-MATRIX_SIZE*(MATRIX_SIZE+2);\
    int row=newIndex/2;\
    int col=newIndex%2;\
outputMatrix(row,col)=value;\
}else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)){\
    fDelay=value;\
}else if(index==MATRIX_SIZE*(MATRIX_SIZE+4)+1){\
    fMaxEigenvalue=value;\
}

enum ActivationFunctionType{
    activationFunctionClip,
    activationFunctionTanh,
    activationFunctionNone,
    activationFunctionCount
};
static const char* activationFunctionNames[]={
    "Clip",
    "Tanh",
    "None"
};

#define MATRIX_SIZE 30
#define PARAM_COUNT MATRIX_SIZE*(MATRIX_SIZE+4)+2//delay and max eigenvalue
#define MAX_DELAY 1000

enum Parameters{
    kParamDelay=MATRIX_SIZE*(MATRIX_SIZE+4),
    kParamMaxEigenvalue,
    kParamCount
};

#endif // DEFINES_HPP
