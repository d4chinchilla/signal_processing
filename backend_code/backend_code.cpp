
#include "DFT.h"
#include "x_corr.h"

using namespace std;

int sample;

vector<double> dec_str_1 = { 225, 10, 193, 242, 130, 148, 14, 21, 149, 194, 93, 145, 99, 61, 9, 243, 189, 116, 60, 77, 130, 23, 230, 130, 150, 172, 131, 223, 230, 110, 125, 207, 244, 15, 240, 247, 45, 150, 233, 120, 85, 30, 100, 215, 193, 235, 41, 144, 133, 210 };
vector<double> dec_str_2 = { 66, 73, 132, 26, 223, 123, 155, 176, 139, 211, 226, 115, 129, 202, 247, 5, 250, 251, 50, 153, 246, 124, 92, 36, 90, 233, 188, 230, 45, 137, 141, 219,124, 214, 75, 201, 195, 28, 107, 174, 37, 49, 65, 74, 80, 246, 29, 13, 142, 225};

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

