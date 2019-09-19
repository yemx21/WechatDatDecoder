#include "FileStream.h"
#include <exception>
#include <string>
using namespace std;
using namespace System::IO;

__inline bool IsValidAddress(const void* lp, unsigned int nBytes, bool readWrite = true)
{
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!readWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
};

FileStream::FileStream() : _fileHandle(Invalid_FileHandle), _autoClose(false), _buffer(nullptr)
{
	_autoClose = false;
}

FileStream::FileStream(FileHandle hFile, bool autoClose) : _buffer(nullptr)
{
	Attach(hFile, autoClose);
}

FileStream::~FileStream()
{
	Detach();
}

FileStream::operator FileHandle()
{
	return _fileHandle;
}

bool FileStream::Open(const wchar_t* filePath, FileAdvancedAccess desiredAccess, FileShare shareMode, FileCreationDisposition creationDistribution, int bufferSize, bool nothrow, LPSECURITY_ATTRIBUTES lpSecurityAttributes, FileAttributes attributes, FileHandle templateFileHandle)
{

	FileHandle hFile = CreateFileW(filePath, (DWORD)desiredAccess, (DWORD)shareMode, lpSecurityAttributes, (DWORD)creationDistribution, (DWORD)attributes, templateFileHandle);

	bool bSuccess = false;
	if (hFile != Invalid_FileHandle)
	{
		bSuccess = true;
		_fileHandle = hFile;
		_path = filePath;
		_autoClose = true;
		_type =(FileType) ::GetFileType(_fileHandle);


		_canRead = Enums::Contains(desiredAccess, FileAdvancedAccess::File_Read_Data) || desiredAccess == FileAdvancedAccess::GenericRead || desiredAccess == FileAdvancedAccess::GenericAll;
		_canWrite = Enums::Contains(desiredAccess, FileAdvancedAccess::File_Write_Data) || desiredAccess == FileAdvancedAccess::GenericWrite || desiredAccess == FileAdvancedAccess::GenericAll;
		_canSeek = _type == FileType::Disk;
		_bufferSize = bufferSize;
		_readPos = 0;
		_readLen = 0;
		_writePos = 0; 
		_isPipe = _type == FileType::Pipe; 
		_pos = 0; 
		_appendStart = -1LL;
	}
	else
	{
		if (!nothrow) throw 1;
	}
	return bSuccess;
}

bool FileStream::Open(const wchar_t* filePath, FileAccess desiredAccess, FileShare shareMode, FileMode mode, int bufferSize, bool nothrow, LPSECURITY_ATTRIBUTES lpSecurityAttributes, FileAttributes attributes, FileHandle templateFileHandle)
{
	FileAdvancedAccess desiredAccessEx = FileAdvancedAccess::File_Generic__ReadWrite;
	switch (desiredAccess)
	{
		case FileAccess::Read:
		{
			desiredAccessEx = FileAdvancedAccess::File_Generic__Read;
			_canRead = true;
			_canWrite = false;
			break;
		}
		case FileAccess::Write:
		{
			desiredAccessEx = FileAdvancedAccess::File_Generic__Write; 
		    _canRead = false;
			_canWrite = true;
			break;
		}
		case FileAccess::All:
		default:
		{
			_canRead = true; 
			_canWrite = true;
			break;
		}	
	}
	bool flag = false;
	if (mode == FileMode::Append)
	{
		mode = FileMode::OpenOrCreate;
		flag = true;
	}

	FileHandle hFile = CreateFileW(filePath, (DWORD)desiredAccessEx, (DWORD)shareMode, lpSecurityAttributes, (DWORD)mode, (DWORD)attributes, templateFileHandle);

	bool bSuccess = false;
	if (hFile != Invalid_FileHandle)
	{
		bSuccess = true;
		_fileHandle = hFile;
		_path = filePath;
		_autoClose = true;
		_type = (FileType)::GetFileType(_fileHandle);

		_canSeek = _type == FileType::Disk;
		_bufferSize = bufferSize;
		_readPos = 0;
		_readLen = 0;
		_writePos = 0;
		_isPipe = _type == FileType::Pipe;
		_pos = 0;
		if (!flag)
		{
			_appendStart = -1LL;
			return true;
		}
		_appendStart = SeekCore(0LL, SeekOrigin::End, nothrow);
	}
	else
	{
		if (!nothrow) throw 1;
	}
	return bSuccess;
}

