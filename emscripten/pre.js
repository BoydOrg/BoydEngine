if (typeof (Module) === "undefined") Module = {};

// Need to preload the asset in their own Worker
// See: https://emscripten.org/docs/porting/files/Synchronous-Virtual-XHR-Backed-File-System-Usage.html
var worker = new Worker('LoaderWorker.js');
worker.onmessage = function (moduleMsg) {
    Module = moduleMsg.data['Module'];
    console.log(Module);
};
