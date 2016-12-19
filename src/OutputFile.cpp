
//@HEADER
// ***************************************************
//
// HPCG: High Performance Conjugate Gradient Benchmark
//
// Contact:
// Michael A. Heroux ( maherou@sandia.gov)
// Jack Dongarra     (dongarra@eecs.utk.edu)
// Piotr Luszczek    (luszczek@eecs.utk.edu)
//
// ***************************************************
//@HEADER


#include <fstream>
#include <list>
#include <sstream>
#include <string>

#include "OutputFile.hpp"

using std::string;
using std::stringstream;
using std::list;
using std::ofstream;

OutputFile::OutputFile(const string & name_arg, const string & version_arg)
  : name(name_arg), version(version_arg), eol("\n"), keySeparator("::") {}

OutputFile::OutputFile(void) : eol("\n"), keySeparator("::") {}

OutputFile::~OutputFile() {
  for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
    delete *it;
  }
}

void
OutputFile::add(const string & key_arg, const string & value_arg) {
  descendants.push_back(allocKeyVal(key_arg, value_arg));
}

void
OutputFile::add(const string & key_arg, double value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void
OutputFile::add(const string & key_arg, int value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#ifndef HPCG_NO_LONG_LONG

void
OutputFile::add(const string & key_arg, long long value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#endif

void
OutputFile::add(const string & key_arg, size_t value_arg) {
  stringstream ss;
  ss << value_arg;
  descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void
OutputFile::setKeyValue(const string & key_arg, const string & value_arg) {
  key = key_arg;
  value = value_arg;
}

OutputFile *
OutputFile::get(const string & key_arg) {
  for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
    if ((*it)->key == key_arg)
      return *it;
  }

  return 0;
}

string
OutputFile::generateRecursive(string prefix) {
  string result = "";

  result += prefix + key + "=" + value + eol;

  for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
    result += (*it)->generateRecursive(prefix + key + keySeparator);
  }

  return result;
}

string
OutputFile::generate(void) {
  string result = name + "\nversion=" + version + eol;

  for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it) {
    result += (*it)->generateRecursive("");
  }

  time_t rawtime;
  time(&rawtime);
  tm * ptm = localtime(&rawtime);
  char sdate[25];
  //use tm_mon+1 because tm_mon is 0 .. 11 instead of 1 .. 12
  sprintf (sdate,"%04d-%02d-%02d_%02d-%02d-%02d",ptm->tm_year + 1900, ptm->tm_mon+1,
        ptm->tm_mday, ptm->tm_hour, ptm->tm_min,ptm->tm_sec);

  string filename = name + "_" + version + "_";
  filename += string(sdate) + ".txt";

  ofstream myfile(filename.c_str());
  myfile << result;
  myfile.close();

  return result;
}

OutputFile * OutputFile::allocKeyVal(const std::string & key_arg, const std::string & value_arg) {
  OutputFile * of = new OutputFile();
  of->setKeyValue(key_arg, value_arg);
  return of;
}
