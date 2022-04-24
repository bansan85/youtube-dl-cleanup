// YoutubeDlCleanUp.cpp : Defines the entry point for the application.
//

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <queue>
#include <vector>
#include <ranges>

namespace fs = std::filesystem;

#include <windows.h>

std::vector<std::wstring_view> extension_movies{ L"mkv", L"mp4", L"webm" };
std::vector<std::wstring_view> all_extensions{ L"mkv", L"mp4", L"webm", L"m4a" };

static bool isMovie(std::wstring_view ext)
{
	return std::find(extension_movies.begin(), extension_movies.end(), ext) != extension_movies.end();
}

static bool isMoviesEntiredDownloaded(std::wstring_view ext)
{
	return std::find(extension_movies.begin(), extension_movies.end(), ext) != extension_movies.end();
}

template <class Type>
struct S;

static bool isFxxx(std::wstring_view ext)
{
	return ext.starts_with(L"f") && ext.length() == 4 &&
		std::all_of(ext.begin() + 1, ext.end(), [](wchar_t c) {return std::isdigit(c); });
}

static bool isPartFrag(std::wstring_view ext)
{
	return ext.starts_with(L"part-Frag") &&
		std::all_of(ext.begin() + 9, ext.end(), [](wchar_t c) {return std::isdigit(c); });
}

static bool isMoviesPartialDownloaded(std::wstring_view filename)
{
	auto ints = filename | std::views::split('.');
	std::vector<std::wstring_view> v;
	for (auto txt : ints)
		v.push_back(txt);
	/*
.fxxx.webm.part-Fragyyy.part
.fxxx.webm.part-Fragyyy
.fxxx.webm.part
*/

	if (v.back() == L"ytdl")
		return true;

	if (v.size() >= 3 && isMovie(v.back()))
	{
		// .temp.mkv
		if (v.end()[-2] == L"temp")
		{
			return true;
		}
		// .fxxx.m4a
		// .fxxx.webm
		if (isFxxx(v.end()[-2]))
		{
			return true;
		}
	}

	// .fxxx.webm.part
	// .fxxx.webm.part-Fragyyy
	if (v.size() >= 4)
	if (v.size() >= 4 && isFxxx(v.end()[-3]) && isMovie(v.end()[-2]) && (v.back() == L"part" || isPartFrag(v.back())))
	{
		return true;
	}

	// .fxxx.webm.part-Fragyyy.part
	if (v.size() >= 5 && isFxxx(v.end()[-4]) && isMovie(v.end()[-3]) && isPartFrag(v.end()[-2]) && v.back() == L"part")
	{
		return true;
	}

	return false;
}

static void RemoveFile(std::wstring_view file, bool dry_run)
{
	std::wcout << file << std::endl;
		if (!dry_run)
			fs::remove(file);
}

int main(int argc, char* argv[])
{
	std::queue<fs::path> folders;

	std::locale::global(std::locale(""));

	bool r = false;
	bool dry_run = false;
	for (int i = 1; i < argc; i++)
	{
		if (std::string_view("-r") == argv[i])
			r = true;
		else if (std::string_view("--dry-run") == argv[i])
			dry_run = true;
		else if (!fs::is_directory(argv[i]))
		{
			std::cerr << "\"" << argv[i] << "\" is not a folder.\n";
			return 1;
		}
		else
			folders.push(argv[i]);
	}

	if (folders.empty())
	{
		std::cout << "Usage: youtubedl-cleanup.exe [--dry-run] [-r] path [more_path]\n";
		return 1;
	}

	while (!folders.empty())
	{
		auto path = folders.front();
		std::map<std::wstring, std::vector<std::wstring>> file_ext;
		for (const auto& entry : fs::directory_iterator(path))
		{
			try
			{
				if (entry.is_directory())
				{
					if (r)
						folders.push(entry);
				}
				else
				{
					auto ext = entry.path().extension().wstring();
					if (isMoviesPartialDownloaded(entry.path().wstring()))
					{
						RemoveFile(entry.path().wstring(), dry_run);
					}
					else if (ext.length() > 1 && isMoviesEntiredDownloaded(std::wstring_view{ ext }.substr(1)))
					{
						auto root = (entry.path().parent_path() / entry.path().filename().replace_extension("")).wstring();
						if (!file_ext.contains(root))
							file_ext.insert({ root, {ext} });
						else
							file_ext[root].push_back(ext);
					}
				}
			}
			catch (const std::system_error&)
			{
			}
		}

		for (const auto& [file, ext] : file_ext)
		{
			if (ext.size() < 2)
				continue;

			std::map<std::uint64_t, std::wstring> times;

			for (const auto& exti : ext)
			{
				HANDLE hFile;
				FILETIME ftCreate;
				std::wstring filei = file + exti;
				hFile = CreateFileW(filei.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile == INVALID_HANDLE_VALUE)
				{
					std::wcerr << "Could not open file \"" << filei << "\", error " << GetLastError() << "\n";
					return 1;
				}

				if (!GetFileTime(hFile, &ftCreate, NULL, NULL))
				{
					std::wcerr << "Corrumpted file \"" << filei << "\"\n";
					return 1;
				}

				CloseHandle(hFile);

				times.insert({ (static_cast<std::uint64_t>(ftCreate.dwHighDateTime) << 32L) + ftCreate.dwLowDateTime, exti });
			}
			RemoveFile(file.c_str() + times.begin()->second, dry_run);
		}

		folders.pop();
	}

	return 0;
}
