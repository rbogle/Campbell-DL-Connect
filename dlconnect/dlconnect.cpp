// dlconnect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "simplepb.h"
#include <iostream>
#include <fstream>
#include <boost\program_options.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\log\trivial.hpp>
#include <boost\log\expressions.hpp>
#include <boost\log\core.hpp>
#include <boost\log\utility\setup\file.hpp>
#include <boost\log\utility\setup\common_attributes.hpp>
#include <libconfig.hh>

using namespace std;
using namespace libconfig;
using namespace boost;
namespace po=boost::program_options;

void readConfig(Config* c, string* fn){
	try
	{
		c->readFile(fn->c_str());
		cout << "Read in configuration from: " << fn->c_str() << endl;
		BOOST_LOG_TRIVIAL(info) << "Read in configuration from: " << fn->c_str();
	}
	catch (const FileIOException &fioex)
	{
		BOOST_LOG_TRIVIAL(error) << "I/O error while reading file: " << fn;
		exit(1);
	}
	catch (const ParseException &pex)
	{
		BOOST_LOG_TRIVIAL(error) << "Parse error at " << pex.getFile() << ":" << pex.getLine()
			<< " - " << pex.getError();
		exit(1);
	}

}

void writeConfig(Config* c, string* fn, int& lastrec){
	
	try{
		Setting &s = c->lookup("lastrecord");
		s = lastrec;
	}
	catch (const SettingNotFoundException &snfex){
		Setting &r = c->getRoot();
		Setting &s = r.add("lastrecord", Setting::TypeInt);
		s = lastrec;
	}
	
	try
	{
		c->writeFile(fn->c_str());
		cout << "Configuration successfully updated to: " << fn->c_str() << endl;
		BOOST_LOG_TRIVIAL(info) << "Configuration successfully updated to: " << fn->c_str();
	}
	catch (const FileIOException &fioex)
	{
		BOOST_LOG_TRIVIAL(error) << "I/O error while writing file: " << fn << endl;
	}
}

void parseConfig(Config* c, int& port,int& baud, int& type, int& addr, int& table, int& recno, string& of){

	c->lookupValue("port", port);
	c->lookupValue("baud", baud);
	c->lookupValue("type", type);
	c->lookupValue("table", table);
	c->lookupValue("pakaddress", addr);
	c->lookupValue("lastrecord", recno);
	if (recno > 0) recno++;
	c->lookupValue("datafile", of);
	
}

int getData(string& datafile, int& pakaddr, int& type, int& table, int& recno){
	int more = 1;
	char* data="";
	int len;
	string dstr;
	//save cout buffer
	streambuf *coutbuf = cout.rdbuf();
	ofstream outfile;
	//we swap buffers on outstreams if we get a file name
	//this way we use cout for file output or stdout.
	if (datafile != ""){
		outfile.open(datafile, ios::out | ios::app);
		if (!outfile.is_open()){
			BOOST_LOG_TRIVIAL(error) << "Problem opening datafile: " << datafile;
			return 0;
		}
		//set cout's buffer to datafile
		cout.rdbuf(outfile.rdbuf());
	}
	while (more == 1){
		more = GetCommaData(pakaddr, type, table, recno, &data, &len);
		dstr = data;
		//nuke the line feeds and tabs
		dstr.erase(dstr.find_last_not_of("\n\r\t") + 1);
		cout << dstr << endl;
	}

	//reset buffer to cout and close file
	if (datafile != ""){
		cout.rdbuf(coutbuf);
		cout << "Data written to file: " << datafile << endl;
		BOOST_LOG_TRIVIAL(info) << "Data written to file : " << datafile;
		outfile.close();
	}

	//return last record number read
	vector <string> cols;
	split(cols, dstr, is_any_of(","));
	return stoi(cols[1]);
	
}

void initLogging(){
	log::add_common_attributes();
	log::add_file_log
		(
			log::keywords::file_name = "sample_%N.log",
			log::keywords::rotation_size = 10 * 1024 * 1024,
			log::keywords::format = "[%TimeStamp%]: %Message%"
		);

	log::core::get()->set_filter
		(
			log::trivial::severity >= log::trivial::info
		);
}

