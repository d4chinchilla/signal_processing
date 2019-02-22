
#include "DFT.h"
#include "x_corr.h"

using namespace std;

int sample;

vector<double> dec_str_1 = { 47, 115, 87, 128, 38, 210, 35, 127, 63, 165, 61, 255, 245, 144, 23, 80, 50, 17, 143, 156, 198, 39, 107, 82, 223, 105, 94, 199, 84, 226 };
vector<double> dec_str_2 = { 115, 87, 128, 38, 210, 35, 127, 63, 165, 61, 255, 245, 144, 23, 80, 50, 17, 143, 156, 198, 39, 107, 82, 223, 105, 94, 199, 84, 226, 132};

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
			cout << setprecision(6) << "dft_str " << i << " is: " << dft_str_1_re[i] << dft_str_1_im[i] << "i" << endl;
		}
		else
		{
			cout << setprecision(6) << "dft_str " << i << " is: " << dft_str_1_re[i] << "+" << dft_str_1_im[i] << "i" << endl;
		}
	}

	cout << endl;

	idft_str_1 = i_dft(dft_str_1_re, dft_str_1_im);
	idft_str_2 = i_dft(dft_str_2_re, dft_str_2_im);

	x_corr_f = x_corr_dft(dec_str_1, dec_str_2);
	//number of element+1 shows delay, take absolute value, if -ve means 1st set of data lag 'n' elements of 2nd data, if +ve means 1st set of data lead 'n' elements of 2nd data
	//elements all 0 ==> delay = 0;

	delay = delay_dft_func(x_corr_f, dec_str_1, dec_str_2);

	system("pause");
	return 0;
}

