#pragma once
#include <Windows.h>
#include <string>
#include <future>

#include <type_traits>

namespace System
{
	namespace Enums
	{
		template<typename T>
		inline typename std::enable_if<std::is_enum<T>::value, bool>::type Contains(T x, T testFlag)
		{
			typedef typename std::underlying_type<T>::type base;

			return static_cast<base>(x & testFlag) == static_cast<base>(testFlag);
		}

		template<typename BASE, typename T>
		inline typename std::enable_if<std::is_convertible<T, BASE>::value, bool>::type Contains(T x, T testFlag)
		{
			return static_cast<BASE>(x & testFlag) == static_cast<BASE>(testFlag);
		}
	};
}

#define ENUM_OPERATORS(T) \
enum class T;	\
	inline T	operator	&	(T x, T y) { return static_cast<T>	(static_cast<intptr_t>(x)& static_cast<intptr_t>(y)); }; \
	inline T	operator	|	(T x, T y) { return static_cast<T>	(static_cast<intptr_t>(x) | static_cast<intptr_t>(y)); }; \
	inline T	operator	^	(T x, T y) { return static_cast<T>	(static_cast<intptr_t>(x) ^ static_cast<intptr_t>(y)); }; \
	inline T	operator	~	(T x) { return static_cast<T>	(~static_cast<intptr_t>(x)); }; \
	inline T&	operator	&=	(T& x, T y) { x = x & y;	return x; }; \
	inline T&	operator	|=	(T& x, T y) { x = x | y;	return x; }; \
	inline T&	operator	^=	(T& x, T y) { x = x ^ y;	return x; }; 

#define ENUM_FLAGS(T) ENUM_OPERATORS(T)


#define ENUM_OPERATORSEX(T, TYPE) \
enum class T: TYPE;	\
	inline T	operator	&	(T x, T y) { return static_cast<T>	(static_cast<TYPE>(x)& static_cast<TYPE>(y)); }; \
	inline T	operator	|	(T x, T y) { return static_cast<T>	(static_cast<TYPE>(x) | static_cast<TYPE>(y)); }; \
	inline T	operator	^	(T x, T y) { return static_cast<T>	(static_cast<TYPE>(x) ^ static_cast<TYPE>(y)); }; \
	inline T	operator	~	(T x) { return static_cast<T>	(~static_cast<TYPE>(x)); }; \
	inline T&	operator	&=	(T& x, T y) { x = x & y;	return x; }; \
	inline T&	operator	|=	(T& x, T y) { x = x | y;	return x; }; \
	inline T&	operator	^=	(T& x, T y) { x = x ^ y;	return x; }; 

#define ENUM_FLAGSEX(T, TYPE) ENUM_OPERATORSEX(T, TYPE)



using namespace std;

namespace System
{
	namespace IO
	{
		typedef HANDLE FileHandle;
		#define Invalid_FileHandle INVALID_HANDLE_VALUE //��Ч�ļ�ָ��

		ENUM_FLAGSEX(FileAdvancedAccess, unsigned long)
			/// <summary>
			/// �߼��ļ�����Ȩ��
			/// </summary>
		enum class FileAdvancedAccess : unsigned long
		{
			//
			// Standart Section
			//

			AccessSystemSecurity = 0x1000000,   // AccessSystemAcl access type
			MaximumAllowed = 0x2000000,     // MaximumAllowed access type

			Delete = 0x10000,
			ReadControl = 0x20000,
			WriteDAC = 0x40000,
			WriteOwner = 0x80000,
			Synchronize = 0x100000,

			StandardRightsRequired = 0xF0000,
			StandardRightsRead = ReadControl,
			StandardRightsWrite = ReadControl,
			StandardRightsExecute = ReadControl,
			StandardRightsAll = 0x1F0000,
			SpecificRightsAll = 0xFFFF,

