#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include "FileStream.h"

#if _HAS_CXX17
namespace fs = std::filesystem;
#else
namespace fs = std::experimental::filesystem;
#endif

struct XorInfo
{
	std::string Type;
	int Value;

	XorInfo() :Type{}, Value{ -1 } {}
	XorInfo(const std::string& t, int v) :Type(t), Value(v) {}
};

struct XorInfos
{
	int Value1;
	int Value2;
	int Value3;
	std::string Type;

	XorInfos(int v1, int v2, int v3, const std::string& t) :Value1(v1), Value2(v2), Value3(v3), Type(t) {}
};

static std::vector<XorInfos> FileInfos;

void getAllFileType();

XorInfo getXorFromDir(const std::wstring& path);

XorInfo getXor(const std::wstring& path);

void convert(const std::wstring& src, const std::wstring& dest, int val);

int main()
{
	

	getAllFileType();

	wchar_t result[MAX_PATH];
	auto dir = fs::path(std::wstring(result, GetModuleFileName(NULL, result, MAX_PATH))).parent_path();

	std::cout << dir << std::endl;

	auto tmpxor = getXorFromDir(dir.wstring());
	int  gxor = tmpxor.Value;

	std::vector<fs::path> unprocessed;

	for (const auto& p : fs::recursive_directory_iterator(dir))
	{
		if (p.status().type()!=fs::file_type::directory)
		{
			auto path = p.path();
			std::cout << path << std::endl;
			if (path.extension().compare(L".dat")==0)
			{
				std::cout << "dat file" << std::endl;
				auto cxor = getXor(path.wstring());
				if (gxor != -1) gxor = cxor.Value;
				if (!cxor.Type.empty() && gxor != -1)
				{
					auto target = fs::path(path);
					convert(path.wstring(), target.replace_extension(cxor.Type), gxor);
				}
			}
		}
	}
	if (gxor != -1)
	{
		for (auto& path : unprocessed)
		{
			auto cxor = getXor(path.wstring());
			if (!cxor.Type.empty())
			{
				auto target = fs::path(path);
				convert(path.wstring(), target.replace_extension(cxor.Type), gxor);
			}
		}
	}
}

void convert(const std::wstring& src, const std::wstring& dest, int val)
{
	System::IO::FileStream input;
	if (input.Open(src.c_str(), System::IO::FileAccess::Read, System::IO::FileShare::Read, System::IO::FileMode::Open))
	{
		System::IO::FileStream output;
		if (output.Open(dest.c_str(), System::IO::FileAccess::Write, System::IO::FileShare::Write, System::IO::FileMode::Create))
		{
			char buf[4096];
			int bufsize=0;
			do
			{
				bufsize = input.Read(buf, 4096, 0, 4096);
				if (bufsize == 0) break;
				for (int i = 0; i < bufsize; i++)
				{
					buf[i] = (char)(int)(buf[i] ^ val);
				}
				output.Write(buf, bufsize, 0, bufsize);
			} while (true);
			output.Flush();
			output.Close();
		}
		input.Close();
	}
}


XorInfo getXor(char bytes[4])
{
	for (const auto& type : FileInfos)
	{
		int xor0 = bytes[0] & 0xFF ^ type.Value1;
		int xor1 = bytes[1] & 0xFF ^ type.Value2;
		int xor2 = bytes[2] & 0xFF ^ type.Value3;

		if (xor0 == xor1 && xor1 == xor2) 
		{
			return { type.Type, xor0 };
		}
	}
	return {};
}

XorInfo getXor(const std::wstring& path)
{
	System::IO::FileStream fs;
	if (fs.Open(path.c_str(), System::IO::FileAccess::Read, System::IO::FileShare::Read, System::IO::FileMode::Open))
	{
		char bytes[4];
		if (fs.Read(bytes, 4, 0,  4) == 4)
		{
			auto xors = getXor(bytes);
			if (!xors.Type.empty())
			{
				fs.Close();
				return xors;
			}
		}
		fs.Close();
	}
	return {};
}

XorInfo getXorFromDir(const std::wstring& dir)
{
	for (const auto& p : fs::directory_iterator(dir))
	{
		auto path = p.path();
		if (path.extension().compare(L".dat"))
		{
			auto ret = getXor(path.wstring());
			if (!ret.Type.empty()) return ret;
		}
	}
	return {};
}

inline void AddFileInfo(const std::string& hex, const std::string& ext)
{
	FileInfos.emplace_back(std::stoi(hex.substr(0, 2), 0, 16), std::stoi(hex.substr(2, 2), 0, 16), std::stoi(hex.substr(4, 2), 0, 16), ext);
}

