
const fs = require("fs");
const path = require("path");

var DIRSEPA;
var PLATFORM;
if(process.platform=='darwin'){
  DIRSEPA='/';
  PLATFORM='mac';
}
else if(process.platform=='win32'){
  DIRSEPA='\\';
  PLATFORM='win';
}

if(process.argv.length<3){
  console.error("pack error.");
}

var filepath1="";
if(process.argv[2]=="cpdll"){

    if(process.platform=='darwin'){
    }
    else if(process.platform=='win32'){
      filepath1 = __dirname+DIRSEPA+"..\\ThirdPartyLibraries\\windows\\x86\\bin"+DIRSEPA;
    }
    var filepath2 = __dirname+DIRSEPA+"build"+DIRSEPA+"Release"+DIRSEPA;
    let files=[
      "libcrypto-1_1.dll",
      "libssl-1_1.dll",
      "zlib1.dll",
      "ssh.dll",
      "pthreadVC3.dll",
      "pthreadVCE3.dll",
      "pthreadVSE3.dll",
      "charset-1.dll",
      "cpprest_2_10.dll"
    ];
    for(let i=0;i<files.length;i++){
      let file=files[i];
      let source_filepath = filepath1 + file;
      let dest_file = filepath2 + file;
      fs.cpSync(source_filepath, dest_file);
      console.info("//(-----------");
      console.info(source_filepath);
      console.info(dest_file);
      console.info("//)-----------");
    }

}