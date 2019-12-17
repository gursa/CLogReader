#include <clogreader.h>

using namespace std;

int main(int argc, char *argv[])
{	
    if(argc < 3) {
        fprintf(stderr,
                "[ERROR] Invalid number of parameters. You must use the following format:\n\t%s \"%s\" \"%s\"\n",
                argv[0], "C:\\Logs\\log.txt", "order*closed");
        return 1;
    }

    DWORD startTime = GetTickCount();
    CLogReader cl;
    if(!cl.Open(argv[1])) {
        fprintf(stderr, "[ERROR] CLogReader::Open(): failed\n");
        return 1;
    }
    fprintf(stdout, "CLogReader::Open(): success\n");

    if(!cl.SetFilter(argv[2])) {
        fprintf(stderr, "[ERROR] CLogReader::SetFilter(): failed\n");
        return 1;
    }
    fprintf(stdout, "CLogReader::SetFilter(): success\n");

    DWORD matchCount = 0;
    while(1) {
        char buf[4096];
        if(false == cl.GetNextLine(buf, sizeof (buf)))
            break;

        fprintf(stdout, "\n[%06lu]:\t%s\n", matchCount, buf);
        matchCount++;
    }
    cl.Close();

	fprintf(stdout, "\nFile processing is complete! Duration: %lu ms\n", GetTickCount()-startTime);

    system("pause");
    return 0;
}
