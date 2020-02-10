// See: https://emscripten.org/docs/porting/files/Synchronous-Virtual-XHR-Backed-File-System-Usage.html

if (typeof(Module) === "undefined") Module = {};
Module['arguments'] = []
Module['preInit'] = function() {
    // Mount assets folder to Emscripten FS
    
};
