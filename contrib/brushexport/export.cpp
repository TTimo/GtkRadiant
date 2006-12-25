#include "export.h"
#include "debugging/debugging.h"
#include "ibrush.h"
#include "iscenegraph.h"
#include "iselection.h"
#include "stream/stringstream.h"
#include "stream/textfilestream.h"

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
	ExportData(const std::set<std::string>& ignorelist, collapsemode mode);
	virtual ~ExportData(void);
	
	virtual void BeginBrush(Brush& b);
	virtual void AddBrushFace(Face& f);
	virtual void EndBrush(void);
	
	virtual bool WriteToFile(const std::string& path) const = 0;
	
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

ExportData::ExportData(const std::set<std::string>& _ignorelist, collapsemode _mode)
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
	
	// faces mit materials auf der ignoreliste ignorieren
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
		
	globalOutputStream() << "Last: " << last_slash << " " << "lenght: " << (const unsigned int)tmp.length() << "Name: " << name.c_str() << "\n";
}

/*
	Exporter writing facedata as wavefront object
*/
class ExportDataAsWavefront : public ExportData
{
public:
	ExportDataAsWavefront(const std::set<std::string>& _ignorelist, collapsemode _mode)
		: ExportData(_ignorelist, _mode)
	{
	}
	
	bool WriteToFile(const std::string& path) const;
};

bool ExportDataAsWavefront::WriteToFile(const std::string& path) const
{
	TextFileOutputStream out(path.c_str());
	
	if(out.failed())
	{
		globalErrorStream() << "Unable to open file\n";
		return false;
	}
		
	out << "# Wavefront Objectfile exported with radiants brushexport plugin 2.0 by Thomas 'namespace' Nitschke, spam@codecreator.net\n\n";
	
	unsigned int vertex_count = 0;

	const std::list<ExportData::group>::const_iterator gend(groups.end());
	for(std::list<ExportData::group>::const_iterator git(groups.begin()); git != gend; ++git)
	{
		const std::list<const Face*>::const_iterator end(git->faces.end());
		
		// submesh starts here
		out << "\ng " << git->name.c_str() << "\n";
		
		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// vertices
			for(size_t i = 0; i < w.numpoints; ++i)
      			out << "v " << w[i].vertex.x() << " " << w[i].vertex.y() << " " << w[i].vertex.z() << "\n";
		}
		out << "\n";	
		
		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// texcoords
			for(size_t i = 0; i < w.numpoints; ++i)
      			out << "vt " << w[i].texcoord.x() << " " << w[i].texcoord.y() << "\n";
		}
		
		for(std::list<const Face*>::const_iterator it(git->faces.begin()); it != end; ++it)
		{
			const Winding& w((*it)->getWinding());
			
			// faces
			out << "\nf";
			for(size_t i = 0; i < w.numpoints; ++i, ++vertex_count)
			{
				out << " " << vertex_count+1 << "/" << vertex_count+1;
			}
		}
		out << "\n";
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
 
bool ExportSelection(const std::set<std::string>& ignorelist, collapsemode m, const std::string& path)
{
	ExportDataAsWavefront exporter(ignorelist, m);
	
	ForEachSelected vis(exporter);
	GlobalSelectionSystem().foreachSelected(vis);
	
	return exporter.WriteToFile(path);
}
