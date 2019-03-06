#include "x_corr.h"

//try to use convolution to do cross_correlation, but test failed and not in used
vector<double> x_corr(const vector<double> &corr_a, const vector<double> &corr_b)
{
	int vec_size;

	//get how many sample contain
	if (corr_a.size() >= corr_b.size())
	{
		vec_size = corr_a.size();
	}
	else
	{
		vec_size = corr_b.size();
	}

	//temp vector
	vector<double> corr_a_temp = corr_a;
	vector<double> corr_b_temp = corr_b;

	//insert '0' at the start and the end of the vector with same size of vec_size
	vector<double>::iterator f = corr_a_temp.begin();
	corr_a_temp.insert(f, vec_size, 0);

	for (int i = 0; i < vec_size +1; i++)
	{
		corr_a_temp.push_back(0);
	}

	//insert '0' at the start of the vector with size vec_size*2+1
	for (int i = 0; i < vec_size*2 +1; i++)
	{
		corr_b_temp.push_back(0);
	}

	vector<double> x_corr;
	for (int i = 0; i < vec_size*2 +1; i++)
	{
		x_corr.push_back(0);
	}

	//compute cross_correlation (by convolution)
	for (int i = 0; i < vec_size*2 +1; i++)
	{
		for (int j = 0; j < vec_size*3 +1; j++)
		{
			x_corr[i] += corr_a_temp[j] * corr_b_temp[j];
		}
		cout << setprecision(6) << "x_corr " << i << "is: " << x_corr[i] << endl; //cout for display data and checking

		//shift the vector by insert 0 at the beginning
		vector<double>::iterator s = corr_b_temp.begin();
		corr_b_temp.insert(s, 0);
	}

	cout << endl;

	return x_corr; //return cross_correlation result
}

//do cross_correlation by using DFT
vector<double> x_corr_dft(const vector<double> &corr_a, const vector<double> &corr_b) 
{
	int vec_size;

	//store the biggest size of vector
	if (corr_a.size() >= corr_b.size())
	{
		vec_size = corr_a.size();
	}
	else
	{
		vec_size = corr_b.size();
	}

	//temp vector
	vector<double> corr_a_temp = corr_a;
	vector<double> corr_b_temp = corr_b;

	//insert '0' at the end of the vector with size vec_size*2-1
	for (int i = 0; i < vec_size * 2 - 1; i++)
	{
		corr_a_temp.push_back(0);
		corr_b_temp.push_back(0);
	}

	//temp vector
	vector<double> corr_a_temp_re;
	vector<double> corr_a_temp_im;
	vector<double> corr_b_temp_re;
	vector<double> corr_b_temp_im;

	corr_a_temp_re = c_dft_re(corr_a_temp);
	corr_a_temp_im = c_dft_im(corr_a_temp);

	corr_b_temp_re = c_dft_re(corr_b_temp);
	corr_b_temp_im = c_dft_im(corr_b_temp);

	vector<double> x_corr_dft;

	//do the conjugate for the 2nd DFT_im vector
	vector<double> conj_corr_b = conjugate(corr_b_temp_im);

	//do cross_correlation, result = idft(dft_a * conj(dft_b))
	x_corr_dft = i_dft(dft_multiply_re(corr_a_temp_re, corr_b_temp_re, corr_a_temp_im, conj_corr_b), dft_multiply_im(corr_a_temp_re, corr_b_temp_re, corr_a_temp_im, conj_corr_b));

	for (int i = 0; i < x_corr_dft.size(); i++)
	{
		cout << setprecision(6) << "x_corr_dft " << i << "is: " << x_corr_dft[i] << endl; //cout for display data and checking
	}

	cout << endl;

	return x_corr_dft; //return cross_correlation result
}

