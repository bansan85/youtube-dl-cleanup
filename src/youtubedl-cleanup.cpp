// YoutubeDlCleanUp.cpp : Defines the entry point for the application.
//

#include <filesystem>
#include <iostream>
#include <map>
#include <optional>
#include <queue>

namespace fs = std::filesystem;

#include <windows.h>

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
		std::map<std::wstring, std::vector<fs::path>> file_ext;
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
					auto ext = entry.path().extension();
					if (ext == ".mkv" || ext == ".mp4" || ext == ".webm")
					{
						auto root = (entry.path().parent_path() / entry.path().filename().replace_extension("")).wstring();
						if (!file_ext.contains(root))
							file_ext.insert({ root, {ext} });
						else
							file_ext[root].push_back(ext);
					}
				}
			}
			catch (const std::system_error&) {}
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
				std::wstring filei = file + exti.wstring();
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

				times.insert({ (static_cast<std::uint64_t>(ftCreate.dwHighDateTime) << 32L) + ftCreate.dwLowDateTime, exti.wstring() });
			}
			std::wcout << file.c_str() << times.begin()->second << std::endl;
			if (!dry_run)
				fs::remove(file + times.begin()->second);
		}

		folders.pop();
	}

	return 0;
}
