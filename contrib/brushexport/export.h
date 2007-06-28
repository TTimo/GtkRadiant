#ifndef EXPORT_H
#define EXPORT_H
#include <set>
#include <string>

enum collapsemode
{
	COLLAPSE_ALL,
	COLLAPSE_BY_MATERIAL,
	COLLAPSE_NONE
};

bool ExportSelection(const std::set<std::string>& ignorelist, collapsemode m, bool exmat, const std::string& path, bool limitMatNames, bool objects);

#endif 
