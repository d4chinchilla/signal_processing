#define DFT_MAX_FREQ    2000.0
#define DFT_OUT_LEN     40
#define DFT_SAMPLE_RATE SAMPLE_RATE

#ifdef __cplusplus
#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip>
#include "../sample.h"
using namespace std;
vector<double> c_dft_re(const vector<double> &dec_in); //compute DTF real part

vector<double> c_dft_im(const vector<double> &dec_in); //compute DTF imaginary part

vector<double> i_dft(const vector<double> &re_freq, const vector<double> &im_freq); //compute invert DTF
#endif
