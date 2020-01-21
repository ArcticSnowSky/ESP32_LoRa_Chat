#ifndef __DUALSTREAM_H__
#define __DUALSTREAM_H__

/*
 * DualStream
 * 
 * Merge two streams into one
 *
 * by Thomas Mailänder <mailaender.t@gmail.com>
 * 21.01.2020
*/

#include "Stream.h"
#include "stdio.h"


class DualStream: public Stream {
public:
    DualStream(Stream& s1, Stream& s2) : s1(s1), s2(s2) {}

    template<class T>
    DualStream& operator<<(const T& x) {
		s1 << x;
        s2 << x;
        return *this;
    }
	template<class T>
    DualStream& operator>>(const T& x) {
		s1 >> x;
        s2 >> x;
        return *this;
    }
	int available() { return s1.available() || s2.available(); }
	size_t write(uint8_t d) {
		size_t ret;
		ret |= s1.write(d);
        ret |= s2.write(d);
		return ret;
	}
	int read() {
        int val = s1.read();
        if (val == EOF)
            val = s2.read();
        return val;
	}
    int peek() {
        int val = s1.peek();
        if (val == EOF)
            val = s2.peek();
        return val;
	}
    void flush() {
		s1.flush();
        s2.flush();
	}
private:
    Stream &s1, &s2;
};

#endif
