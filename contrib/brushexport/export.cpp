#include "export.h"
#include "debugging/debugging.h"
#include "ibrush.h"
#include "iscenegraph.h"
#include "iselection.h"
#include "stream/stringstream.h"
#include "stream/textfilestream.h"

#include <map>

// this is very evil, but right now there is no better way
#include "../../radiant/brush.h"

/*
	Abstract baseclass for modelexporters
	the class collects all the data which then gets
	exported through the WriteToFile method.
*/
class ExportData
{
public:
	ExportData(const std::set<std::string>& ignorelist, collapsemode mode, bool limNames, bool objs);
	virtual ~ExportData(void);
	
	virtual void BeginBrush(Brush& b);
	virtual void AddBrushFace(Face& f);
	virtual void EndBrush(void);
	
	virtual bool WriteToFile(const std::string& path, collapsemode mode) const = 0;
	
protected:

	// a group of faces
	class group
	{
	public:
		std::string name;
		std::list<const Face*> faces;
	};
	
	std::list<group> groups;
	
private:

	// "textures/common/caulk" -> "caulk"
	void GetShaderNameFromShaderPath(const char* path, std::string& name);

	group* current;	
	collapsemode mode;
	const std::set<std::string>& ignorelist;
};

ExportData::ExportData(const std::set<std::string>& _ignorelist, collapsemode _mode, bool _limNames, bool _objs)
	:	mode(_mode),
		ignorelist(_ignorelist)
{
	current = 0;
	
	// in this mode, we need just one group
	if(mode == COLLAPSE_ALL)
	{
		groups.push_back(group());
		current = &groups.back();
		current->name = "all";
	}
}

ExportData::~ExportData(void)
{
	
}

void ExportData::BeginBrush(Brush& b)
{
	// create a new group for each brush
	if(mode == COLLAPSE_NONE)
	{
		groups.push_back(group());
		current = &groups.back();
		
		StringOutputStream str(256);
		str << "Brush" << (const unsigned int)groups.size();
		current->name = str.c_str();
	}
}

void ExportData::EndBrush(void)
{
	// all faces of this brush were on the ignorelist, discard the emptygroup
	if(mode == COLLAPSE_NONE)
	{
		ASSERT_NOTNULL(current);
		if(current->faces.empty())
		{
			groups.pop_back();
			current = 0;
		}
	}
}

void ExportData::AddBrushFace(Face& f)
{
	std::string shadername;
	GetShaderNameFromShaderPath(f.GetShader(), shadername);
	
	// ignore faces from ignore list
	if(ignorelist.find(shadername) != ignorelist.end())
		return;
		
	if(mode == COLLAPSE_BY_MATERIAL)
	{
		// find a group for this material
		current = 0;
		const std::list<group>::iterator end(groups.end());
		for(std::list<group>::iterator it(groups.begin()); it != end; ++it)
		{
			if(it->name == shadername)
				current = &(*it);
		}
		
		// no group found, create one
		if(!current)
		{
			groups.push_back(group());
			current = &groups.back();
			current->name = shadername;
		}
	}
	
	ASSERT_NOTNULL(current);
	
	// add face to current group
	current->faces.push_back(&f);
	
#ifdef _DEBUG
	globalOutputStream() << "Added Face to group " << current->name.c_str() << "\n";
#endif
}

void ExportData::GetShaderNameFromShaderPath(const char* path, std::string& name)
{
	std::string tmp(path);

	size_t last_slash = tmp.find_last_of("/");
	
	if(last_slash != std::string::npos && last_slash == (tmp.length() - 1))
		name = path;
	else
		name = tmp.substr(last_slash + 1, tmp.length() - last_slash);

#ifdef _DEBUG
	globalOutputStream() << "Last: " << last_slash << " " << "length: " << (const unsigned int)tmp.length() << "Name: " << name.c_str() << "\n";
#endif
}

/*
	Exporter writing facedata as wavefront object
*/
class ExportDataAsWavefront : public ExportData
{
private:
	bool expmat;
	bool limNames;
	bool objs;

public:
	ExportDataAsWavefront(const std::set<std::string>& _ignorelist, collapsemode _mode, bool _expmat, bool _limNames, bool _objs)
		: ExportData(_ignorelist, _mode, _limNames, _objs)
	{
		expmat = _expmat;
		limNames = _limNames;
		objs = _objs;
	}
	
	bool WriteToFile(const std::string& path, collapsemode mode) const;
};

