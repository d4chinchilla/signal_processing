#include <iostream>
#include <cmath>
#include <vector>
#include <iomanip> 

using namespace std;

vector<double> c_dft_re(const vector<double> &dec_in); //compute DTF real part

vector<double> c_dft_im(const vector<double> &dec_in); //compute DTF imaginary part

vector<double> i_dft(const vector<double> &re_freq, const vector<double> &im_freq); //compute inverse DTF