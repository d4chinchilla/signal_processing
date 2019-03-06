#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

int select_mic(); //select which microphone data to compute

double cal_amplitude(const vector<double> &data); //calculate the power of signal in dB