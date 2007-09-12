
#define P3D_GET_CROSSLINE		1
#define P3D_GET_RESTOFLINE		2

int P3DLoad( const char *filename );
void P3DClose();

int P3DGetNextPair( char **name, char **associatedShader );
