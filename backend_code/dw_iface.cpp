#include "dw_iface.h"

int select_mic()
{
	bool check = 0;
	int mic_no = 0;
	while (check == 0)
	{
		cout << "Please enter 1st mic number to compute: " << endl;
		cin >> mic_no;
		if (mic_no <= 0 || mic_no >= 5)
		{
			cout << "error: selected number out of range (range:1 to 4)" << endl << endl;
		}
		else
		{
			check = 1;
		}
	}
	return mic_no;
}

double cal_amplitude(const vector<double> &data)
{
	vector<double> data_temp = data;

	double data_avg = 0;
	double sum_sqre = 0;
	double amplitude = 0;

	for (int i = 0; i < data_temp.size(); i++)
	{
		data_avg += data_temp[i];
	}

	data_avg /= data_temp.size();

	for (int i = 0; i < data_temp.size(); i++)
	{
		amplitude += (pow(data_temp[i], 2) - data_avg);
	}

	amplitude = 10 * log(amplitude);

	cout << "signal power(in dB) is: " << amplitude << endl << endl;

	return amplitude;
}