inline bool FileStream::IsOpen() const
{
	return (_fileHandle != Invalid_FileHandle);
}

inline bool FileStream::IsClosed() const
{
	return (_fileHandle == Invalid_FileHandle);
}

bool FileStream::CanRead() const
{
	return _canRead;
}

bool FileStream::CanWrite() const
{
	return _canWrite;
}

bool FileStream::CanSeek() const
{
	return _canSeek;
}

void FileStream::Attach(FileHandle hFile, bool autoClose)
{
	_fileHandle = hFile;
	_path.clear(); 
	_autoClose = autoClose;
}

void FileStream::Detach()
{
	if (IsOpen() && _autoClose)
		Close();
	else
	{
		_fileHandle = Invalid_FileHandle;
		_autoClose = false;
		_path.clear();
	}
	if (_buffer)
	{
		delete _buffer;
		_buffer = nullptr;
	}
}

void FileStream::Close(bool nothrow)
{
	if (!IsOpen()) return;

	bool bError = !::CloseHandle(_fileHandle);

	_fileHandle = Invalid_FileHandle;
	_autoClose = false;
	_path.clear();

	if (bError && !nothrow)
		throw 1;
	if (_buffer)
	{
		delete _buffer;
		_buffer = nullptr;
	}
}

bool FileStream::Write(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	if (buffer == nullptr)
	{
		if (!nothrow) throw std::exception("empty buffer");
		return false;
	}

	if (offset + count > size)
	{
		if (!nothrow) throw std::exception("buffer overflow");
		return false;
	}

	if (_writePos == 0)
	{
		if (!_canWrite)
		{
			if (!nothrow) throw std::exception("can not write");
			return false;
		}
		if (_readPos < _readLen)
		{
			FlushRead();
		}
		_readPos = 0;
		_readLen = 0;
	}
	if (_writePos > 0)
	{
		long num = _bufferSize - _writePos;
		if (num > 0)
		{
			if (num > count)
			{
				num = count;
			}
			memcpy(_buffer + _writePos, buffer + offset, num);
			_writePos += num;
			if (count == num)
			{
				return true;
			}
			offset += num;
			count  -= num;
		}

		WriteCore(_buffer, _bufferSize, 0, _writePos, nothrow);
		_writePos = 0;
	}
	if (count >= _bufferSize)
	{
		WriteCore(buffer, size, offset, count);
		return true;
	}
	if (count == 0)
	{
		return true;
	}
	if (_buffer == nullptr)
	{
		_buffer = new char[_bufferSize];
	}
	memcpy(_buffer + _writePos, buffer + offset, count);
	_writePos = count;
	return true;
}

bool FileStream::WriteByte(int byte, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0;
	}

	if (_writePos == 0)
	{
		if (!_canWrite)
		{
			if (!nothrow) throw std::exception("can not write");
			return false;
		}
		if (_readPos < _readLen)
		{
			FlushRead();
		}
		_readPos = 0;
		_readLen = 0;
		if (_buffer == nullptr)
		{
			_buffer = new char[_bufferSize];
		}
	}
	if (_writePos == _bufferSize)
	{
		FlushWrite(nothrow);
	}
	_buffer[_writePos] = byte;
	_writePos++;
	return true;
}

