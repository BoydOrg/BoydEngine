if (typeof (Module) === "undefined") Module = {};

// Need to preload the asset in their own Worker
// See: https://emscripten.org/docs/porting/files/Synchronous-Virtual-XHR-Backed-File-System-Usage.html
var worker = new Worker('LoaderWorker.js');
worker.onmessage = function (moduleMsg) {
    console.log("Got Module from the XHR worker");
    var newModule = moduleMsg.data['Module'];
    console.log(newModule);

    Module = Object.assign(Module, newModule);
};
