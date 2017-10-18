

enum class PluginType : unsigned int {
	UNKNOWN // Add plugin 
};

struct PluginVersion {
	unsigned char major;
	unsigned char minor;
	unsigned short patch;
};

class PluginInfo {
public :
	PluginVersion version; 
	PluginType type;
	char name[64];
	char description[256];
};

