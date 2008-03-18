// Pak.cpp: implementation of the CPak class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pak.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPak::CPak()
{
	m_bModified=FALSE;
	
}

CPak::~CPak()
{
	Close();
}


bool CPak::Open(const char *filename)
{
	Close();
	
	unzFile uf=NULL;

	uf = unzOpen(filename);
	if (uf==NULL)
		return FALSE;

	unz_global_info gi;
	int err = unzGetGlobalInfo (uf,&gi);
	if (err!=UNZ_OK)
	{
		Close();
		return FALSE;
	}

	m_filename=filename;

	// ok, now build the internal list of entries

	unzGoToFirstFile(uf);

	do
	{
		pak_entry pe;

		pe.frompak=TRUE;
		pe.pakname=m_filename;
		pe.rename=FALSE;

		char filename_inzip[256];
		unz_file_info file_info;
		//		uLong ratio=0;
		//		const char *string_method;
		err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		if (err!=UNZ_OK)
		{
			continue;
		}
		
		pe.filename=filename_inzip;
		pe.compressed=file_info.compressed_size;
		pe.uncompressed=file_info.uncompressed_size;

		entries.push_back(pe);

	} while (unzGoToNextFile(uf)==UNZ_OK);
	

	unzClose(uf);
	m_bModified=FALSE;

	return TRUE;
}

bool CPak::Save(const char *path)
{
	if (!m_bModified)
		return FALSE;

	if (path==NULL)
		return FALSE;

	//	unzFile uf=NULL;
	zipFile zf=NULL;
	
	zf=zipOpen(path,0);

	for ( std::list<pak_entry>::const_iterator iEntry = entries.begin() ; iEntry != entries.end() ; iEntry++)
	{
		pak_entry pe=*iEntry;

		if (pe.frompak)
		{
			if (pe.pakname.compare(m_filename)==0)
			{
				// its in this pak
				long size;
				unsigned char *buf=Extract(pe.filename.c_str(),size);

				if (!buf)
					continue;

				zip_fileinfo zipfi;

				std::string name=pe.filename;
				if (pe.rename)
					name=pe.newname;

				zipOpenNewFileInZip(zf,name.c_str(),&zipfi,NULL,0,NULL,0,NULL,Z_DEFLATED,Z_DEFAULT_COMPRESSION);
				zipWriteInFileInZip(zf,buf,size);
				zipCloseFileInZip(zf);

				delete buf;
			}
			else
			{
				// its in another pack

				CPak p;
				if (!p.Open(pe.pakname.c_str()))
					continue;

				long size;
				unsigned char *buf=p.Extract(pe.filename.c_str(),size);

				if (!buf)
					continue;

				zip_fileinfo zipfi;

				std::string name=pe.filename;
				if (pe.rename)
					name=pe.newname;

				zipOpenNewFileInZip(zf,name.c_str(),&zipfi,NULL,0,NULL,0,NULL,Z_DEFLATED,Z_DEFAULT_COMPRESSION);
				zipWriteInFileInZip(zf,buf,size);
				zipCloseFileInZip(zf);

				delete buf;
			}

		}
		else
		{
			// its an external file

			FILE *fp;
			fp=fopen(pe.pathname.c_str(),"rb");

			if (!fp)
				continue;

			fseek(fp,0,SEEK_END);
			long size=ftell(fp);
			fseek(fp,0,0);
			unsigned char *buf=new unsigned char[size];
			fread(buf,sizeof(unsigned char),size,fp);
			fclose(fp);

			if (!buf)
				continue;

			zip_fileinfo zipfi;

			std::string name=pe.filename;
			if (pe.rename)
				name=pe.newname;

			zipOpenNewFileInZip(zf,name.c_str(),&zipfi,NULL,0,NULL,0,NULL,Z_DEFLATED,Z_DEFAULT_COMPRESSION);
			zipWriteInFileInZip(zf,buf,size);
			zipCloseFileInZip(zf);

			delete buf;

		}
	
	}

	zipClose(zf,"Created by Pk3man - http://pk3man.sourceforge.net");

	m_bModified=FALSE;
	return TRUE;
}

void CPak::Close()
{

	m_filename="";
	entries.clear();
	m_bModified=FALSE;

}

bool CPak::IsModified()
{
	return m_bModified;
}