int main(int argc, char* argv[])
{
	int port, type, recno, baud, table, pakaddr;
	//string[] errcodes = { "Sucess", "Comm timeout", "Com port closed", };
	string cfgfile, outfile;
	Config cfg;

	initLogging();

	//setup cmdline options
	po::options_description desc("Allowed options");
	desc.add_options()
		("help,?", "produce help message\n")
		("address,a", po::value<int>(&pakaddr)->default_value(1), "PakBus address for datalogger. (Default=1)\n")
		("baud,b", po::value<int>(&baud)->default_value(9600), "COM port speed. (Default: 9600)\n")
		("config,c", po::value<string>(&cfgfile), "path to config file. Specifying a config file will override other options on cmdline\n")
		("dltype,d", po::value<int>(&type)->default_value(1), "Datalogger model: (Default: 1) \ncr200: 1\ncr10x/cr23x/cr510pb: 2\ncr1000: 3\ncr3000: 4\ncr800/cr850: 5\n")
		("info,i", "report datalogger info and exit\n")
		("output,o", po::value<string>(&outfile), "output file for data (Default: stdout)\n")
		("port,p", po::value<int>(&port)->default_value(1), "COM port number. (Default: 1)\n")
		("record,r", po::value<int>(&recno)->default_value(0), "record number to start data download (Default: 0)\n-1 will produce only the latest record\n")
		("sync,s", "will sync datalogger clock with system clock\n")
		("table,t", po::value<int>(&table)->default_value(2), "Table number to retrieve data from. (Default=2, usually first user table)\n")
		;
	//process options
	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	//user just wants help
	if (vm.count("help")) {
		cout << "Dlconnect will connect to a single campbell datalogger connected to a specified\n"
			 << "serial port. Multiple commandline options are available to specify the\n"
			 << "parameters of the connection and the action to be taken. Dlconnect will sync\n"
			 << "the clock time between logger and host, retrieve logger info, or collect data\n"
			 << "from specific table. If you specify a config file dlconnect will keep track of\n"
			 << "the last record downloaded and begin with that record on next collection.\n"
			 << "Valid paramaters in the configfile are: port, baud, type, table, pakaddress,\n"
			 << "lastrecord, and datafile. If you do not specify an output or datafile then all\n"
			 << "table info will be output on the commandline.\n\n";
		cout << desc << endl;
		return 0;
	}

	//config file was set so read that in. This overrides any other cmdline specified options
	if (vm.count("config")){
		readConfig(&cfg, &cfgfile);
		parseConfig(&cfg, port,baud, type, pakaddr, table, recno, outfile);
	}
	
	//try connecting to datalogger
	int rval = OpenPort(port, baud);
	if (rval<0){
		BOOST_LOG_TRIVIAL(error) << "Problem opening port: " << port << " to datalogger with baud: " << baud;
		exit(1);
	}
	cout << "Datalogger Opened..." << endl;

	//user wants clock to sync
	if (vm.count("sync")){
		char* data;
		int datlen;
		rval = SetClock(pakaddr,type,&data,&datlen);
		if (rval<0){
			BOOST_LOG_TRIVIAL(error) << "Datalogger clock could not be syncronized." ;
		}
		else{
			cout << "Datalogger clock syncronized:\n" << data << endl;
			BOOST_LOG_TRIVIAL(info) << "Datalogger clock syncronized:\n" << data;
		}
	}

	//user only wants info not data
	if (vm.count("info")){
		char* data; int datalen;
		rval = GetStatus(pakaddr, type, &data, &datalen);
		if (rval<0){
			BOOST_LOG_TRIVIAL(error) << "Could not get status of datalogger.";
			ClosePort();
			exit(1);
		}
		cout << "Datalogger Status:" << endl;
		cout << "Port: " << port << endl;
		cout << "Baud: " << baud << endl;
		cout << "Type: " << type << endl;
		cout << "PakBus Addr: " << pakaddr << endl;
		cout << data << endl;
		rval = GetTableNames(pakaddr, type, &data, &datalen);
		cout << "Table Names: \n" << data << endl;
		BOOST_LOG_TRIVIAL(info) << "Datalogger info retrieved, exiting.";
		ClosePort();
		exit(0);
	}

	//now collect data and write to outfile or stdout 
	cout << "Collecting data from datalogger table: " << table << endl;
	int lastrec = getData(outfile, pakaddr, type, table, recno);
	
	//if we used a config file write out the lastrec to it otherwise report it
	if (vm.count("config")){
		writeConfig(&cfg, &cfgfile, lastrec);
	}
	cout << "Last record read was: " << lastrec << endl;
	BOOST_LOG_TRIVIAL(info) << "Last record read was: " << lastrec;
	//close datalogger
	cout << "Closing datalogger...." << endl;
	rval = ClosePort();

	return 0;
}

