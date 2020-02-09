// See: https://emscripten.org/docs/porting/files/Synchronous-Virtual-XHR-Backed-File-System-Usage.html

if (typeof(Module) === "undefined") Module = {};
Module['arguments'] = []
Module['preInit'] = function() {
    // Mount assets folder to Emscripten FS
    FS.mkdir('/assets');
    FS.mkdir('/assets/GLTF');
    FS.createLazyFile('/assets/GLTF', 'SuzanneColor0.hpp', 'assets/GLTF/SuzanneColor0.hpp', true, false);
};