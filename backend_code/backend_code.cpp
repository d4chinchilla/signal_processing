
#include "DFT.h"
#include "x_corr.h"

using namespace std;

int sample;

vector<double> dec_str_1 = {34,66,91,44,67,88,45,11,90,66,82,13,56,22,55,92,34,76,22,11,65,87,34,90,18,9,51,49,75,80,31,41};
vector<double> dec_str_2 = {20,99,16,84,23,58,33,49,65,71,91,34,66,91,44,67,88,45,11,90,66,82,13,56,22,55,92,34,76,32,77,12};

//vector<double> dec_str_1 = { 334,668,912,447,679};
//vector<double> dec_str_2 = { 334,668,912,447,679 };

vector<double> dft_str_1_re;
vector<double> dft_str_1_im;
vector<double> dft_str_2_re;
vector<double> dft_str_2_im;

vector<double> idft_str_1;
vector<double> idft_str_2;

vector<double> x_corr_f;
vector<double> x_corr_s;

int delay;

int main()
{

	x_corr_f = x_corr_dft(dec_str_1, dec_str_2);
	//number of element+1 shows delay, take absolute value, if -ve means 1st set of data lag 'n' elements of 2nd data, if +ve means 1st set of data lead 'n' elements of 2nd data
	//elements all 0 ==> delay = 0;

	delay = delay_dft_func(x_corr_f, dec_str_1, dec_str_2);

	x_corr_s = x_corr(dec_str_1, dec_str_2);

	dft_str_1_re = c_dft_re(dec_str_1);
	dft_str_1_im = c_dft_im(dec_str_1);

	dft_str_2_re = c_dft_re(dec_str_2);
	dft_str_2_im = c_dft_im(dec_str_2);

	if (dft_str_1_re.size() >= dft_str_1_im.size())
	{
		sample = dft_str_1_re.size();
	}
	else
	{
		sample = dft_str_1_im.size();
	}

	for (int i = 0; i < sample; i++)
	{
		if (dft_str_1_im[i] <= 0)
		{
			cout << "dft_str " << i << " is: " << dft_str_1_re[i] << dft_str_1_im[i] << "i" << endl;
		}
		else
		{
			cout << "dft_str " << i << " is: " << dft_str_1_re[i] << "+" << dft_str_1_im[i] << "i" << endl;
		}
	}

	cout << endl;

	idft_str_1 = i_dft(dft_str_1_re, dft_str_1_im);
	idft_str_2 = i_dft(dft_str_2_re, dft_str_2_im);

	
	system("pause");
	return 0;
}

