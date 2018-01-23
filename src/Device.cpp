#include <Device.h>

// ctor
Device::Device() : value(0.0) {}

// validasi
bool Device::IsPermitted()
{
	bool valid = (assignedRange.size() == nyala);

	for (Interval& p : assignedRange)
		if (p.begin < permittedRange.begin
			|| p.end > permittedRange.end)
			valid = false;

	return valid;
}

// cek wajib / opsional
bool Device::IsWajib() { return wajib; }

// geser waktu pemakaian
// mengembalikan hasil geseran (bisa atau tidak)
bool Device::shift(Interval& intv)
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

bool operator< (Device& a, Device& b)
{
	return (a.IsWajib() > b.IsWajib()) || (a.value < b.value);
}