#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "p3dFilt.h"
#include "p3dTime.h"

double _p3dHuangYagerThresholding_Ux(int g, int u0, int u1, int t) {
    double ux, x;

    if (g <= t) {
        x = 1.0 + ((double) abs(g - u0)) / 255.0;
        ux = 1.0 / x;
    } else {
        x = 1.0 + ((double) abs(g - u1)) / 255.0;
        ux = 1.0 / x;
    }
    //if (ux < 0.5 || ux > 1.0)
        //printf("Ux = %f\n", ux);
    return ux;
}

double _p3dHuangYagerThresholding_Ux16(int g, int u0, int u1, int t) {
    double ux, x;

    if (g <= t) {
        x = 1.0 + ((double) abs(g - u0)) / 65535.0;
        ux = 1.0 / x;
    } else {
        x = 1.0 + ((double) abs(g - u1)) / 65535.0;
        ux = 1.0 / x;
    }
    //if (ux < 0.5 || ux > 1.0)
        //printf("Ux = %f\n", ux);
    return ux;
}

double _p3dHuangYagerThresholding_yager(int u0, int u1, int t) {
    int i;
    double x, sum = 0.0;

    for (i = 0; i < 256; i++) {
        x = _p3dHuangYagerThresholding_Ux(i, u0, u1, t);
        x = x * (1.0 - x);
        sum += x*x;
    }
    x = (double) sqrt((double) sum);
    return x;
}

double _p3dHuangYagerThresholding_yager16(int u0, int u1, int t) {
    int i;
    double x, sum = 0.0;

    for (i = 0; i < 65536; i++) {
        x = _p3dHuangYagerThresholding_Ux16(i, u0, u1, t);
        x = x * (1.0 - x);
        sum += x*x;
    }
    x = (double) sqrt((double) sum);
    return x;
}

