#include "dw_iface.h"

//select which microphone data to compute
int select_mic()
{
	bool check = 0;
	int mic_no = 0;
	while (check == 0)
	{
		cout << "Please enter mic number to compute: " << endl;
		cin >> mic_no;

		//check the input is valid or not, because only 4 microphones available
		if (mic_no <= 0 || mic_no >= 5)
		{
			cout << "error: selected number out of range (range:1 to 4)" << endl << endl;
		}
		else
		{
			check = 1;
		}
	}
	return mic_no; //return selected microphone number
}

//calculate the power of signal in dB
double cal_amplitude(const vector<double> &data)
{
	vector<double> data_temp = data;

	double data_avg = 0;
	double sum_sqre = 0;
	double amplitude = 0;

	//calculate the average of data
	for (int i = 0; i < data_temp.size(); i++)
	{
		data_avg += data_temp[i];
	}

	data_avg /= data_temp.size();

	//amplitude = the sum of data[i]^2 - average of data
	for (int i = 0; i < data_temp.size(); i++)
	{
		amplitude += (pow(data_temp[i], 2) - data_avg);
	}

	//calculate in dB
	amplitude = 10 * log(amplitude);

	cout << "signal power(in dB) is: " << amplitude << endl << endl;

	return amplitude;
}