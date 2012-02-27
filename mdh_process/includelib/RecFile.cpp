/*******************************************************************************************/
/* Copyright 1999, 2000, 2004, Washington University, Mallinckrodt Institute of Radiology.       */
/* All Rights Reserved.                                                                    */
/* This software may not be reproduced, copied, or distributed without written             */
/* permission of Washington University. For further information contact A. Z. Snyder.      */
/*******************************************************************************************/
/*
 * Revision 1.1  2004/04/22  04:04:10  mohana
 *Based on Code by Avi Snyder
 * Initial revision
 **/

#include "RecFile.h"

#define  MAXL	256

static char rcsid[] = "TO DO: $Id: RecFile.cpp 2 2006-05-19 18:31:02Z rudolph $";


RecFile* RecFile::mp_RecFile = NULL;
string RecFile::mstr_OutDir="";
string RecFile::mstr_RecFileName="";
stringstream RecFile::recfileData("");
bool RecFile::optedFor=false;

RecFile* RecFile::getInstance() {
	if (!optedFor) {
		mp_RecFile = NULL;
	}else if (mp_RecFile==NULL){
		mp_RecFile = new RecFile();
	}
	return mp_RecFile;
}

void RecFile::toString() {
	cout<<"RecFile is alive...\n";
}

void RecFile::error (
			string          astr_msg        /*= "Some error has occured"    */ ,
			int             code           	 /*= -1                          */ )
	{
		//
		// ARGS
		//  atr_msg                 in              message to dump to stderr
		//  code                    in              error code
		//
		// DESC
		//  Print error related information. This routine throws an exception
		//  to the class itself, allowing for coarse grained, but simple
		//  error flagging.
		//

		cerr << "\nFatal error encountered.\n";
		cerr << "\t Image object \n";
		cerr << "\t" << astr_msg << "\n";
		cerr << "Throwing an exception to (this) with code " << code << "\n\n";
		throw(this);
	}

void RecFile::setOutDir(string outDir) {
	mstr_OutDir = outDir;
}

 void RecFile::setRecFileName(string recFileName) {
	mstr_RecFileName = recFileName;
}

bool RecFile::getOptedFor() {
	return optedFor;
}

void RecFile::setOptedFor(bool option) {
	optedFor=option;
}

string RecFile::getCompleteFileLocation() {
	return (mstr_OutDir + "/" + mstr_RecFileName+".rec");
}

bool RecFile::isNull() {
	if (getInstance()==NULL) return true;
	else return false;
}

void RecFile::append(string info) {
	recfileData <<"\n"<< info;
}


int RecFile::startrec (int argc, char *argv[], string rcsid) {

	int 		k;
	char		str[MAXL];
	recfileData<<"rec  "<<mstr_RecFileName<<"  ";
	get_time_usr(str);
	recfileData<<str<<"\n";

	for (k = 0; k < argc; k++)
		recfileData<<argv[k]<<"  ";
	recfileData <<"\n"<<rcsid<<"\n";
	return 0;
}

int RecFile::endrec (void) {
	char 		str[MAXL];
	string 	str_recFile = getCompleteFileLocation();

	ofstream recFile(str_recFile.c_str());
	if (!recFile)
		error("While ending the Rec File::"+str_recFile+"...Check that the create permissions exist", -1);

	recFile<<recfileData.str()<<"\n";
	get_time_usr (str);
	recFile<<"endrec "<<str<<"\n";

	recFile.close();
	cleanUP();

	return 0;
}

void RecFile::cleanUP(void) {
	mstr_RecFileName="";
	recfileData.str("");

}

void RecFile::Destroy(void) {
	if (!isNull())
		delete mp_RecFile;
}

void	RecFile::get_time_usr (char *string) {
	time_t		time_sec;

	time (&time_sec);
	strcpy (string, ctime (&time_sec));
	string [24] = '\0';
	strcat (string, "  ");
	cuserid (string + 26);
}


