#undef UNICODE
#undef _UNICODE

#include <stdio.h>
#include <Windows.h>
#include <string.h>

typedef struct result
{
	int numberOfFiles;
	int numberOfDirectories;
} result;

result searchDir(LPSTR);

int main(int argc, LPSTR argv[])
{
	result res;

	if (argc < 2)
	{
		printf("Nije zadana putanja direktorija.\n");
		return 1;
	}

	res = searchDir(argv[1]);

	if (res.numberOfDirectories == -1 || res.numberOfFiles == -1)
	{
		printf("Greska prilikom otvaranja zadane putanje.\n");
		return 2;
	}

	printf("Broj direktorija: %d\n", res.numberOfDirectories);
	printf("Broj datoteka: %d\n", res.numberOfFiles);
	return 0;
}

result searchDir(LPSTR path)
{
	result res;
	HANDLE hFindFile;
	WIN32_FIND_DATA fileData;

	res.numberOfDirectories = 0;
	res.numberOfFiles = 0;

	if (!SetCurrentDirectory(path))
	{
		res.numberOfDirectories = -1;
		res.numberOfFiles = -1;
		return res;
	}

	hFindFile = FindFirstFile("*", &fileData);
	if (hFindFile == INVALID_HANDLE_VALUE) 
	{
		FindClose(hFindFile);
		return res;
	}

	do
	{
		if ((strcmp(fileData.cFileName, ".") != 0) && (strcmp(fileData.cFileName, "..") != 0))
		{
			if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
				res.numberOfDirectories++;
			else
				res.numberOfFiles++;
		}
	} while (FindNextFile(hFindFile, &fileData));

	FindClose(hFindFile);
	return res;
}