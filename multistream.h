#ifndef __MULTISTREAM_H__
#define __MULTISTREAM_H__

/*
 * MultiStream
 * 
 * Merge multiple streams into one
 *
 * by Thomas Mailänder <mailaender.t@gmail.com>
 * 21.01.2020
*/

#include "Stream.h"
#include "stdio.h"

class MultiStream: public Stream {
public:
    MultiStream(Stream* streams[], int len) : streams(streams), len(len) {}

    template<class T>
    MultiStream& operator<<(const T& x) {
		for (int i = 0; i < len; i++)
        	streams[i] << x;
        return *this;
    }
	template<class T>
    MultiStream& operator>>(const T& x) {
		for (int i = 0; i < len; i++)
        	streams[i] >> x;
        return *this;
    }
	int available() {
		for (int i = 0; i < len; i++) {
        	if (streams[i]->available())
				return true;
		}
        return false;
	}
	size_t write(uint8_t d) {
		size_t ret;
		for (int i = 0; i < len; i++)
			ret = streams[i]->write(d);
		return ret;
	}
	int read() {
		for (int i = 0; i < len; i++) {
			int val = streams[i]->read();
			if (val != EOF)
				return val;
		}
		return EOF;
	}
    int peek() {
		for (int i = 0; i < len; i++) {
			int val = streams[i]->peek();
			if (val != EOF)
				return val;
		}
		return EOF;
	}
    void flush() {
		for (int i = 0; i < len; i++)
			streams[i]->flush();
	}
private:
    Stream** streams;
	int len;
};

#endif
