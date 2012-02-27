/*******************************************************************************************/
/* Copyright 1999, 2000,2004, Washington University, Mallinckrodt Institute of Radiology.       */
/* All Rights Reserved.                                                                    */
/* This software may not be reproduced, copied, or distributed without written             */
/* permission of Washington University. For further information contact A. Z. Snyder */
/*******************************************************************************************/
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>

#include <string>
using namespace std;

#include <unistd.h>		/* R_OK and W_OK */
#include <time.h>

#include <stdio.h>

class RecFile { //A Singleton Class. Implementation not multi-thread safe
	private:
		//Constructor
		RecFile(){};

		//Member Variables
		static string	 mstr_OutDir;
		static string	 mstr_RecFileName;
		static stringstream recfileData;
		static RecFile* mp_RecFile;
		static bool optedFor;
		void cleanUP(void);
		void error(string str_msg, int code);

	public:

		//Helper methods
		static RecFile* getInstance();
		static  void setOutDir(string outDir);
		static  void setRecFileName(string recFileName);
		static bool getOptedFor();
		static  void setOptedFor(bool option);
		static string  getCompleteFileLocation();

		static bool isNull();


		static void append(string info);

		int startrec(int argc, char *argv[], string rcsid);
		int endrec(void);
		void	get_time_usr (char *string);
		static void Destroy(void);
		void toString();
};