unsigned long FileStream::Read(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0;
	}

	if (buffer == nullptr)
	{
		if (!nothrow) throw std::exception("empty buffer");
		return false;
	}

	if (offset + count > size)
	{
		if (!nothrow) throw std::exception("buffer overflow");
		return false;
	}

	bool flag = false;
	unsigned long num = (unsigned long)(_readLen - _readPos);
	if (num == 0)
	{
		if (!_canRead)
		{
			if (!nothrow) throw std::exception("can not read");
			return 0UL;
		}
		if (_writePos > 0UL)
		{
			FlushWrite(nothrow);
		}
		if (!_canSeek || count >= _bufferSize)
		{
			num = ReadCore(buffer, size, offset, count, nothrow);
			_readPos = 0;
			_readLen = 0;
			return num;
		}		
		if (_buffer == nullptr)
		{
			_buffer = new char[_bufferSize];
		}
		num = ReadCore(_buffer, _bufferSize, 0, _bufferSize, nothrow);
		if (num == 0UL)
		{
			return 0;
		}
		flag = (int)num < _bufferSize;
		_readPos = 0;
		_readLen = num;
	}
	if (num > count)
	{
		num = count;
	}
	memcpy(buffer + offset, _buffer + _readPos, num);
	_readPos += num;
	if (!_isPipe && num < count && !flag)
	{
		unsigned long num1 = ReadCore(buffer, size, offset + num, count - num, nothrow);
		num = num + num1;
		_readPos = 0;
		_readLen = 0;
	}
	return (unsigned long)num;
}

int FileStream::ReadByte(bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return -1;
	}

	if (_readLen == 0 && !_canRead)
	{
		if (!nothrow) throw std::exception("can not read");
		return -1;
	}

	if (_readPos == _readLen)
	{
		if (_writePos > 0)
		{
			FlushWrite(nothrow);
		}
		if (_buffer == nullptr)
		{
			_buffer = new char[_bufferSize];
		}
		_readLen = ReadCore(_buffer, _bufferSize, 0, _bufferSize, nothrow);
		_readPos = 0;
	}
	if (_readPos == _readLen)
	{
		return -1;
	}
	int num = (int) _buffer[_readPos];
	_readPos ++;
	return num;
}

unsigned long FileStream::ReadCore(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	if (buffer == nullptr)
	{
		if (!nothrow) throw std::exception("empty buffer");
		return false;
	}

	if (offset + count > size)
	{
		if (!nothrow) throw std::exception("buffer overflow");
		return false;
	}


	if (!IsValidAddress(buffer + offset, count))
	{
		if (!nothrow) throw std::overflow_error("invalid buffer or not enough space");
		return false;
	}

	unsigned long dwRead;
	if (!::ReadFile(_fileHandle, buffer + offset, count, &dwRead, NULL))
	{
		if (!nothrow) throw 1;
		return false;
	}

	_pos += (unsigned long long)dwRead;
	return dwRead;
}

bool FileStream::WriteCore(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	if (buffer == nullptr)
	{
		if (!nothrow) throw std::exception("empty buffer");
		return false;
	}

	if (offset + count > size)
	{
		if (!nothrow) throw std::exception("buffer overflow");
		return false;
	}

	if (!IsValidAddress(buffer + offset, count, false))
	{
		if (!nothrow) throw std::overflow_error("invalid buffer or not enough space");
		return false;
	}

	DWORD nWritten;
	if (!::WriteFile(_fileHandle, buffer + offset, count, &nWritten, NULL))
	{
		if (!nothrow) throw 1;
		return false;
	}

	if (nWritten != count)
	{
		if (!nothrow) throw std::exception("disk is full");
		return false;
	}

	_pos += (unsigned long long)nWritten;
	return true;
}

void FileStream::FlushWrite(bool nothrow)
{
	WriteCore(_buffer, _bufferSize, 0, _writePos, nothrow);
	_writePos = 0;
}

