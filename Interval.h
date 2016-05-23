#ifndef INTV_H
#define INTV_H

class Interval {
public:
	int begin, end;

	Interval() {}

	Interval (int begin, int end) 
	{
		this->begin = begin;
		this->end = end;
	}

	bool IsInside (int cur)
	{
		return cur >= begin && cur < end;
	}

	bool IsCollide(const Interval& intv)
	{
		return IsInside(intv.begin) || IsInside(intv.end - 1);
	}

	void operator++() { begin++, end++; }

	void operator--() { begin--, end--; }
};

bool operator< (const Interval& lhs, const Interval& rhs)
{
	return (lhs.begin < rhs.begin) || (lhs.end < rhs.end);
}

#endif