			File_Read_Data = 0x0001,        // file & pipe
			File_List_Directory = 0x0001,       // directory
			File_Write_Data = 0x0002,       // file & pipe
			File_Add_File = 0x0002,         // directory
			File_Append_Data = 0x0004,      // file
			File_Add_SubDirectory = 0x0004,     // directory
			File_Create_Pipe_Instance = 0x0004, // named pipe
			File_Read_EA = 0x0008,          // file & directory
			File_Write_EA = 0x0010,         // file & directory
			File_Execute = 0x0020,          // file
			File_Traverse = 0x0020,         // directory
			File_Delete_Child = 0x0040,     // directory
			File_Read_Attributes = 0x0080,      // all
			File_Write_Attributes = 0x0100,     // all

			//
			// Generic Section
			//

			GenericRead = 0x80000000,
			GenericWrite = 0x40000000,
			GenericExecute = 0x20000000,
			GenericAll = 0x10000000,

			Specific_Rights_All = 0x00FFFF,
			File_All_Access = StandardRightsRequired | Synchronize | 0x1FF,

			File_Generic__Read = StandardRightsRead | File_Read_Data | File_Read_Attributes | File_Read_EA | Synchronize,

			File_Generic__Write = StandardRightsWrite | File_Write_Data | File_Write_Attributes | File_Write_EA | File_Append_Data | Synchronize,

			File_Generic__ReadWrite = ReadControl | File_Read_Data | File_Write_Data | File_Read_Attributes | File_Write_Attributes | File_Read_EA | File_Write_EA | File_Append_Data | Synchronize,

			File_Generic_Execute = StandardRightsExecute | File_Read_Attributes | File_Execute | Synchronize
		};


		ENUM_OPERATORS(FileAccess)
			/// <summary>
			/// �ļ�����Ȩ��
			/// </summary>
		enum class FileAccess
		{
			Read,
			Write,
			All,
		};

		ENUM_FLAGSEX(FileShare, unsigned long)
			/// <summary>
			/// �ļ�����Ȩ��
			/// </summary>
		enum class FileShare :unsigned long
		{
			/// <summary>
			/// 
			/// </summary>
			None = 0x00000000,
			/// <summary>
			/// Enables subsequent open operations on an object to request read access. 
			/// Otherwise, other processes cannot open the object if they request read access. 
			/// If this flag is not specified, but the object has been opened for read access, the function fails.
			/// </summary>
			Read = 0x00000001,
			/// <summary>
			/// Enables subsequent open operations on an object to request write access. 
			/// Otherwise, other processes cannot open the object if they request write access. 
			/// If this flag is not specified, but the object has been opened for write access, the function fails.
			/// </summary>
			Write = 0x00000002,

			/// <summary>
			/// Enables subsequent open and write operations on an object.
			/// </summary>
			ReadWrite = 0x00000003,

			/// <summary>
			/// Enables subsequent open operations on an object to request delete access. 
			/// Otherwise, other processes cannot open the object if they request delete access.
			/// If this flag is not specified, but the object has been opened for delete access, the function fails.
			/// </summary>
			Delete = 0x00000004
		};

		ENUM_OPERATORS(FileCreationDisposition)
			/// <summary>
			/// �ļ���������
			/// </summary>
		enum class FileCreationDisposition
		{
			/// <summary>
			/// Creates a new file. The function fails if a specified file exists.
			/// </summary>
			New = 1,
			/// <summary>
			/// Creates a new file, always. 
			/// If a file exists, the function overwrites the file, clears the existing attributes, combines the specified file attributes, 
			/// and flags with FILE_ATTRIBUTE_ARCHIVE, but does not set the security descriptor that the SECURITY_ATTRIBUTES structure specifies.
			/// </summary>
			CreateAlways = 2,
			/// <summary>
			/// Opens a file. The function fails if the file does not exist. 
			/// </summary>
			OpenExisting = 3,
			/// <summary>
			/// Opens a file, always. 
			/// If a file does not exist, the function creates a file as if dwCreationDisposition is CREATE_NEW.
			/// </summary>
			OpenAlways = 4,
			/// <summary>
			/// Opens a file and truncates it so that its size is 0 (zero) bytes. The function fails if the file does not exist.
			/// The calling process must open the file with the GENERIC_WRITE access right. 
			/// </summary>
			TruncateExisting = 5
		};