//multiply the real part of 2 DFT vector
vector<double> dft_multiply_re(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im)
{
	//temp vector
	vector<double> dft_a_re_temp = dft_a_re;
	vector<double> dft_b_re_temp = dft_b_re;
	vector<double> dft_a_im_temp = dft_a_im;
	vector<double> dft_b_im_temp = dft_b_im;

	vector<double> multi_result_re;

	//allocate memory
	for (int i = 0; i < dft_a_re_temp.size(); i++)
	{
		multi_result_re.push_back(0);
	}
	
	//assume dft_a = (a+bj), dft_b = (c+dj), then result after multiply (real part) = (a*c) + (b*d)*(-1)
	for (int i = 0; i < multi_result_re.size(); i++)
	{
		multi_result_re[i] = dft_a_re_temp[i] * dft_b_re_temp[i] - dft_a_im_temp[i] * dft_b_im_temp[i];
	}

	return multi_result_re; //return the result of multiply of 2 DFT_re
}

//multiply the imaginary part of 2 DFT vector
vector<double> dft_multiply_im(const vector<double> &dft_a_re, const vector<double> &dft_a_im, const vector<double> &dft_b_re, const vector<double> &dft_b_im)
{
	//temp vector
	vector<double> dft_a_re_temp = dft_a_re;
	vector<double> dft_b_re_temp = dft_b_re;
	vector<double> dft_a_im_temp = dft_a_im;
	vector<double> dft_b_im_temp = dft_b_im;

	vector<double> multi_result_im;

	//allocate memory
	for (int i = 0; i < dft_a_re_temp.size(); i++)
	{
		multi_result_im.push_back(0);
	}

	//assume dft_a = (a+bj), dft_b = (c+dj), then result after multiply (imaginary part) = (a*d)*(j) + (b*c)*(j)
	for (int i = 0; i < multi_result_im.size(); i++)
	{
		multi_result_im[i] = dft_a_re_temp[i] * dft_b_im_temp[i] + dft_a_im_temp[i] * dft_b_re_temp[i];
	}

	return multi_result_im; //return the result of multiply of 2 DFT_im
}

//do the conjugate for the 2nd DFT_im vector
vector<double> conjugate(vector<double> &im_part)
{
	vector<double> im_part_temp = im_part;

	//invert the sign for every element in the vector
	for (int i = 0; i < im_part.size(); i++)
	{
		im_part_temp[i] = -im_part_temp[i];
	}

	return im_part_temp; //return result
}

//calculate delay between 2 signal
int delay_dft_func(vector<double> &find_delay, vector<double> &check_data_a, vector<double> &check_data_b)
{
	int delay;
	int vec_size;

	//store the biggest size of vector
	if (check_data_a.size() >= check_data_b.size())
	{
		vec_size = check_data_a.size();
	}
	else
	{
		vec_size = check_data_b.size();
	}

	int count = 0;

	//if 2 set data are the same, just return delay = 0
	for (int i = 0; i < vec_size; i++)
	{

		if (check_data_a[i] == check_data_b[i])
		{
			count += 1;
		}

		if (count == vec_size)
		{
			delay = 0;
			cout << "two sets of data delay is 0" << endl << endl;
			return delay;
		}
	}

	vector<double> find_delay_temp = find_delay;

	//because of the effect of insert '0' and the samples only involved of real numbers, the result of top half of vector = inverse of the result of bottom half of vector.
	//Also, result of cross-correlation are +ve numbers, so using abs() to change the value be +ve
	//In addition, for the result of cross_correlation, if -ve means 1st set of data lag 'n' elements of 2nd data, if +ve means 1st set of data lead 'n' elements of 2nd data
	for (int i = 0; i < find_delay_temp.size(); i++)
	{
		find_delay_temp[i] = abs(find_delay_temp[i]);
	}

	//find maximum value from the result of top half of the vector only. Due to the result of top half of vector = inverse of the result of bottom half of vector.
	double max_val = *max_element(find_delay_temp.begin(), find_delay_temp.begin() + (find_delay_temp.size()/2)-1);

	//compare the value in the result of cross_correlation vector and the maximum value to find the delay
	for (int i = 0; i < find_delay_temp.size(); i++)
	{
		if (max_val == find_delay_temp[i])
		{
			delay = i;
			break;
		}
	}

	//cout for display delay and checking
	if (delay != 0)
	{
		if (find_delay[delay] < 0)
		{
			cout << "1st set of data lag " << delay << " samples compare to 2nd set of data" << endl;
		}
		else
		{
			cout << "1st set of data lead " << delay << " samples compare to 2nd set of data" << endl;
		}
	}

	return delay; //return delay
}