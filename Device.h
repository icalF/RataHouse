#ifndef DEVICE_H
#define DEVICE_H

#include "Interval.h"
#include <utility>
#include <string>
#include <vector>

using namespace std;

class Device {
public:
	string name;
	int power;
	int slot;
	Interval permittedRange;
	vector<Interval> assignedRange;
	bool wajib;
	int nyala;
	double value;
	
	// ctor
	Device() : value(0.0) {}
	
	// validasi
	bool IsPermitted() 
	{
		bool valid = (assignedRange.size() == nyala);

		for (Interval& p : assignedRange)
		if (p.begin < permittedRange.begin
			|| p.end > permittedRange.end)
			valid = false;

		return valid;
	}

	// cek wajib / opsional
	bool IsWajib() { return wajib; }

private:
	// geser waktu pemakaian
	// mengembalikan hasil geseran (bisa atau tidak)
	bool shift(Interval& intv)
	{
		intv.begin++;
		intv.end++;
		if (!IsPermitted()) // revert
		{
			intv.begin--;
			intv.end--;
			return false;
		}
		return true;
	}
};

bool operator< (Device& a, Device& b)
{
	return (a.IsWajib() > b.IsWajib()) || (a.value < b.value);
}

#endif
