
#include <iostream>
#include <string>
#include<vector>
using namespace std;

class StringUtils {
	public:
		static string getDateTime(string inStr);
		static void Tokenize(const string& str,  vector<string>& tokens, const string& delimiters = " ");
};


string StringUtils::getDateTime(string inStr) {
	vector<string>tokens;
	Tokenize(inStr,tokens,".");
	//for (int i=0;i<tokens.size();i++)
	//	cout<<tokens[i]<<endl;
	string DateTime = tokens[9];
	stringstream sout("");
	sout<<DateTime.substr(4,2)<<"/"<<DateTime.substr(6,2)<<"/"<<DateTime.substr(0,4)<<"   "<<DateTime.substr(8,2)<<":"<<DateTime.substr(10,2)<<":"<<DateTime.substr(12,2);

	return sout.str();
};

void StringUtils::Tokenize(const string& str,  vector<string>& tokens, const string& delimiters)
{
    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
};


