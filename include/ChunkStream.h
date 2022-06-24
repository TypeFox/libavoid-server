/**
 * @file    ChunkStream.h
 * @author  mri
 *
 * @section LICENSE
 *
 * Copyright (c) 2010, 2022 Kiel University and others.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * @section DESCRIPTION
 *
 * The definition of classes related to the ChunkStream. The ChunkStream
 * encapsulates an input stream. When reading from the stream a special delimiter
 * specified on construction is treated as end-of-file. After such a delimiter has
 * been read no more reading operations can be performed on the stream until the
 * stream is explicitly restarted by moving it to the next chunk of data.
 */

#ifndef __CHUNKSTREAM_H__INCLUDED__
#define __CHUNKSTREAM_H__INCLUDED__

#include <string>
#include <streambuf>
#include <cassert>

template<class _E, class _Tr = std::char_traits<_E> >
class ChunkStreamBuffer: public std::basic_streambuf<_E, _Tr> {
public:
	/** the buffer size. */
	static const size_t BUFF_SIZE = 1024;

	typedef typename _Tr::int_type int_type;
	typedef typename _Tr::pos_type pos_type;
	typedef typename _Tr::off_type off_type;

	/**
	 * Constructs the ChunkStreamBuffer.
	 *
	 * @param buf
	 *            the wrapped stream buffer
	 * @param delimiter
	 *            the delimiter that seperates the chunks; has to be smaller
	 *            than BUFF_SIZE
	 */
	explicit ChunkStreamBuffer(std::basic_streambuf<_E, _Tr>& buf,
			const std::basic_string<_E>& delimiter) :
		mStreamBuf(buf), mInBuf(new _E[BUFF_SIZE + delimiter.size()]),
				mChunkCompleted(false), mRealEof(false) {
		assert(delimiter.size() > 0 && delimiter.size() < BUFF_SIZE);
		mDelimiterLen = delimiter.size();
		// copy the keyword
		mDelimiter = new _E[mDelimiterLen];
		_Tr::copy(mDelimiter, delimiter.c_str(), mDelimiterLen);
		// initialize the get pointer
		this->setg(0, 0, 0);
	}

	virtual ~ChunkStreamBuffer() {
		delete[] mInBuf;
		delete[] mDelimiter;
	}

	/**
	 * Prepares the buffer for reading the next chunk of data.
	 */
	void nextChunk();

	/**
	 * Is the real eof reached?
	 *
	 * @return true if the real eof is reached; false else
	 */
	inline bool isRealEof() {
		return mRealEof;
	}

protected:
	/**
	 * Reads a character from the underlying stream.
	 *
	 * @return the current character at the get pointer pos, if any; eof else
	 */
	virtual int_type underflow();

private:
	/** the wrapped stream buffer. */
	std::basic_streambuf<_E, _Tr>& mStreamBuf;
	/** the chunk seperation delimiter. */
	_E *mDelimiter;
	/** the delimiter length. */
	size_t mDelimiterLen;
	/** the input buffer. */
	_E *mInBuf;
	/** is the current chunk completed? */
	bool mChunkCompleted;
	/** has the stream really reached eof? */
	bool mRealEof;
};

template<class _E, class _Tr>
void ChunkStreamBuffer<_E, _Tr>::nextChunk() {
	// read the rest of the chunk
	int_type c = this->sbumpc();
	while (!_Tr::eq_int_type(c, _Tr::eof())) {
		c = this->sbumpc();
	}
	mChunkCompleted = false;
}

template<class _E, class _Tr>
typename ChunkStreamBuffer<_E, _Tr>::int_type ChunkStreamBuffer<_E, _Tr>::underflow() {
	if (mChunkCompleted || mRealEof) {
		return _Tr::eof();
	}
	// read from the wrapped stream and search for the delimiter
	_E *buf = mInBuf;
	std::streamsize len = 0;
	size_t curDelimiterPos = 0;
	while (buf < mInBuf + BUFF_SIZE) {
		int_type intc = mStreamBuf.sbumpc();
		// eof reached?
		if (_Tr::eq_int_type(intc, _Tr::eof())) {
			mRealEof = true;
			break;
		}
		_E c = _Tr::to_char_type(intc);
		// match the read char with the current delimiter char
		if (_Tr::eq(c, mDelimiter[curDelimiterPos])) {
			++curDelimiterPos;
			// complete delimiter found?
			if (curDelimiterPos == mDelimiterLen) {
				mChunkCompleted = true;
				break;
			}
		} else {
			// insert the prefix of the delimiter which was skipped so far
			if (curDelimiterPos > 0) {
				_Tr::copy(buf, mDelimiter, curDelimiterPos);
				buf += curDelimiterPos;
				len += curDelimiterPos;
				curDelimiterPos = 0;
			}
			// write the current char into the buffer
			*buf = c;
			++buf;
			++len;
		}
	}
	this->setg(mInBuf, mInBuf, mInBuf + len);
	// nothing read -> eof
	if (len == 0) {
		return _Tr::eof();
	}
	return _Tr::not_eof(mInBuf[0]);
}

template<class _E, class _Tr = std::char_traits<_E> >
class ChunkInputStream: public std::basic_istream<_E, _Tr> {
public:
	/**
	 * Constructs the ChunkInputStream.
	 *
	 * @param inputStream
	 *            the wrapped stream
	 * @param keyword
	 *            the keyword that seperates the chunks
	 */
	explicit ChunkInputStream(std::basic_istream<_E, _Tr>& inputStream,
			const std::basic_string<_E>& keyword) :
		streamBuffer(*inputStream.rdbuf(), keyword),
				std::basic_istream<_E, _Tr>(&streamBuffer) {
	}

	/**
	 * Prepares the buffer for reading the next chunk of data.
	 */
	void nextChunk() {
		streamBuffer.nextChunk();
		this->clear();
	}

	/**
	 * Is the real eof reached?
	 *
	 * @return true if the real eof is reached; false else
	 */
	inline bool isRealEof() {
		return streamBuffer.isRealEof();
	}

private:
	/** the chunk stream buffer. */
	ChunkStreamBuffer<_E, _Tr> streamBuffer;
};

typedef ChunkInputStream<char> chunk_istream;
typedef ChunkInputStream<wchar_t> chunk_wistream;

#endif
