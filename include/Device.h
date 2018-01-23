#ifndef DEVICE_H
#define DEVICE_H

#include <utility>
#include <string>
#include <vector>

#include <Interval.h>

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
	Device();
	
	// validasi
	bool IsPermitted();

	// cek wajib / opsional
	bool IsWajib();

private:
	// geser waktu pemakaian
	// mengembalikan hasil geseran (bisa atau tidak)
	bool shift(Interval& intv);
};

bool operator< (Device& a, Device& b);

#endif
