#pragma once

#include <map>
#include <cstdio>
#include <vector>

void save_vector(FILE* fout, const std::vector<int> &v);
void load_vector(FILE* fin, std::vector<int> &v);
void save_vector_vector(FILE* fout, const std::vector<std::vector<int> > &v);
void load_vector_vector(FILE* fin, std::vector<std::vector<int> > &v);
void save_vector_pair(FILE* fout, const std::vector<std::pair<int,int> > &v);
void load_vector_pair(FILE* fin, std::vector<std::pair<int,int> > &v);
void save_map_int_pair(FILE* fout, std::map<int,std::pair<int,int> > &h);
void load_map_int_pair(FILE* fin, std::map<int,std::pair<int,int> > &h);
void save_map_int_int(FILE* fout, std::map<int,int> &h);
void load_map_int_int(FILE* fin, std::map<int,int> &h);