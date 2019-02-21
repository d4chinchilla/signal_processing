#include <iostream> 
#include <cmath>
#include <vector>
#include <algorithm>

#include "DFT.h"

using namespace std;

vector<double> x_corr(const vector<double> &corr_a, const vector<double> &corr_b);

vector<double> x_corr_dft(const vector<double> &corr_a,const vector<double> &corr_b);

vector<double> dft_multiply_re(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im);

vector<double> dft_multiply_im(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im);

vector<double> conjugate(vector<double> &im_part);

int delay_dft_func(vector<double> &find_delay, vector<double> &check_data_a, vector<double> &check_data_b);