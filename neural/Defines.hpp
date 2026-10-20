#ifndef DEFINES_HPP
#define DEFINES_HPP

#define DEBUG 1
#define MATRIX_SIZE 30

enum Parameters{
    kParamDelay=MATRIX_SIZE*(MATRIX_SIZE+4),
    kParamMaxEigenvalue,
    kParamNoteSlide,
    kParamCount
};

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
}else if(index==kParamDelay){\
    fDelay=value;\
}else if(index==kParamMaxEigenvalue){\
    fMaxEigenvalue=value;\
}else if(index==kParamNoteSlide){\
fNoteSlide=value;\
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

#define MAX_DELAY 2000



#endif // DEFINES_HPP
