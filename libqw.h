#include <string>
#include <map>

using namespace std;

typedef map<string,string> strmap;

int AddToLocalQueue(const char* queuefile, strmap& data, int cmd);
