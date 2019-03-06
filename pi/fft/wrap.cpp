#include "DFT.h"

extern "C" {
void dft_wrap(int *in, double *out_re, double *out_im)
{
    int ind;
    vector<double> invec;

    for (ind = 0; ind < SAMPLE_SIZE; ++ind)
        invec.push_back(in[ind]);

    vector<double> reals = c_dft_re(invec);
    vector<double> imags = c_dft_im(invec);

    for (ind = 0; ind < DFT_OUT_LEN; ++ind)
    {
        out_re[ind] = reals[ind];
        out_im[ind] = imags[ind];
    }
}

}