int p3dHuangYagerThresholding_8(
        unsigned char* in_im,
        unsigned char* out_im,
        const int dimx,
        const int dimy,
        const int dimz,
        unsigned char* thresh,
        int (*wr_log)(const char*, ...),
        int (*wr_progress)(const int, ...)
        ) 
{

    double *S, *Sbar, *W, *Wbar;
    double *hist, *F, maxv = 0.0, delta, sum, minsum;
    int i, t, tbest = -1, u0, u1;
    int start, end;

    int ct;


    // Start tracking computational time:
    if (wr_log != NULL) {
        p3dResetStartTime();
        wr_log("Pore3D - Thresholding image according to Huang's method...");
    }


    /* Allocate and initialize to zero kernel histogram: */
    /* Allocate and initialize to zero kernel histogram: */
    P3D_MEM_TRY(hist = (double*) calloc((UCHAR_MAX + 1), sizeof (double)));
    P3D_MEM_TRY(S = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
    P3D_MEM_TRY(Sbar = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
    P3D_MEM_TRY(W = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
    P3D_MEM_TRY(Wbar = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));
    P3D_MEM_TRY(F = (double *) malloc(sizeof (double) *(UCHAR_MAX + 1)));

    /* Compute image histogram: */
    for (ct = 0; ct < (dimx * dimy * dimz); ct++)
        hist[in_im[ ct ]] = hist[in_im[ ct ]] + 1.0;


    /* Find cumulative histogram */
    S[0] = hist[0];
    W[0] = 0;
    for (i = 1; i <= UCHAR_MAX; i++) {
        S[i] = S[i - 1] + hist[i];
        W[i] = i * hist[i] + W[i - 1];
    }

    /* Cumulative reverse histogram */
    Sbar[UCHAR_MAX] = 0;
    Wbar[UCHAR_MAX] = 0;
    for (i = (UCHAR_MAX - 1); i >= 0; i--) {
        Sbar[i] = Sbar[i + 1] + hist[i + 1];
        Wbar[i] = Wbar[i + 1] + (i + 1) * hist[i + 1];
    }

    for (t = 1; t < UCHAR_MAX; t++) {
        if (hist[t] == 0.0) continue;
        if (S[t] == 0.0) continue;
        if (Sbar[t] == 0.0) continue;

        /* Means */
        u0 = (int) (W[t] / S[t] + 0.5);
        u1 = (int) (Wbar[t] / Sbar[t] + 0.5);

        /* Fuzziness measure */
        F[t] = _p3dHuangYagerThresholding_yager(u0, u1, t) / (dimx * dimy * dimz);

        /* Keep the minimum fuzziness */
        if (F[t] > maxv)
            maxv = F[t];
        if (tbest < 0)
            tbest = t;
        else if (F[t] < F[tbest]) tbest = t;
    }

    /* Find best out of a range of thresholds */
    delta = F[tbest] + (maxv - F[tbest])*0.05; /* 5% */
    start = (int) (tbest - delta);
    if (start <= 0)
        start = 1;
    end = (int) (tbest + delta);
    if (end >= UCHAR_MAX)
        end = (UCHAR_MAX - 1);
    minsum = UINT_MAX;

    for (i = start; i <= end; i++) {
        sum = hist[i - 1] + hist[i] + hist[i + 1];
        if (sum < minsum) {
            t = i;
            minsum = sum;
        }
    }

    *thresh = (unsigned char) t;


#pragma omp parallel for
    for (ct = 0; ct < (dimx * dimy * dimz); ct++)
        out_im[ ct ] = (in_im[ ct ] > (*thresh)) ? OBJECT : BACKGROUND;

    // Free memory:
    free(hist);
    free(S);
    free(Sbar);
    free(W);
    free(Wbar);
    free(F);

    // Print elapsed time (if required):
    if (wr_log != NULL) {
        wr_log("\tDetermined threshold: %d.", *thresh);
        wr_log("Pore3D - Image thresholded successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
    }

    // Return success:
    return P3D_SUCCESS;


MEM_ERROR:

    if (wr_log != NULL) {
        wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
    }

    // Free memory:
    free(hist);
    free(S);
    free(Sbar);
    free(W);
    free(Wbar);
    free(F);

    // Return error:
    return P3D_ERROR;


}

int p3dHuangYagerThresholding_16(
        unsigned short* in_im,
        unsigned char* out_im,
        const int dimx,
        const int dimy,
        const int dimz,
        unsigned short* thresh,
        int (*wr_log)(const char*, ...),
        int (*wr_progress)(const int, ...)
        ) 
{

    double *S, *Sbar, *W, *Wbar;
    double *hist, *F, maxv = 0.0, delta, sum, minsum;
    int i, t, tbest = -1, u0, u1;
    int start, end;

    int ct;

    // Start tracking computational time:
    if (wr_log != NULL) {
        p3dResetStartTime();
        wr_log("Pore3D - Thresholding image according to Huang's method...");
    }


    /* Allocate and initialize to zero kernel histogram: */
    P3D_MEM_TRY(hist = (double*) calloc((USHRT_MAX + 1), sizeof (double)));
    P3D_MEM_TRY(S = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
    P3D_MEM_TRY(Sbar = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
    P3D_MEM_TRY(W = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
    P3D_MEM_TRY(Wbar = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));
    P3D_MEM_TRY(F = (double *) malloc(sizeof (double) *(USHRT_MAX + 1)));

    /* Compute image histogram: */
    for (ct = 0; ct < (dimx * dimy * dimz); ct++)
        hist[in_im[ ct ]] = hist[in_im[ ct ]] + 1.0;


    /* Find cumulative histogram */
    S[0] = hist[0];
    W[0] = 0;
    for (i = 1; i <= USHRT_MAX; i++) {
        S[i] = S[i - 1] + hist[i];
        W[i] = i * hist[i] + W[i - 1];
    }

    /* Cumulative reverse histogram */
    Sbar[USHRT_MAX] = 0;
    Wbar[USHRT_MAX] = 0;
    for (i = (USHRT_MAX - 1); i >= 0; i--) {
        Sbar[i] = Sbar[i + 1] + hist[i + 1];
        Wbar[i] = Wbar[i + 1] + (i + 1) * hist[i + 1];
    }

    for (t = 1; t < USHRT_MAX; t++) {
        if (hist[t] == 0.0) continue;
        if (S[t] == 0.0) continue;
        if (Sbar[t] == 0.0) continue;

        /* Means */
        u0 = (int) (W[t] / S[t] + 0.5);
        u1 = (int) (Wbar[t] / Sbar[t] + 0.5);

        /* Fuzziness measure */
        F[t] = _p3dHuangYagerThresholding_yager16(u0, u1, t) / (dimx * dimy * dimz);

        /* Keep the minimum fuzziness */
        if (F[t] > maxv)
            maxv = F[t];
        if (tbest < 0)
            tbest = t;
        else if (F[t] < F[tbest]) tbest = t;
    }

    /* Find best out of a range of thresholds */
    delta = F[tbest] + (maxv - F[tbest])*0.05; /* 5% */
    start = (int) (tbest - delta);
    if (start <= 0)
        start = 1;
    end = (int) (tbest + delta);
    if (end >= USHRT_MAX)
        end = (USHRT_MAX - 1);
    minsum = UINT_MAX;

    for (i = start; i <= end; i++) {
        sum = hist[i - 1] + hist[i] + hist[i + 1];
        if (sum < minsum) {
            t = i;
            minsum = sum;
        }
    }

    *thresh = (unsigned short) t;


#pragma omp parallel for
    for (ct = 0; ct < (dimx * dimy * dimz); ct++)
        out_im[ ct ] = (in_im[ ct ] > (*thresh)) ? OBJECT : BACKGROUND;

    // Free memory:
    free(hist);
    free(S);
    free(Sbar);
    free(W);
    free(Wbar);
    free(F);

    // Print elapsed time (if required):
    if (wr_log != NULL) {
        wr_log("\tDetermined threshold: %d.", *thresh);
        wr_log("Pore3D - Image thresholded successfully in %dm%0.3fs.", p3dGetElapsedTime_min(), p3dGetElapsedTime_sec());
    }

    // Return success:
    return P3D_SUCCESS;


MEM_ERROR:

    if (wr_log != NULL) {
        wr_log("Pore3D - Not enough (contiguous) memory. Program will exit.");
    }

    // Free memory:
    free(hist);
    free(S);
    free(Sbar);
    free(W);
    free(Wbar);
    free(F);

    // Return error:
    return (int) P3D_ERROR;

}
