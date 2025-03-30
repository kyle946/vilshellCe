const fs = require("fs");
const path = require("path");

var DIRSEPA;
var PLATFORM;

if(process.platform=='darwin'){
  DIRSEPA='/';
  PLATFORM='mac';
}
else if(process.platform=='linux'){
  DIRSEPA='/';
  PLATFORM='linux';
}
else if(process.platform=='win32'){
  DIRSEPA='\\';
  PLATFORM='win';
}

if(process.argv[2]=="cpfile"){
  {
    let modulename = "villib1";
    let _src = path.dirname(__dirname) + DIRSEPA+modulename+DIRSEPA+"build"+DIRSEPA+"Release"+DIRSEPA+modulename+".node";
    let _dest = __dirname +DIRSEPA+"dist"+DIRSEPA+modulename+".node";
    fs.cpSync(_src, _dest);
    console.log("src", _src);
    console.log("dest", _dest);
  }
}