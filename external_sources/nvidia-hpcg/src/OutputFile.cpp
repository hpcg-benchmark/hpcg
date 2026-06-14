
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

/*
 * SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>

#include "OutputFile.hpp"

using std::string;
using std::stringstream;
using std::list;
using std::ofstream;

extern int use_output_file;

OutputFile::OutputFile(const string& name_arg, const string& version_arg)
    : name(name_arg)
    , version(version_arg)
    , eol("\n")
    , keySeparator("::")
{
}

OutputFile::OutputFile(void)
    : eol("\n")
    , keySeparator("::")
{
}

OutputFile::~OutputFile()
{
    for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it)
    {
        delete *it;
    }
}

void OutputFile::add(const string& key_arg, const string& value_arg)
{
    descendants.push_back(allocKeyVal(key_arg, value_arg));
}

void OutputFile::add(const string& key_arg, double value_arg)
{
    stringstream ss;
    ss << value_arg;
    descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void OutputFile::add(const string& key_arg, int value_arg)
{
    stringstream ss;
    ss << value_arg;
    descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#ifndef HPCG_NO_LONG_LONG

void OutputFile::add(const string& key_arg, long long value_arg)
{
    stringstream ss;
    ss << value_arg;
    descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

#endif

void OutputFile::add(const string& key_arg, size_t value_arg)
{
    stringstream ss;
    ss << value_arg;
    descendants.push_back(allocKeyVal(key_arg, ss.str()));
}

void OutputFile::setKeyValue(const string& key_arg, const string& value_arg)
{
    key = key_arg;
    value = value_arg;
}

OutputFile* OutputFile::get(const string& key_arg)
{
    for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it)
    {
        if ((*it)->key == key_arg)
            return *it;
    }

    return 0;
}

string OutputFile::generateRecursive(string prefix)
{
    string result = "";

    result += prefix + key + "=" + value + eol;

    for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it)
    {
        result += (*it)->generateRecursive(prefix + key + keySeparator);
    }

    return result;
}

string OutputFile::generate(void)
{
    string result = name + "\nversion=" + version + eol;

    for (list<OutputFile*>::iterator it = descendants.begin(); it != descendants.end(); ++it)
    {
        result += (*it)->generateRecursive("");
    }

    time_t rawtime;
    time(&rawtime);
    tm* ptm = localtime(&rawtime);
    char sdate[64];
    // use tm_mon+1 because tm_mon is 0 .. 11 instead of 1 .. 12
    sprintf(sdate, "%04d-%02d-%02d_%02d-%02d-%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,
        ptm->tm_min, ptm->tm_sec);

    string filename = name + "_" + version + "_";
    filename += string(sdate) + ".txt";

    if (use_output_file)
    {
        ofstream myfile(filename.c_str());
        myfile << result;
        myfile.close();
    }
    else
    {
        std::cout << result << std::flush;
    }

    return result;
}

OutputFile* OutputFile::allocKeyVal(const std::string& key_arg, const std::string& value_arg)
{
    OutputFile* of = new OutputFile();
    of->setKeyValue(key_arg, value_arg);
    return of;
}
