// MD3 file headers


typedef struct
{
	char ID[4];         //id of file, always "IDP3"
	int  Version;       //i suspect this is a version
						//number, always 15
	char FileName[68]; 	//sometimes left Blank...
						//65 chars, 32bit aligned ==
						//68 chars
	int  BoneFrame_num; //number of BoneFrames
	int  Tag_num;       //number of 'tags' per BoneFrame
	int  Mesh_num;      //number of meshes/skins
	int  MaxSkin_num;   //maximum number of unique skins
						//used in md3 file
	int  HeaderLength;  //always equal to the length of
						//this header
	int  Tag_Start;     //starting position of
						//tag-structures
	int  Tag_End;      	//ending position of
						//tag-structures/starting
						//position of mesh-structures
	int  FileSize;     	//size of file
} md3_header_t;


typedef struct
{
	char Name[64];    	//name of 'tag' as it's usually
						//called in the md3 files try to
						//see it as a sub-mesh/seperate
						//mesh-part.
						//sometimes this 64 string may
						//contain some garbage, but
						//i've been told this is because
						//some tools leave garbage in
						//those strings, but they ARE
						//strings...
	float Postition[3]; //relative position of tag
	float Rotation[3][3]; 	//the direction the tag is facing relative to the rest of the model
} md3_tag_t;


typedef struct
{
	//unverified:
	float  Mins[3];
	float  Maxs[3];
	float  Position[3];
	float  scale;
	char   Creator[16]; 	//i think this is the
                    		//"creator" name..
                    		//but i'm only guessing.
} md3_boneframe_t;


typedef struct
{
	char ID[4];         	//id, must be IDP3
	char Name[68];       	//name of mesh
                     		//65 chars,
                     		//32 bit aligned == 68 chars
	int  MeshFrame_num;  	//number of meshframes
                     		//in mesh
	int  Skin_num;       	//number of skins in mesh
	int  Vertex_num;     	//number of vertices
	int  Triangle_num;   	//number of Triangles
	int  Triangle_Start; 	//starting position of
                     		//Triangle data, relative
                     		//to start of Mesh_Header
	int  HeaderSize;     	//size of header
	int  TexVec_Start;   	//starting position of
                     		//texvector data, relative
                     		//to start of Mesh_Header
	int  Vertex_Start;   	//starting position of
                     		//vertex data,relative
                     		//to start of Mesh_Header
	int  MeshSize;       	//size of mesh
} md3_mesh_t;


typedef struct
{
	char Name[68]; 	//name of skin used by mesh
               		//65 chars,
               		//32 bit aligned == 68 chars
} md3_skin_t;


typedef struct
{
	int  Triangle[3]; 	//vertex 1,2,3 of triangle

} md3_triangles_t;


typedef struct
{
	float  TexVec[2]; 	//Texture U/V coordinates of vertex

} md3_texcoords_t;


typedef struct
{
	//!!!important!!! signed!
	signed short Vec[3];   		//vertex X/Y/Z coordinate
	unsigned char EnvTex[2];   	//enviromental mapping texture coordinates

} md3_vertices_t;
