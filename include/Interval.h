#ifndef INTV_H
#define INTV_H

class Interval {
public:
	int begin, end;

	Interval();

	Interval(int begin, int end);

	bool IsInside(int cur);

	bool IsCollide(const Interval& intv);

	void operator++();

	void operator--();
};

bool operator< (const Interval& lhs, const Interval& rhs);

#endif