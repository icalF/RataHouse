#include <Interval.h>

Interval::Interval() {}

Interval::Interval(int begin, int end)
{
	this->begin = begin;
	this->end = end;
}

bool Interval::IsInside(int cur)
{
	return cur >= begin && cur < end;
}

bool Interval::IsCollide(const Interval& intv)
{
	return IsInside(intv.begin) || IsInside(intv.end - 1);
}

void Interval::operator++() { begin++, end++; }

void Interval::operator--() { begin--, end--; }

bool operator< (const Interval& lhs, const Interval& rhs)
{
	return (lhs.begin < rhs.begin) || (lhs.end < rhs.end);
}