/***************************************************************************/
/* (C) 2016 Elettra - Sincrotrone Trieste S.C.p.A.. All rights reserved.   */
/*                                                                         */
/*                                                                         */
/* This file is part of Pore3D, a software library for quantitative        */
/* analysis of 3D (volume) images.                                         */
/*                                                                         */
/* Pore3D is free software: you can redistribute it and/or modify it       */
/* under the terms of the GNU General Public License as published by the   */
/* Free Software Foundation, either version 3 of the License, or (at your  */
/* option) any later version.                                              */
/*                                                                         */
/* Pore3D is distributed in the hope that it will be useful, but WITHOUT   */
/* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or   */
/* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License    */
/* for more details.                                                       */
/*                                                                         */
/* You should have received a copy of the GNU General Public License       */
/* along with Pore3D. If not, see <http://www.gnu.org/licenses/>.          */
/*                                                                         */
/***************************************************************************/

//
// Author: Francesco Brun
// Last modified: Sept, 28th 2016
//

// A square image with dimensions POLARX x POLARX is returned. Due to the fact
// the POLARX is not known a priori, memory image is allocated within this
// procedure so parameter OUT_IM should be passed as reference. The out parameter
// POLARX should be used for further handling of OUT_IM.

//int _isPath(char* ); amal

void p3dCartesian2polar_8(
        unsigned char* in_im, // IN: input image in cartesian coordinates
        unsigned char** out_im, // OUT: output image in polar coordinates
        const int dimx, // IN: width of input image
        const int dimy, // IN: heigth of input image
        const double centerX, // IN: X coordinate for center of polar transform
        const double centerY, // IN: Y coordinate for center of polar transform
        const double precision,
        int* polarX // OUT: side of the square image returned as output
        );

void p3dCartesian2polar_16(
        unsigned short* in_im, // IN: input image in cartesian coordinates
        unsigned short** out_im, // OUT: output image in polar coordinates
        const int dimx, // IN: width of input image
        const int dimy, // IN: heigth of input image
        const double centerX, // IN: X coordinate for center of polar transform
        const double centerY, // IN: Y coordinate for center of polar transform
        const double precision,
        int* polarX // OUT: side of the square image returned as output
        );


// For coherence with dual procedure CARTESIAN2POLAR memory image is allocated
// within this procedure so parameter OUT_IM should be passed as reference.

void p3dPolar2cartesian_8(
        unsigned char* in_im, // IN: input image in polar coordinates
        unsigned char** out_im, // OUT: output image in cartesian coordinates
        const int polarX, // IN: side of the square of polar image
        const double centerX, // IN: X coordinate for center of polar transform
        const double centerY, // IN: Y coordinate for center of polar transform
        const int original_dimx, // IN: width of the output cartesian image
        const int original_dimy // IN: heigth of the output cartesian image
        );

void p3dPolar2cartesian_16(
        unsigned short* in_im, // IN: input image in polar coordinates
        unsigned short** out_im, // OUT: output image in cartesian coordinates
        const int polarX, // IN: side of the square of polar image
        const double centerX, // IN: X coordinate for center of polar transform
        const double centerY, // IN: Y coordinate for center of polar transform
        const int original_dimx, // IN: width of the output cartesian image
        const int original_dimy // IN: heigth of the output cartesian image
        );