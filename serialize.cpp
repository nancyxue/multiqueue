#include <istream>
#include <map>
#include "serialize.h"

using namespace std;

typedef map<string,string>::const_iterator const_strmapit;

int unserialize(istream& data,strmap& datamap)
{
        char cc[2];
	int array_count = 0;
	int array_pos;
	int len;
	char varname[64];
	
	datamap.clear();
	
	while(!data.eof())
	{
		data.read(cc,2);
		switch(*(short*)cc)
		{
		case 0x3a61:
			data >> array_count;
			array_pos = 0;
			data.seekg(2,ios_base::cur);
			break;
		case 0x3a73:
			array_pos++;
			data >> len;
			data.seekg(2,ios_base::cur);
			if (array_pos%2)
			{
				data.read(varname,len);
				varname[len] = 0;
			}
			else
			{
				char* pval = new char[len+1];
				data.read(pval,len);
				pval[len] = 0;
				datamap[varname].assign(pval,len);
				delete pval;
			}
			data.seekg(2,ios_base::cur);
			break;
		case 0x3a64:
		case 0x3a69:
			array_pos++;
			if (array_pos%2)
			{
				data.getline(varname,64,';');
			}
			else
			{
				getline(data,datamap[varname],';');
			}
			break;
		case 0x3b4e:
			array_pos++;
			if (array_pos%2)
				return -1;
			else
				datamap[varname] = "";
			break;
		default:
			return -1;
		}

		if (array_count > 0 && array_pos > 0)
		{
			if (array_pos%2 == 0)
				array_count--;
			if (array_count == 0)
				data.seekg(1,ios_base::cur);
		}
	}

	return 0;
}

int unserialize(const char* data, int size, strmap& datamap)
{
	const char* pstart = data;
	const char* pend, *pdataend;
	int array_count = 0;
	int array_pos;
	int len;
	char varname[64];

	datamap.clear();
	
	pdataend=data+size;
	while(*pstart && pstart<pdataend)
	{
		switch(*(short* )pstart)
		{
		case 0x3a61: //"a:"
			pstart += 2;
			array_count = atoi(pstart);
			array_pos = 0;
			pstart = strchr(pstart,'{');
			pstart++;
		break;
		case 0x3a73: //"s:"
			array_pos++;
			pstart += 2;
			len = atoi(pstart);
			pstart = strchr(pstart,'"');
			pstart++;
			if (array_pos%2) //name
			{
				memcpy(varname,pstart,len);
				varname[len] = 0;
			}
			else //value
			{
				datamap[varname].assign(pstart,len);
			}
			pstart += len+2;
		break;
		case 0x3a64: //"d:"
		case 0x3a69: //"i:"
			array_pos++;
			pstart += 2;
			pend = strchr(pstart,';');
			if (pend == NULL)
				return -1;
			len = pend-pstart;
			if (array_pos%2) //name
			{
				memcpy(varname,pstart,len);
				varname[len] = 0;
			}
			else //value
			{
				datamap[varname].assign(pstart,len);
			}
			pstart = pend+1;
		break;
		case 0x3b4e:
			array_pos++;
			pstart += 2;
			if (array_pos%2)
				return -1;
			else
				datamap[varname] = "";
			break;
		default:
			return -1;
		}
		if (array_count > 0 && array_pos > 0)
		{
			if (array_pos%2 == 0)
				array_count--;
			if (array_count == 0)
				pstart++;
		}
	}
	return 0;
}

int serialize(const strmap& datamap, SerializeResult& Result)
{
	int size = 8;
	for(const_strmapit it = datamap.begin();it != datamap.end(); it++)
	{
		size += it->first.size();
		size += it->second.size();
		size += 16;
	}
	char* result = Result.GetBuff(size+1024);	//TODO: +1024是为了解决内存越界的问题;内存越界还没有来得及查
	sprintf(result,"a:%d:{",datamap.size());
	size = strlen(result);
	for(const_strmapit it = datamap.begin(); it != datamap.end(); it++)
	{
		sprintf(result + size, "s:%d:\"", it->first.size());
		size+=strlen(result + size);
		it->first.copy(result+size, it->first.size());
		size += it->first.size();
		strcpy(result + size,"\";");
		size += 2;
		sprintf(result + size, "s:%d:\"", it->second.size());
		size += strlen(result + size);
		it->second.copy(result + size, it->second.size());
		size += it->second.size();
		strcpy(result + size, "\";");
		size += 2;
	}
	strcpy(result + size, "}");
	return size + 1;
}

