#include "DFT.h"

using namespace std;
constexpr double PI = 3.14159265358979323846;

#define DFT_ANG_FREQ_STEP (2.0*PI*DFT_MAX_FREQ/DFT_OUT_LEN)

vector<double> c_dft_re(const vector<double> &dec_in) //compute DTF real part, passing address
{
	vector<double> re_freq_temp;

	for (int i = 0; i < DFT_OUT_LEN; i++)
	{
		re_freq_temp.push_back(0);
	}

	for (int i = 0; i < re_freq_temp.size(); i++)
	{
		for (int j = 0; j < dec_in.size(); j++)
		{
			re_freq_temp[i] += dec_in[j] * cos( (DFT_ANG_FREQ_STEP*(1+i)*j/DFT_SAMPLE_RATE));
		}
	}

	return re_freq_temp;
}

vector<double> c_dft_im(const vector<double> &dec_in) //compute DTF imaginary part, passing address
{
	vector<double> im_freq_temp;

	for (int i = 0; i < DFT_OUT_LEN; i++)
	{
		im_freq_temp.push_back(0);
	}

	for (int i = 0; i < im_freq_temp.size(); i++)
	{
		for (int j = 0; j < dec_in.size(); j++)
		{
			im_freq_temp[i] += -dec_in[j] * sin( (DFT_ANG_FREQ_STEP*(1+i)*j/DFT_SAMPLE_RATE));
		}
	}

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
	
	for (int j = 0; j < vec_size; j++)
	{
		for (int i = 0; i < vec_size; i++)
		{
			i_dft_temp[j] += re_freq[i] * cos( (2 * PI*i*j) / vec_size);
			i_dft_temp[j] += -im_freq[i] * sin( (2 * PI*i*j) / vec_size);
		}

		i_dft_temp[j] /= vec_size;
		
		cout << setprecision(6) << "i_dft_temp " << j << " is: " << i_dft_temp[j] << endl;
	}
	cout << endl;

	return i_dft_temp; //copy result to global variable
}