		ENUM_OPERATORS(FileMode)
			/// <summary>
			/// �ļ�ģʽ
			/// </summary>
		enum class FileMode
		{
			CreateNew = 1,
			/// <summary>
			/// Creates a new file, always. 
			/// If a file exists, the function overwrites the file, clears the existing attributes, combines the specified file attributes, 
			/// and flags with FILE_ATTRIBUTE_ARCHIVE, but does not set the security descriptor that the SECURITY_ATTRIBUTES structure specifies.
			/// </summary>
			Create = 2,
			/// <summary>
			/// Opens a file. The function fails if the file does not exist. 
			/// </summary>
			Open = 3,
			/// <summary>
			/// Opens a file, always. 
			/// If a file does not exist, the function creates a file as if dwCreationDisposition is CREATE_NEW.
			/// </summary>
			OpenOrCreate = 4,
			/// <summary>
			/// Opens a file and truncates it so that its size is 0 (zero) bytes. The function fails if the file does not exist.
			/// The calling process must open the file with the GENERIC_WRITE access right. 
			/// </summary>
			Truncate = 5,
			/// <summary>
			/// Opens a file, always. 
			/// If a file does not exist, the function creates a file as if dwCreationDisposition is CREATE_NEW.
			/// Then the file will be seeked to the end.
			/// </summary>
			Append = 6
		};

		ENUM_FLAGSEX(FileAttributes, unsigned long)
			/// <summary>
			/// �ļ�����
			/// </summary>
		enum class FileAttributes :unsigned long
		{
			None = 0x0000000,
			Readonly = 0x00000001,
			Hidden = 0x00000002,
			System = 0x00000004,
			Directory = 0x00000010,
			Archive = 0x00000020,
			Device = 0x00000040,
			Normal = 0x00000080,
			Temporary = 0x00000100,
			SparseFile = 0x00000200,
			ReparsePoint = 0x00000400,
			Compressed = 0x00000800,
			Offline = 0x00001000,
			NotContentIndexed = 0x00002000,
			Encrypted = 0x00004000,
			Write_Through = 0x80000000,
			Overlapped = 0x40000000,
			NoBuffering = 0x20000000,
			RandomAccess = 0x10000000,
			SequentialScan = 0x08000000,
			DeleteOnClose = 0x04000000,
			BackupSemantics = 0x02000000,
			PosixSemantics = 0x01000000,
			OpenReparsePoint = 0x00200000,
			OpenNoRecall = 0x00100000,
			FirstPipeInstance = 0x00080000
		};

		/// <summary>
		/// ������ʼλ��
		/// </summary>
		enum class SeekOrigin
		{ 
			Begin,
			Current,
			End,
		};

		/// <summary>
		/// �ļ�����
		/// </summary>
		enum class FileType
		{
			/// <summary>			
			/// The specified file is a character file, typically an LPT device or a console.
			/// </summary>
			Char =0x0002, 
			/// <summary>			
			/// The specified file is a disk file.
			/// </summary>
			Disk=0x0001,
			/// <summary>			
			/// The specified file is a socket, a named pipe, or an anonymous pipe.
			/// </summary>
			Pipe=0x0003,
			/// <summary>			
			/// Unused.
			/// </summary>
			Remote=0x8000,
			Unknown=0x0000,
		};

		/// <summary>
		/// �ļ�����
		/// </summary>
		class FileStream
		{
		private:
			FileHandle  _fileHandle;
			bool    _autoClose;
			wstring _path;
			char* _buffer; 
			int _readPos;
			long long _readLen;
			long long _writePos;
			int _bufferSize;
			bool _canRead;
			bool _canWrite;
			bool _canSeek;
			bool _isPipe;
			unsigned long long _pos;
			long long _appendStart;
			FileType _type;

		protected:
			/// <summary>
			/// ��ȡ�ֽ�
			/// </summary>
			/// <param name="buffer">�ֽ�</param>
			/// <param name="size">�ֽڴ�С</param>
			/// <param name="offset">�ֽ�ƫ��</param>
			/// <param name="count">�ֽ���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			unsigned long ReadCore(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow = true);

