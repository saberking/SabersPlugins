#ifndef DEFINES_HPP
#define DEFINES_HPP

#define DEBUG 1
#define MAX_MATRIX_SIZE 30

enum Parameters{
    kParamDelay=MAX_MATRIX_SIZE*(MAX_MATRIX_SIZE+4),
    kParamMaxEigenvalue,
    kParamNoteSlide,
    kParamMatrixSize,
    kParamCount
};

#define SET_PARAMETER_VALUE \
if(index<MAX_MATRIX_SIZE*2){\
    int row=index/2;\
    int col=index%2;\
inputMatrix(row,col)=value;\
}else if(index<MAX_MATRIX_SIZE*(MAX_MATRIX_SIZE+2)){\
    int newIndex=index-MAX_MATRIX_SIZE*2;\
    int row=newIndex/MAX_MATRIX_SIZE;\
    int col=newIndex%MAX_MATRIX_SIZE;\
    weightMatrix(row,col)=value;\
}else if(index<MAX_MATRIX_SIZE*(MAX_MATRIX_SIZE+4)){\
    int newIndex=index-MAX_MATRIX_SIZE*(MAX_MATRIX_SIZE+2);\
    int row=newIndex/2;\
    int col=newIndex%2;\
outputMatrix(row,col)=value;\
}else if(index==kParamDelay){\
    fDelay=value;\
}else if(index==kParamMaxEigenvalue){\
    fMaxEigenvalue=value;\
}else if(index==kParamNoteSlide){\
fNoteSlide=value;\
}else if(index==kParamMatrixSize){\
fMatrixSize=(int)value;\
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
