#include "DFT.h"

using namespace std;
constexpr double PI = 3.14159265358979323846;

vector<double> c_dft_re(const vector<double> &dec_in) //compute DTF real part, passing address
{
	vector<double> re_freq_temp;

	for (int k = 0; k < dec_in.size(); k++)
	{
		re_freq_temp.push_back(0);
	}

	for (int k = 0; k < dec_in.size(); k++)
	{
		for (int i = 0; i < dec_in.size(); i++)
		{
			re_freq_temp[k] += dec_in[i] * cos( (2 * PI*k*i) / dec_in.size());
		}

		cout << setprecision(6) << "re_freq_temp " << k << "is: " << re_freq_temp[k] << endl;
	}

	cout << endl;

	return re_freq_temp;
}

vector<double> c_dft_im(const vector<double> &dec_in) //compute DTF imaginary part, passing address
{
	vector<double> im_freq_temp;

	for (int k = 0; k < dec_in.size(); k++)
	{
		im_freq_temp.push_back(0);
	}

	for (int k = 0; k < dec_in.size(); k++)
	{
		for (int i = 0; i < dec_in.size(); i++)
		{
			im_freq_temp[k] += -dec_in[i] * sin( (2 * PI*k*i) / dec_in.size());
		}

		cout << setprecision(6) << "im_freq_temp " << k << "is: " << im_freq_temp[k] << "i" << endl;
	}

	cout << endl;

	return im_freq_temp;
}

vector<double> i_dft(const vector<double> &re_freq,const vector<double> &im_freq) //compute invert DTF, passing address
{
	
	int vec_size;

	//get how many sample contain
	if (re_freq.size() >= im_freq.size())
	{
		vec_size = re_freq.size();
	}
	else
	{
		vec_size = im_freq.size();
	}

	//ensure the original data do not change
	vector<double> re_freq_temp;
	vector<double> im_freq_temp;
	vector<double> i_dft_temp;

	re_freq_temp = re_freq;
	im_freq_temp = im_freq;

	for (int i = 0; i < vec_size; i++)
	{
		i_dft_temp.push_back(0);
	}
	
	for (int i = 0; i < vec_size; i++)
	{
		for (int k = 0; k < vec_size; k++)
		{
			i_dft_temp[i] += re_freq[k] * cos( (2 * PI*k*i) / vec_size);
			i_dft_temp[i] += -im_freq[k] * sin( (2 * PI*k*i) / vec_size);
		}

		i_dft_temp[i] /= vec_size;
		
		cout << setprecision(6) << "i_dft_temp " << i << " is: " << i_dft_temp[i] << endl;
	}
	cout << endl;

	return i_dft_temp; //copy result to global variable
}