void getAllFileType()
{
	AddFileInfo("ffd8ffe000104a464946",".jpg"); //JPEG (jpg)
	AddFileInfo("89504e470d0a1a0a0000",".png"); //PNG (png)
	AddFileInfo("47494638396126026f01",".gif"); //GIF (gif)
	AddFileInfo("49492a00227105008037",".tif"); //TIFF (tif)
	AddFileInfo("424d228c010000000000",".bmp"); //16色位图(bmp)
	AddFileInfo("424d8240090000000000",".bmp"); //24位位图(bmp)
	AddFileInfo("424d8e1b030000000000",".bmp"); //256色位图(bmp)
	AddFileInfo("41433130313500000000",".dwg"); //CAD (dwg)
	AddFileInfo("3c21444f435459504520",".html"); //HTML (html)
	AddFileInfo("3c21646f637479706520",".htm"); //HTM (htm)
	AddFileInfo("48544d4c207b0d0a0942",".css"); //css
	AddFileInfo("696b2e71623d696b2e71",".js"); //js
	AddFileInfo("7b5c727466315c616e73",".rtf"); //Rich Text Format (rtf)
	AddFileInfo("38425053000100000000",".psd"); //Photoshop (psd)
	AddFileInfo("46726f6d3a203d3f6762",".eml"); //Email [Outlook Express 6] (eml)
	AddFileInfo("d0cf11e0a1b11ae10000",".doc"); //MS Excel 注意：word、msi 和 excel的文件头一样
	AddFileInfo("d0cf11e0a1b11ae10000",".vsd"); //Visio 绘图
	AddFileInfo("5374616E64617264204A",".mdb"); //MS Access (mdb)
	AddFileInfo("252150532D41646F6265",".ps");
	AddFileInfo("255044462d312e360d25",".pdf"); //Adobe Acrobat (pdf)
	AddFileInfo("2e524d46000000120001",".rmvb"); //rmvb/rm相同
	AddFileInfo("464c5601050000000900",".flv"); //flv与f4v相同
	AddFileInfo("00000020667479706973",".mp4");
	AddFileInfo("49443303000000000f76",".mp3");
	AddFileInfo("000001ba210001000180",".mpg"); //
	AddFileInfo("3026b2758e66cf11a6d9",".wmv"); //wmv与asf相同
	AddFileInfo("524946464694c9015741",".wav"); //Wave (wav)
	AddFileInfo("52494646d07d60074156",".avi");
	AddFileInfo("4d546864000000060001",".mid"); //MIDI (mid)
	AddFileInfo("504b0304140000000800",".zip");
	AddFileInfo("526172211a0700cf9073",".rar");
	AddFileInfo("235468697320636f6e66",".ini");
	AddFileInfo("504b03040a0000000000",".jar");
	AddFileInfo("4d5a9000030000000400",".exe");//可执行文件
	AddFileInfo("3c25402070616765206c",".jsp");//jsp文件
	AddFileInfo("4d616e69666573742d56",".mf");//MF文件
	AddFileInfo("3c3f786d6c2076657273",".xml");//xml文件
	AddFileInfo("efbbbf2f2a0d0a53514c",".sql");//xml文件
	AddFileInfo("7061636b616765207765",".java");//java文件
	AddFileInfo("406563686f206f66660d",".bat");//bat文件
	AddFileInfo("1f8b0800000000000000",".gz");//gz文件
	AddFileInfo("6c6f67346a2e726f6f74",".properties");//bat文件
	AddFileInfo("cafebabe0000002e0041",".class");//bat文件
	AddFileInfo("49545346030000006000",".chm");//bat文件
	AddFileInfo("04000000010000001300",".mxp");//bat文件
	AddFileInfo("504b0304140006000800",".docx");//docx文件
	AddFileInfo("d0cf11e0a1b11ae10000",".wps");//WPS文字wps、表格et、演示dps都是一样的
	AddFileInfo("6431303a637265617465",".torrent");
	AddFileInfo("494d4b48010100000200",".264");

	AddFileInfo("6D6F6F76",".mov"); //Quicktime (mov)
	AddFileInfo("FF575043",".wpd"); //WordPerfect (wpd)
	AddFileInfo("CFAD12FEC5FD746F",".dbx"); //Outlook Express (dbx)
	AddFileInfo("2142444E",".pst"); //Outlook (pst)
	AddFileInfo("AC9EBD8F",".qdf"); //Quicken (qdf)
	AddFileInfo("E3828596",".pwl"); //Windows Password (pwl)
	AddFileInfo("2E7261FD",".ram"); //Real Audio (ram)
}