			/// <summary>
			/// д���ֽ�
			/// </summary>
			/// <param name="buffer">�ֽ�</param>
			/// <param name="size">�ֽڴ�С</param>
			/// <param name="offset">�ֽ�ƫ��</param>
			/// <param name="count">�ֽ���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool WriteCore(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow = true);

			/// <summary>
			/// ˢ��д��
			/// </summary>
			/// <param name="nothrow">The nothrow.</param>
			void FlushWrite(bool nothrow = true);

			/// <summary>
			/// ˢ�¶�ȡ
			/// </summary>
			void FlushRead();

			/// <summary>
			/// �������λ��
			/// </summary>
			/// <param name="offset">λ��ƫ��</param>
			/// <param name="origin">λ��.</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>��λ��</returns>
			unsigned long long SeekCore(long long offset, SeekOrigin origin, bool nothrow = true);

			/// <summary>
			/// �����ļ�������
			/// </summary>
			/// <param name="newLength">�³���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool SetLengthCore(unsigned long long newLength, bool nothrow = true);

			/// <summary>
			/// ˢ��
			/// </summary>
			void FlushCore();
		public:
			/// <summary>
			/// ���� <see cref="FileStream"/> ʵ��.
			/// </summary>
			FileStream();

			/// <summary>
			/// ���� <see cref="FileStream"/> ʵ��.
			/// </summary>
			/// <param name="hFile">WindowsAPI �ļ����</param>
			/// <param name="autoClose">�Զ��ر�</param>
			FileStream(FileHandle hFile, bool autoClose = true);

			~FileStream();

			operator FileHandle();

			/// <summary>
			/// ��ָ���ļ�
			/// </summary>
			/// <param name="filePath">�ļ�·��</param>
			/// <param name="desiredAccess">�����ļ�����Ȩ��</param>
			/// <param name="shareMode">�ļ�����ģʽ</param>
			/// <param name="creationDistribution">�ļ�����ģʽ</param>
			/// <param name="bufferSize">�����С</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <param name="lpSecurityAttributes">WindowsAPI ��ȫ���Ծ��</param>
			/// <param name="attributes">�ļ�����</param>
			/// <param name="templateFileHandle">WindowsAPI ��ʱ�ļ����</param>
			/// <returns>bool.</returns>
			bool Open(const wchar_t* filePath, FileAdvancedAccess desiredAccess, FileShare shareMode, FileCreationDisposition creationDistribution, int bufferSize=4096, bool nothrow = true,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
				FileAttributes attributes = FileAttributes::None,
				FileHandle templateFileHandle = nullptr);

			/// <summary>
			/// ��ָ���ļ�
			/// </summary>
			/// <param name="filePath">�ļ�·��</param>
			/// <param name="desiredAccess">�����ļ�����Ȩ��</param>
			/// <param name="shareMode">�ļ�����ģʽ</param>
			/// <param name="creationDistribution">�ļ�����ģʽ</param>
			/// <param name="bufferSize">�����С</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <param name="lpSecurityAttributes">WindowsAPI ��ȫ���Ծ��</param>
			/// <param name="attributes">�ļ�����</param>
			/// <param name="templateFileHandle">WindowsAPI ��ʱ�ļ����</param>
			/// <returns>bool.</returns>
			bool Open(const wchar_t* filePath, FileAccess desiredAccess, FileShare shareMode, FileMode mode, int bufferSize = 4096, bool nothrow = true,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes = nullptr,
				FileAttributes attributes = FileAttributes::None,
				FileHandle templateFileHandle = nullptr);

			/// <summary>
			/// �ж��ļ����Ƿ�ɶ�
			/// </summary>
			bool CanRead() const;

			/// <summary>
			/// �ж��ļ����Ƿ��д
			/// </summary>
			bool CanWrite() const;

			/// <summary>
			/// �ж��ļ����Ƿ�ɿ��
			/// </summary>
			bool CanSeek() const;

			/// <summary>
			/// �ж��ļ����Ƿ��ڴ�״̬
			/// </summary>
			bool IsOpen() const;

