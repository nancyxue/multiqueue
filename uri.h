#ifndef URI_H_
#define URI_H_

#ifdef __cplusplus
extern "C" 
{
#endif

int URLDecode(char* src);
int URLEncode(const char* src, int nSrcLen, char* des);

#ifdef __cplusplus
}
#endif

#endif //URI_H_
