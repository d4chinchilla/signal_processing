#include "DFT.h"

using namespace std;
constexpr double PI = 3.14159265358979323846;

vector<double> c_dft_re(const vector<double> &dec_in) //compute Discrete Fourier Transform, real part only, by using the vector that have been passed
{
	vector<double> re_freq_temp; //temp vector

	for (int i = 0; i < dec_in.size(); i++)
	{
		re_freq_temp.push_back(0); //allocate memory
	}

	//compute Discrete Fourier Transform
	for (int i = 0; i < dec_in.size(); i++)
	{
		for (int j = 0; j < dec_in.size(); j++)
		{
			re_freq_temp[i] += dec_in[j] * cos( (2 * PI*i*j) / dec_in.size());
		}

		cout << setprecision(6) << "re_freq_temp " << i << "is: " << re_freq_temp[i] << endl; //cout for display data and checking
	}

	cout << endl;

	return re_freq_temp; //return DFT_re
}

vector<double> c_dft_im(const vector<double> &dec_in) //compute Discrete Fourier Transform, imaginary part only, by using the vector that have been passed
{
	vector<double> im_freq_temp; //temp vector

	for (int i = 0; i < dec_in.size(); i++)
	{
		im_freq_temp.push_back(0); //allocate memory
	}

	//compute Discrete Fourier Transform
	for (int i = 0; i < dec_in.size(); i++)
	{
		for (int j = 0; j < dec_in.size(); j++)
		{
			im_freq_temp[i] += -dec_in[j] * sin( (2 * PI*i*j) / dec_in.size());
		}

		cout << setprecision(6) << "im_freq_temp " << i << "is: " << im_freq_temp[i] << "j" << endl; //cout for display data and checking
	}

	cout << endl;

	return im_freq_temp; //return DFT_im
}

vector<double> i_dft(const vector<double> &re_freq,const vector<double> &im_freq) //compute inverse Discrete Fourier Transform, by using the vector of DFT_re & DFT_im that have been passed
{
	
	int vec_size;

	//store the biggest size of vector
	if (re_freq.size() >= im_freq.size())
	{
		vec_size = re_freq.size();
	}
	else
	{
		vec_size = im_freq.size();
	}

	//temp vector
	vector<double> re_freq_temp;
	vector<double> im_freq_temp;
	vector<double> i_dft_temp;

	re_freq_temp = re_freq;
	im_freq_temp = im_freq;

	//allocate memory
	for (int i = 0; i < vec_size; i++)
	{
		i_dft_temp.push_back(0);
	}
	
	//compute inverse Discrete Fourier Transform
	for (int j = 0; j < vec_size; j++)
	{
		for (int i = 0; i < vec_size; i++)
		{
			i_dft_temp[j] += re_freq[i] * cos( (2 * PI*i*j) / vec_size);
			i_dft_temp[j] += -im_freq[i] * sin( (2 * PI*i*j) / vec_size);
		}

		i_dft_temp[j] /= vec_size;
		
		cout << setprecision(6) << "i_dft_temp " << j << " is: " << i_dft_temp[j] << endl; //cout for display data and checking
	}
	cout << endl;

	return i_dft_temp; //return the result of inverse DFT
}