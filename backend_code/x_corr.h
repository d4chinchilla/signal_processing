#include <iostream> 
#include <cmath>
#include <vector>
#include <algorithm>

#include "DFT.h"

using namespace std;

vector<double> x_corr(const vector<double> &corr_a, const vector<double> &corr_b); //try to use convolution to do cross_correlation, but test failed and not in used

vector<double> x_corr_dft(const vector<double> &corr_a,const vector<double> &corr_b); //do cross_correlation by using DFT

//multiply the real part of 2 DFT vector
vector<double> dft_multiply_re(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im); 

//multiply the imaginary part of 2 DFT vector
vector<double> dft_multiply_im(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im);

//do the conjugate for the 2nd DFT_im vector
vector<double> conjugate(vector<double> &im_part);

//calculate delay between 2 signal
int delay_dft_func(vector<double> &find_delay, vector<double> &check_data_a, vector<double> &check_data_b);