pak_entry CPak::FindPE(const char *path)
{
	pak_entry pe;	

	for ( std::list<pak_entry>::const_iterator iEntry = entries.begin() ; iEntry != entries.end() ; iEntry++)
	{
		pe=*iEntry;

		if (!pe.rename && pe.filename.compare(path)==0)
			return pe;

		if (pe.rename && pe.newname.compare(path)==0)
			return pe;
	}

	pe.compressed=0;
	pe.uncompressed=0;

	return pe;
}

bool CPak::IsEmpty()
{
	return entries.empty();
}

unsigned char *CPak::Extract(const char *path, long &size)
{
	unzFile uf=NULL;

	uf = unzOpen(m_filename.GetBuffer());
	if (uf==NULL)
		return FALSE;

	unz_global_info gi;
	int err = unzGetGlobalInfo (uf,&gi);
	if (err!=UNZ_OK)
	{
		Close();
		return FALSE;
	}
	
	unzGoToFirstFile(uf);

	do
	{
		char filename_inzip[256];
		unz_file_info file_info;
				
		err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
		
		if (err!=UNZ_OK)
		{
			unzClose(uf);
			return FALSE;
		}
		
		size=file_info.uncompressed_size;

		if (strcmp(path,filename_inzip)==0)
		{
			// found it
			// extract it
			
			if (unzOpenCurrentFile(uf)!=UNZ_OK)
			{
				unzClose(uf);
				return NULL;
			}
						
			unsigned char *buf=new unsigned char[size];
			unzReadCurrentFile(uf,buf,size);
			unzCloseCurrentFile(uf);
			unzClose(uf);
			return buf;
		}

	} while (unzGoToNextFile(uf)==UNZ_OK);


	unzClose(uf);
	return NULL;
}

bool CPak::ExtractTo(const char *path, char *pathto)
{
	long size;
	unsigned char *buf=Extract(path,size);
	
	if (!buf)
		return FALSE;
	
	FILE *f=fopen(pathto,"wb");
	if (f)
	{				
		fwrite(buf,sizeof(unsigned char),size,f);
		fclose(f);
		delete buf;
		return TRUE;
	}	

	delete buf;
	return FALSE;
}

bool CPak::IsInPak(const char *path)
{
	pak_entry pe;	

	for ( std::list<pak_entry>::const_iterator iEntry = entries.begin() ; iEntry != entries.end() ; iEntry++)
	{
		pe=*iEntry;

		if (!pe.rename && pe.filename.compare(path)==0)
			return TRUE;

		if (pe.rename && pe.newname.compare(path)==0)
			return TRUE;
	}

	return FALSE;
}

bool CPak::AddExternalFile(const char *name,const char *path)
{

	FILE *fp;
	fp=fopen(path,"rb");

	if (!fp)
		return FALSE;

	fseek(fp,0,SEEK_END);
	long size=ftell(fp);
	fclose(fp);
	
	pak_entry pe;

	pe.rename=FALSE;
	pe.frompak=FALSE;
	pe.pathname=path;
	pe.filename=name;
	pe.compressed=size;
	pe.uncompressed=size;
	
	entries.push_back(pe);

	m_bModified=TRUE;

	return TRUE;

}

bool CPak::AddPakFile(const char *name,const char *pakname,long comp, long uncomp)
{
	pak_entry pe;

	pe.rename=FALSE;
	pe.frompak=TRUE;

	pe.filename=name;
	pe.pakname=pakname;
	pe.pathname="";
	
	pe.compressed=comp;
	pe.uncompressed=uncomp;
	
	entries.push_back(pe);

	m_bModified=TRUE;

	return TRUE;
}

bool CPak::Delete(const char *path)
{
	pak_entry pe;	

	for ( std::list<pak_entry>::iterator iEntry = entries.begin() ; iEntry != entries.end() ; iEntry++)
	{
		pe=*iEntry;

		if ((!pe.rename && pe.filename.compare(path)==0) || (pe.rename && pe.newname.compare(path)==0))
		{
			entries.erase(iEntry);
			m_bModified=TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

bool CPak::Rename(const char *path,const char *newname)
{
	pak_entry pe;	

	for ( std::list<pak_entry>::iterator iEntry = entries.begin() ; iEntry != entries.end() ; iEntry++)
	{
		pe=*iEntry;

		if ((!pe.rename && pe.filename.compare(path)==0) || (pe.rename && pe.newname.compare(path)==0))
		{
			entries.erase(iEntry);
			
			pe.rename=TRUE;
			pe.newname=newname;

			entries.push_back(pe);
			m_bModified=TRUE;

			return TRUE;
		}
	}

	return FALSE;
}