void FileStream::FlushRead()
{
	if (_readPos - _readLen != 0)
	{
		SeekCore((long)(_readPos - _readLen), SeekOrigin::Current);
	}
	_readPos = 0;
	_readLen = 0;
}

unsigned long long FileStream::SeekCore(long long offset, SeekOrigin origin, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	LARGE_INTEGER li;
	li.QuadPart = offset;
	li.LowPart = ::SetFilePointer(_fileHandle, li.LowPart, &li.HighPart, (DWORD)origin);
	DWORD errorCode = GetLastError();
	if (li.LowPart == INVALID_SET_FILE_POINTER &&  errorCode != NO_ERROR)
	{
		if (!nothrow) throw errorCode;
		li.QuadPart = -1;
	}
	_pos = li.QuadPart;
	return _pos;
}

unsigned long long FileStream::Seek(long long offset, SeekOrigin origin, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0UL;
	}

	if (!_canSeek)
	{
		if(!nothrow) throw std::exception("can not seek");
	}

	if (_writePos > 0)
	{
		FlushWrite(nothrow);
	}
	else if (origin == SeekOrigin::Current)
	{
		offset = offset - (long long)(_readLen - _readPos);
	}

	long long num = _pos + (long long)(_readPos - _readLen);
	unsigned long long num1 = SeekCore(offset, origin);
	if (_appendStart != -1LL && (long long)num1 < _appendStart)
	{
		SeekCore(num, SeekOrigin::Begin, nothrow);
		if (!nothrow) throw std::exception("file seek append overwrite");
		return 0UL;
	}

	if (_readLen > 0)
	{
		if (num == num1)
		{
			if (_readPos > 0)
			{
				memcpy(_buffer, _buffer + _readPos, _readLen - _readPos);
				_readLen -= _readPos;
				_readPos = 0;
			}
			if (_readLen > 0)
			{
				SeekCore((long long)_readLen, SeekOrigin::Current, nothrow);
			}
		}
		else if (num - (long long)_readPos >= num1 || num1 >= num + (long long)_readLen - (long long)_readPos)
		{
			_readPos = 0;
			_readLen = 0;
		}
		else
		{
			long long num2 = (long long)(num1 - num);
			memcpy(_buffer, _buffer + _readPos + num2, _readLen - (_readPos + num2));
			_readLen = _readLen - (_readPos + num2);
			_readPos = 0;
			if (_readLen > 0)
			{
				SeekCore((long long)_readLen, SeekOrigin::Current, nothrow);
			}
		}
	}
	return num1;
}

unsigned long long FileStream::SeekToEnd()
{
	return Seek(0, SeekOrigin::End);
}

void FileStream::SeekToBegin()
{
	Seek(0, SeekOrigin::Begin);
}

bool FileStream::SetLengthCore(unsigned long long newLength, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	unsigned long long num = _pos;
	if (_pos != newLength)
	{
		Seek(newLength, SeekOrigin::Begin, nothrow);
	}
	

	if (!::SetEndOfFile(_fileHandle))
	{
		if (!nothrow) throw 1;
		return false;
	}

	if (num != newLength)
	{
		if (num < newLength)
		{
			SeekCore(num, SeekOrigin::Begin, nothrow);
			return true;
		}
		SeekCore(0LL, SeekOrigin::End, nothrow);
	}

	return true;
}

bool FileStream::SetLength(unsigned long long newLength, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0UL;
	}

	if (!_canSeek)
	{
		throw std::exception("can not seek");
	}

	if (!_canWrite)
	{
		throw std::exception("can not write");
	}

	if (_writePos > 0)
	{
		FlushWrite(nothrow);
	}
	else if (_readPos < _readLen)
	{
		FlushRead();
	}
	_readPos = 0;
	_readLen = 0;

	if (_appendStart != -1LL && (long long)newLength < _appendStart)
	{
		if (!nothrow) throw std::exception("file seek append overwrite");
		return false;
	}

	return SetLengthCore(newLength, nothrow);
}

