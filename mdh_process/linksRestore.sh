#!/bin/bash

topDir=$(pwd)

cd includelib
ln -s ../../../misc/cppmatrixt/src/math_misc.h .
ln -s ../../../misc/cppmatrixt/src/math_misc.cpp .

ln -s ../../../misc/cppmatrixt/src/cmatrix.h .
ln -s ../../../misc/cppmatrixt/src/cmatrix.cpp .

ln -s ../../../C_utils/C_SMessage/includelib/c_SMessage.h .
ln -s ../../../C_utils/C_SMessage/includelib/c_SMessage.cpp .

ln -s ../../../C_utils/C_scanopt/includelib/C_scanopt.h .
ln -s ../../../C_utils/C_scanopt/includelib/C_scanopt.cpp .

ln -s ../../../C_utils/C_SSocket/includelib/c_SSocket.h .
ln -s ../../../C_utils/C_SSocket/includelib/c_SSocket.cpp .

cd $topDir
