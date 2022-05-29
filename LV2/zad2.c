#undef UNICODE
#undef _UNICODE

#include <stdio.h>
#include <Windows.h>
#include <string.h>

typedef struct result
{
	int numberOfLageFiles;
	FILETIME t;
} result;

int r;
int searchDir(LPSTR, result*);

int main(int argc, LPSTR argv[])
{
	result res;
	SYSTEMTIME systime;

	res.numberOfLageFiles = 0;
	res.t.dwHighDateTime = 0xffffffff;
	res.t.dwLowDateTime = 0xffffffff;

	if (argc < 2)
	{
		printf("Nije zadana putanja direktorija.\n");
		return 1;
	}

	r = searchDir(argv[1], &res);

	if (r == 1)
	{
		printf("Greska prilikom otvaranja zadane putanje.\n");
		return 2;
	}

	if (r == 2)
	{
		printf("Direktorij je prazan.\n");
		return 3;
	}

	printf("Broj velikih datoteka: %d\n", res.numberOfLageFiles);
	FileTimeToSystemTime(&(res.t), &systime);
	printf("Vrijeme kreiranja najstarije datoteke: %d.%d.%d.\n", systime.wDay, systime.wMonth, systime.wYear);
	return 0;
}

int searchDir(LPSTR path, result* res)
{
	printf("%s\n", path);
	HANDLE hFindFile;
	WIN32_FIND_DATA fileData;

	if (!SetCurrentDirectory(path))
	{
		return 1;
	}

	hFindFile = FindFirstFile("*", &fileData);
	if (hFindFile == INVALID_HANDLE_VALUE) //no files in directory
	{
		FindClose(hFindFile);
		return 2;
	}

	do
	{
		if ((strcmp(fileData.cFileName, ".") != 0) && (strcmp(fileData.cFileName, "..") != 0))
		{
			if ((fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				searchDir(fileData.cFileName, res);
				SetCurrentDirectory("..");
			}
			else
			{
				printf("%s\n", fileData.cFileName);
				if (fileData.nFileSizeHigh != 0)
					res->numberOfLageFiles++;
				if (fileData.ftCreationTime.dwHighDateTime < res->t.dwHighDateTime
					|| fileData.ftCreationTime.dwLowDateTime < res->t.dwLowDateTime)
				{
					res->t.dwHighDateTime = fileData.ftCreationTime.dwHighDateTime;
					res->t.dwLowDateTime = fileData.ftCreationTime.dwLowDateTime;
				}
			}
		}
	} while (FindNextFile(hFindFile, &fileData));

	FindClose(hFindFile);
	return 0;
}