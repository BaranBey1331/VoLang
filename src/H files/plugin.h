#ifndef PLUGIN_H
#define PLUGIN_H

// Preparation for future native C/C++ plugin support
// This will allow VoLang to surpass things like Discord.js by loading raw, fast C extensions.

typedef struct {
    const char* plugin_name;
    const char* version;
    void* library_handle; // Handle for dlopen (dynamic loading)
} VoPlugin;

// Future functions we will implement later
void plugin_system_init();
int load_plugin(const char* path);

#endif // PLUGIN_H