			/// <summary>
			/// �ж��ļ����Ƿ��ڹر�״̬
			/// </summary>
			bool IsClosed() const; 

			/// <summary>
			/// ����WindowsAPI �ļ����
			/// </summary>
			/// <param name="hFile">WindowsAPI �ļ����</param>
			/// <param name="autoClose">�Ƿ��Զ��ر�</param>
			void Attach(FileHandle hFile, bool autoClose = true);

			/// <summary>
			/// ����WindowsAPI �ļ����
			/// </summary>
			void Detach();

			/// <summary>
			/// �ر��ļ���
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			void Close(bool nothrow = true);

			/// <summary>
			/// д���ֽ�
			/// </summary>
			/// <param name="buffer">�ֽ�</param>
			/// <param name="size">�ֽڴ�С</param>
			/// <param name="offset">�ֽ�ƫ��</param>
			/// <param name="count">�ֽ���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool Write(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow = true);

			/// <summary>
			/// д��һ���ֽ�
			/// </summary>
			/// <param name="byte">�ֽ�</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool WriteByte(int byte, bool nothrow = true);

			/// <summary>
			/// ��ȡ�ֽ�
			/// </summary>
			/// <param name="buffer">�ֽ�</param>
			/// <param name="size">�ֽڴ�С</param>
			/// <param name="offset">�ֽ�ƫ��</param>
			/// <param name="count">�ֽ���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			unsigned long Read(char* buffer, unsigned long size, unsigned long offset, unsigned long count, bool nothrow = true);

			/// <summary>
			/// ��ȡ��һ���ֽ�
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>���ֽ�</returns>
			int ReadByte(bool nothrow = true);

			/// <summary>
			/// �������λ��
			/// </summary>
			/// <param name="offset">λ��ƫ��</param>
			/// <param name="origin">λ��.</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>��λ��</returns>
			unsigned long long Seek(long long offset, SeekOrigin origin, bool nothrow = true);

			/// <summary>
			/// �������β
			/// </summary>
			/// <returns>��λ��</returns>
			unsigned long long SeekToEnd();

			/// <summary>
			/// �˻����ļ�ͷ
			/// </summary>
			void SeekToBegin();

			/// <summary>
			/// �����ļ�������
			/// </summary>
			/// <param name="newLength">�³���</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool SetLength(unsigned long long newLength, bool nothrow = true);

			/// <summary>
			/// ��ȡ�ļ�������
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>�ļ�������</returns>
			unsigned long long GetLength(bool nothrow = true) const;

			/// <summary>
			/// ��ȡ�ļ���λ��
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			unsigned long long GetPosition(bool nothrow = true) const; 

			/// <summary>
			/// �����ļ���λ��
			/// </summary>
			/// <param name="newPos">��λ��</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool SetPosition(unsigned long long newPos, bool nothrow=true);

			/// <summary>
			/// ˢ��
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>bool.</returns>
			bool Flush(bool nothrow = true);

			/// <summary>
			/// ˢ��
			/// </summary>
			/// <param name="flushToDisk">ˢ�µ�Ӳ��</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			virtual bool Flush(bool flushToDisk, bool nothrow = true);

			/// <summary>
			/// �����ļ���
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>�ļ���</returns>
			FileStream* Duplicate(bool nothrow = true) const;

			/// <summary>
			/// ��ֹ�ļ���
			/// </summary>
			void Abort();

			/// <summary>
			/// ��������
			/// </summary>
			/// <param name="lPos">����λ��</param>
			/// <param name="lCount">��������</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool LockRange(const unsigned long long& lPos, const unsigned long long& lCount, bool nothrow = true);

			/// <summary>
			/// �������
			/// </summary>
			/// <param name="lPos">����λ��</param>
			/// <param name="lCount">��������</param>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns></returns>
			bool UnlockRange(const unsigned long long& lPos, const unsigned long long& lCount, bool nothrow = true);

			/// <summary>
			/// ��ȡ�ļ�����
			/// </summary>
			/// <param name="nothrow">�޴���׽</param>
			/// <returns>�ļ�����</returns>
			FileType GetFileType(bool nothrow = true);
		};
	}
}