unsigned long long FileStream::GetLength(bool nothrow) const
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}
	LARGE_INTEGER li;
	li.QuadPart = 0;

	DWORD dwHighLength;
	li.LowPart = GetFileSize(_fileHandle, &dwHighLength);
	if (li.LowPart == (DWORD)-1)
	{
		if (!nothrow) throw 1;
		return false;
	}
	li.HighPart = dwHighLength;

	if (_writePos > 0 && _pos + _writePos > li.QuadPart)
	{
		li.QuadPart = _writePos + _pos;
	}

	return li.QuadPart;
}

unsigned long long FileStream::GetPosition(bool nothrow) const
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0UL;
	}

	if (!_canSeek)
	{
		if (!nothrow) throw std::exception("can not seek");
		return 0UL;
	}

	return _pos + (long)(_readPos - _readLen + _writePos);
}

bool FileStream::SetPosition(unsigned long long newPos, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return 0UL;
	}

	if (_writePos > 0)
	{
		FlushWrite(nothrow);
	}
	_readPos = 0;
	_readLen = 0;
	return Seek(newPos, SeekOrigin::Begin, nothrow) !=0ull;
}

void FileStream::FlushCore()
{
	if (_writePos > 0)
	{
		FlushWrite();
		return;
	}
	if (_readPos < _readLen && _canSeek)
	{
		FlushRead();
	}
}

bool FileStream::Flush(bool nothrow)
{
	return Flush(false, nothrow);

}

bool FileStream::Flush(bool flushToDisk, bool nothrow)
{
	if (IsClosed())
	{
		if (!nothrow) throw std::exception("file is closed");
		return false;
	}
	FlushCore();
	if (flushToDisk && _canWrite)
	{
		if (!::FlushFileBuffers(_fileHandle))
		{
			if (!nothrow) throw 1;
			return false;
		}
	}
	return true;
}

FileStream* FileStream::Duplicate(bool nothrow) const
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	FileStream* pFile = new FileStream();
	FileHandle hFile;
	FileHandle hProcess = GetCurrentProcess();
	if (!::DuplicateHandle(hProcess, _fileHandle, hProcess, &hFile, 0, false, DUPLICATE_SAME_ACCESS))
	{
		delete pFile;
		if (!nothrow) throw 1;
		return nullptr;
	}
	pFile->_fileHandle = hFile;
	pFile->_path = _path;
	pFile->_autoClose = _autoClose;
	return pFile;
}

void FileStream::Abort()
{
	if (IsOpen())
	{
		// close but ignore errors
		::CloseHandle(_fileHandle);
		_fileHandle = Invalid_FileHandle;
		_autoClose = false;
		_path.clear();
	}
}

bool FileStream::LockRange(const unsigned long long& lPos, const unsigned long long& lCount, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	LARGE_INTEGER li_pos;
	li_pos.QuadPart = lPos;

	LARGE_INTEGER li_count;
	li_count.QuadPart = lCount;

	if (!::LockFile(_fileHandle, li_pos.LowPart, li_pos.HighPart, li_count.LowPart, li_count.HighPart))
	{
		if (!nothrow) throw 1;
		return false;
	}
	return true;
}

bool FileStream::UnlockRange(const unsigned long long& lPos, const unsigned long long& lCount, bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return false;
	}

	LARGE_INTEGER li_pos;
	li_pos.QuadPart = lPos;

	LARGE_INTEGER li_count;
	li_count.QuadPart = lCount;

	if (!::UnlockFile(_fileHandle, li_pos.LowPart, li_pos.HighPart, li_count.LowPart, li_count.HighPart))
	{
		if (!nothrow) throw 1;
		return false;
	}
	return true;
}

FileType FileStream::GetFileType(bool nothrow)
{
	if (!IsOpen())
	{
		if (!nothrow) throw std::exception("file is not opened");
		return FileType::Unknown;
	}

	return _type;
}