bool ExportDataAsWavefront::WriteToFile(const std::string& path, collapsemode mode) const
{
	std::string objFile = path.substr(0, path.length() -4) + ".obj";
	std::string mtlFile = path.substr(0, path.length() -4) + ".mtl";

	std::set<std::string> materials;

	TextFileOutputStream out(objFile.c_str());
	
	if(out.failed())
	{
		globalErrorStream() << "Unable to open file\n";
		return false;
	}
		
	out << "# Wavefront Objectfile exported with radiants brushexport plugin 3.0 by Thomas 'namespace' Nitschke, spam@codecreator.net\n\n";

	if(expmat)
	{
		size_t last = mtlFile.find_last_of("//");
		std::string mtllib = mtlFile.substr(last + 1, mtlFile.size() - last).c_str();
		out << "mtllib " << mtllib.c_str() << "\n";
	}
	
	unsigned int vertex_count = 0;

	const std::list<ExportData::group>::const_iterator gend(groups.end());
	for(std::list<ExportData::group>::const_iterator git(groups.begin()); git != gend; ++git)
	{
		typedef std::multimap<std::string, std::string> bm;
		bm brushMaterials;
		typedef std::pair<std::string, std::string> String_Pair;

		const std::list<const Face*>::const_iterator end(git->faces.end());
		
		// submesh starts here
		if(objs)
		{
			out << "\no ";
		} else {
			out << "\ng ";
		}
		out << git->name.c_str() << "\n";

		// material
		if(expmat && mode == COLLAPSE_ALL)
		{
			out << "usemtl material" << "\n\n";
			materials.insert("material");
		}

		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// vertices
			for(size_t i = 0; i < w.numpoints; ++i)
					out << "v " << FloatFormat(w[i].vertex.x(), 1, 6) << " " << FloatFormat(w[i].vertex.z(), 1, 6) << " " << FloatFormat(w[i].vertex.y(), 1, 6) << "\n";
		}
		out << "\n";	
		
		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// texcoords
			for(size_t i = 0; i < w.numpoints; ++i)
      				out << "vt " << FloatFormat(w[i].texcoord.x(), 1, 6) << " " << FloatFormat(w[i].texcoord.y(), 1, 6) << "\n";
		}
		
		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// faces
			StringOutputStream faceLine(256);
			faceLine << "\nf";
			for(size_t i = 0; i < w.numpoints; ++i, ++vertex_count)
			{
				faceLine << " " << vertex_count+1 << "/" << vertex_count+1;
			}

			if(mode != COLLAPSE_ALL)
			{
				materials.insert((*it)->getShader().getShader());
				brushMaterials.insert(String_Pair((*it)->getShader().getShader(), faceLine.c_str()));
			} else {
				out << faceLine.c_str();
			}
		}

		if(mode != COLLAPSE_ALL)
		{
			std::string lastMat;
			std::string mat;
			std::string faces;

			for(bm::iterator iter = brushMaterials.begin(); iter != brushMaterials.end(); iter++)
			{
				mat = (*iter).first.c_str();
				faces = (*iter).second.c_str();

				if(mat != lastMat)
				{
					if(limNames && mat.size() > 20)
					{
						out << "\nusemtl " << mat.substr(mat.size() - 20, mat.size()).c_str();
					} else {
						out << "\nusemtl " << mat.c_str();
					}
				}

				out << faces.c_str();
				lastMat = mat;
			}
		}

		out << "\n";
	}

	if(expmat)
	{
		TextFileOutputStream outMtl(mtlFile.c_str());
		if(outMtl.failed())
		{
			globalErrorStream() << "Unable to open material file\n";
			return false;
		}

		outMtl << "# Wavefront material file exported with GtkRadiants brushexport plugin.\n";
		outMtl << "# Material Count: " << (const Unsigned)materials.size() << "\n\n";
		for(std::set<std::string>::const_iterator it(materials.begin()); it != materials.end(); ++it)
		{
			if(limNames && it->size() > 20)
			{
				outMtl << "newmtl " << it->substr(it->size() - 20, it->size()).c_str() << "\n";
			} else {
				outMtl << "newmtl " << it->c_str() << "\n";
			}
		}
	}
	
	return true;
}


class ForEachFace : public BrushVisitor
{
public:
	ForEachFace(ExportData& _exporter)
		: exporter(_exporter)
	{}
	
	void visit(Face& face) const
	{
		exporter.AddBrushFace(face);
	}
	
private:
	ExportData& exporter;
};

class ForEachSelected : public SelectionSystem::Visitor
{
public:
	ForEachSelected(ExportData& _exporter)
		: exporter(_exporter)
	{}
	
	void visit(scene::Instance& instance) const
    {
		BrushInstance* bptr = InstanceTypeCast<BrushInstance>::cast(instance);
		if(bptr)
		{
			Brush& brush(bptr->getBrush());
			
			exporter.BeginBrush(brush);
				ForEachFace face_vis(exporter);
				brush.forEachFace(face_vis);
			exporter.EndBrush();
		}
    }
    
private:
	ExportData& exporter;
};
 
bool ExportSelection(const std::set<std::string>& ignorelist, collapsemode m, bool exmat, const std::string& path, bool limNames, bool objs)
{
	ExportDataAsWavefront exporter(ignorelist, m, exmat, limNames, objs);
	
	ForEachSelected vis(exporter);
	GlobalSelectionSystem().foreachSelected(vis);
	
	return exporter.WriteToFile(path, m);
}
